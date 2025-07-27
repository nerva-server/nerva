#include "Server.hpp"
#include "Secure/Config/ServerConfig.hpp"

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <system_error>
#include <fcntl.h>

Server::Server(int serverSocket, std::atomic<bool> &shutdownFlag)
    : serverSocket(serverSocket),
      shutdownServer(shutdownFlag),
      activeConnections(0)
{
    int flags = fcntl(serverSocket, F_GETFL, 0);
    if (flags == -1)
    {
        perror("fcntl F_GETFL");
    }
    if (fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        perror("fcntl F_SETFL O_NONBLOCK");
    }
}

Server::~Server()
{
    stopWorker();
}

int Server::initSocket(int port, int listenQueueSize)
{
    int sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (sock == 0)
    {
        perror("socket failed");
        return -1;
    }

    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        close(sock);
        return -1;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        close(sock);
        return -1;
    }

    if (listen(sock, listenQueueSize) < 0)
    {
        perror("listen");
        close(sock);
        return -1;
    }
    return sock;
}

void Server::acceptConnections()
{
    ServerConfig config;
    while (!shutdownServer)
    {
        int clientSocket;
        struct sockaddr_in clientAddress;
        socklen_t clientAddrLen = sizeof(clientAddress);

        clientSocket = accept4(serverSocket, (struct sockaddr *)&clientAddress,
                               &clientAddrLen, SOCK_NONBLOCK);

        if (clientSocket < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(config.ACCEPT_RETRY_DELAY_MS));
                continue;
            }
            if (errno == EMFILE || errno == ENFILE)
            {
                std::cerr << "File descriptor limit reached, waiting...\n";
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }
            perror("accept4");
            continue;
        }

        if (activeConnections >= config.MAX_CONNECTIONS)
        {
            close(clientSocket);
            continue;
        }

        int flag = 1;
        if (setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int)) < 0)
        {
            perror("setsockopt(TCP_NODELAY)");
        }

        socketQueue.push(clientSocket);
    }
}

void Server::handleClient(int clientSocket)
{
    ServerConfig config;
    activeConnections++;
    char buffer[config.BUFFER_SIZE];

    std::string body =
        "<html><body><h1>Merhaba Dunya!</h1>"
        "<p>Bu son derece optimize C++ web sunucusudur.</p></body></html>";
    std::string headers =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Content-Length: " +
        std::to_string(body.size()) + "\r\n"
                                      "Connection: keep-alive\r\n"
                                      "Keep-Alive: timeout=" +
        std::to_string(config.KEEP_ALIVE_TIMEOUT) + "\r\n"
                                                    "\r\n";
    std::string response = headers + body;

    try
    {
        while (!shutdownServer)
        {
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(clientSocket, &readfds);

            struct timeval timeout = {config.KEEP_ALIVE_TIMEOUT, 0};

            int activity = select(clientSocket + 1, &readfds, nullptr, nullptr, &timeout);
            if (activity < 0)
            {
                if (errno == EBADF)
                {
                    break;
                }
                throw std::system_error(errno, std::system_category(), "select");
            }
            if (activity == 0)
                break;

            if (!FD_ISSET(clientSocket, &readfds))
                continue;

            int valread = read(clientSocket, buffer, config.BUFFER_SIZE);
            if (valread <= 0)
                break;

            if (send(clientSocket, response.c_str(), response.size(), MSG_NOSIGNAL) < 0)
            {
                throw std::system_error(errno, std::system_category(), "send");
            }
        }
    }
    catch (const std::system_error &e)
    {
        std::cerr << "Client handling error: " << e.what() << " (" << e.code() << ")\n";
    }

    close(clientSocket);
    activeConnections--;
}

void Server::startWorker()
{
    ServerConfig config;

    for (int i = 0; i < 4; ++i)
    {
        acceptThreads.emplace_back(&Server::acceptConnections, this);
    }

    for (int i = 0; i < config.THREAD_POOL_SIZE; ++i)
    {
        threadPool.emplace_back([this]()
                                {
            while (!shutdownServer) {
                int clientSocket;
                if (socketQueue.pop(clientSocket)) {
                    handleClient(clientSocket);
                }
            } });
    }

    for (auto &t : acceptThreads)
    {
        if (t.joinable())
            t.join();
    }
    for (auto &t : threadPool)
    {
        if (t.joinable())
            t.join();
    }
}

void Server::stopWorker()
{
    for (auto &t : acceptThreads)
    {
        if (t.joinable())
            t.join();
    }
    for (auto &t : threadPool)
    {
        if (t.joinable())
            t.join();
    }
}