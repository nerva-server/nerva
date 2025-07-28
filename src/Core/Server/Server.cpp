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
#include <sstream>
#include <chrono>
#include <thread>
#include <stdexcept>
#include <atomic>
#include <csignal>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#include "Utils/ThreadSafeQueue.hpp"
#include "Core/Cluster/Cluster.hpp"

std::atomic<bool> shutdownServer{false};

Server::Server(ServerConfig &config)
    : config(config),
      activeConnections(0)
{
}

Server::~Server()
{
    Stop();
}

int Server::SetNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int Server::initSocket(int port, int listenQueueSize)
{
    int sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (sock == -1)
    {
        perror("socket failed");
        return -1;
    }

    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt(SO_REUSEADDR)");
        close(sock);
        return -1;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt(SO_REUSEPORT)");
        close(sock);
        return -1;
    }

    if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)))
    {
        perror("setsockopt(TCP_NODELAY)");
        close(sock);
        return -1;
    }

    int bufsize = 1024 * 1024;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize)))
    {
        perror("setsockopt(SO_RCVBUF)");
    }
    if (setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize)))
    {
        perror("setsockopt(SO_SNDBUF)");
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

    SetNonBlocking(sock);

    return sock;
}

void Server::acceptConnections()
{
    ServerConfig config;
    int epollFd = epoll_create1(0);
    if (epollFd == -1)
    {
        perror("epoll_create1");
        return;
    }
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = serverSocket;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket, &event) == -1)
    {
        perror("epoll_ctl");
        close(epollFd);
        return;
    }

    struct epoll_event events[config.MAX_EVENTS];

    while (!shutdownServer)
    {
        int numEvents = epoll_wait(epollFd, events, config.MAX_EVENTS, 0);
        if (numEvents == -1)
        {
            if (errno == EINTR)
                continue;
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < numEvents; ++i)
        {
            if (events[i].data.fd == serverSocket)
            {
                int clientSocket;
                struct sockaddr_in clientAddress;
                socklen_t clientAddrLen = sizeof(clientAddress);

                clientSocket = accept4(serverSocket, (struct sockaddr *)&clientAddress,
                                       &clientAddrLen, SOCK_NONBLOCK);

                if (clientSocket < 0)
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                        continue;
                    if (errno == EMFILE || errno == ENFILE)
                    {
                        std::cerr << "File descriptor limit reached\n";
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

                event.events = EPOLLIN | EPOLLET;
                event.data.fd = clientSocket;
                if (epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket, &event) == -1)
                {
                    perror("epoll_ctl: clientSocket");
                    close(clientSocket);
                    continue;
                }
            }
            else
            {
                int clientSocket = events[i].data.fd;
                if (events[i].events & EPOLLERR || events[i].events & EPOLLHUP)
                {
                    epoll_ctl(epollFd, EPOLL_CTL_DEL, clientSocket, nullptr);
                    close(clientSocket);
                    activeConnections--;
                    continue;
                }

                socketQueue.push(clientSocket);
                epoll_ctl(epollFd, EPOLL_CTL_DEL, clientSocket, nullptr);
            }
        }
    }
    close(epollFd);
}

void Server::handleClient(int clientSocket)
{
    ServerConfig config;
    activeConnections++;
    char buffer[config.BUFFER_SIZE];
    try
    {
        while (!shutdownServer)
        {
            int valread = read(clientSocket, buffer, config.BUFFER_SIZE);
            if (valread < 0)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    continue;

                close(clientSocket);
                activeConnections--;
                return;
            }
            else if (valread == 0)
            {
                close(clientSocket);
                activeConnections--;
                return;
            }
            else if (valread == config.BUFFER_SIZE)
            {
                std::cerr << "Buffer overflow detected, closing connection.\n";
                close(clientSocket);
                activeConnections--;
                return;
            }

            std::string requestData(buffer, valread);

            std::istringstream stream(requestData);
            std::string method, path, httpVersion;
            stream >> method >> path >> httpVersion;

            Http::Request req;
            req.method = method;
            req.path = path;
            req.body = "";

            Http::Response res;

            if (!dispatch(req, res))
            {
            }

            std::string responseData = res.toString();

            if (send(clientSocket, responseData.c_str(), responseData.size(), MSG_NOSIGNAL) < 0)
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

void Server::Start()
{
    int cpuCount = 6;
    if (cpuCount == 0)
        cpuCount = 4;

    serverSocket = initSocket(config.PORT, config.ACCEPT_QUEUE_SIZE);
    if (serverSocket < 0)
    {
        std::cerr << "Failed to initialize server socket. Exiting.\n";
        return;
    }

    struct sigaction sa;
    sa.sa_handler = [](int) {};
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGINT, &sa, NULL) == -1 ||
        sigaction(SIGTERM, &sa, NULL) == -1)
    {
        perror("sigaction");
        return;
    }
    
    Cluster clusterManager;
    std::vector<pid_t> workers = clusterManager.forkWorkers(serverSocket, cpuCount);

    if (workers.empty() && getpid() != getppid())
    {
        StartWorker();
    }
    else
    {
        while (!shutdownServer.load())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        clusterManager.sendShutdownSignal(workers);
        clusterManager.waitForWorkers(workers);
        close(serverSocket);
        std::cout << "Server shut down.\n";
    }
}

void Server::StartWorker()
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

void Server::Stop()
{
    shutdownServer = true;
    
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