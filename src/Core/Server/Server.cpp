#include "Server.hpp"
#include "ThreadSafeQueue.hpp"
#include "Cluster.hpp"

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
#include <poll.h>

std::atomic<bool> shutdownServer{false};

void signalHandler(int signum)
{
    shutdownServer.store(true);
}

Server::Server() : activeConnections(0),
                   config("server.nrvcfg")
{
    std::signal(SIGINT, signalHandler);
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

    struct epoll_event events[config.getInt("max_events")];

    while (!shutdownServer)
    {
        int numEvents = epoll_wait(epollFd, events, config.getInt("max_events"), 0);
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

                if (activeConnections >= config.getInt("max_connections"))
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

void Server::SetConfigFile(std::string path)
{
    config = ConfigParser(path + ".nrvcfg");
}

void Server::handleClient(int clientSocket)
{
    activeConnections++;
    try
    {
        const size_t BUFFER_SIZE = config.getInt("buffer_size");
        std::vector<char> buffer(BUFFER_SIZE);
        std::string requestData;
        requestData.reserve(BUFFER_SIZE * 2);

        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        while (!shutdownServer)
        {
            ssize_t valread = recv(clientSocket, buffer.data(), buffer.size(), 0);

            if (valread < 0)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    struct pollfd pfd = {clientSocket, POLLIN, 0};
                    if (poll(&pfd, 1, 100) <= 0)
                        break;
                    continue;
                }
                if (errno == ECONNRESET || errno == ETIMEDOUT)
                    break;
                throw std::system_error(errno, std::system_category(), "recv failed");
            }
            else if (valread == 0)
            {
                break;
            }

            requestData.append(buffer.data(), valread);

            size_t headerEnd = requestData.find("\r\n\r\n");
            if (headerEnd == std::string::npos)
                continue;

            size_t contentLength = 0;
            if (auto clPos = requestData.find("Content-Length:"); clPos != std::string::npos)
            {
                try
                {
                    size_t clEnd = requestData.find("\r\n", clPos);
                    std::string clStr = requestData.substr(clPos + 15, clEnd - clPos - 15);
                    contentLength = std::stoul(clStr);
                }
                catch (...)
                {
                    throw std::runtime_error("Invalid Content-Length");
                }
            }

            if (requestData.size() < (headerEnd + 4 + contentLength))
            {
                continue;
            }

            Http::Request req;
            if (!req.parse(requestData))
            {
                std::string badReq = "HTTP/1.1 400 Bad Request\r\n"
                                     "Connection: close\r\n"
                                     "Content-Length: 0\r\n\r\n";
                send(clientSocket, badReq.data(), badReq.size(), MSG_NOSIGNAL);
                break;
            }

            Http::Response res;
            res._engine = _engine;
            res.viewDir = keys["views"];

            if (req.headers.find("Cookie") != req.headers.end())
            {
                std::string cookieHeader = req.headers["Cookie"];
                size_t pos = 0;
                while (pos < cookieHeader.length())
                {
                    pos = cookieHeader.find_first_not_of(" ;\t", pos);
                    if (pos == std::string::npos)
                        break;

                    size_t eq_pos = cookieHeader.find('=', pos);
                    if (eq_pos == std::string::npos || eq_pos <= pos)
                    {
                        break;
                    }

                    size_t end_pos = cookieHeader.find(';', eq_pos);
                    if (end_pos == std::string::npos)
                    {
                        end_pos = cookieHeader.length();
                    }

                    std::string name = cookieHeader.substr(pos, eq_pos - pos);
                    std::string value = cookieHeader.substr(eq_pos + 1, end_pos - eq_pos - 1);

                    auto trim = [](std::string &s)
                    {
                        size_t start = s.find_first_not_of(" \t");
                        if (start == std::string::npos)
                            return;
                        size_t end = s.find_last_not_of(" \t");
                        s = s.substr(start, end - start + 1);
                    };

                    trim(name);
                    trim(value);

                    res.incomingCookies[name] = value;

                    pos = end_pos + 1;
                }
            }

            this->Handle(req, res, []() {});

            std::string response = res.toString();
            if (send(clientSocket, response.data(), response.size(), MSG_NOSIGNAL) < 0)
            {
                throw std::system_error(errno, std::system_category(), "send failed");
            }

            bool keepAlive = req.headers["Connection"] == "keep-alive" ||
                             (req.version == "HTTP/1.1" && req.headers["Connection"] != "close");

            if (!keepAlive)
                break;

            size_t requestEnd = headerEnd + 4 + contentLength;
            if (requestData.size() > requestEnd)
            {
                requestData = requestData.substr(requestEnd);
            }
            else
            {
                requestData.clear();
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Client error: " << e.what() << std::endl;
    }

    close(clientSocket);
    activeConnections--;
}

void Server::Start()
{
    int cpuCount = config.getInt("cluster_thread");
    if (cpuCount == 0)
        cpuCount = 4;

    serverSocket = initSocket(config.getInt("port"), config.getInt("accept_queue_size"));
    if (serverSocket < 0)
    {
        std::cerr << "Failed to initialize server socket. Exiting.\n";
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
    for (int i = 0; i < 4; ++i)
    {
        acceptThreads.emplace_back(&Server::acceptConnections, this);
    }

    for (int i = 0; i < config.getInt("thread_pool_size"); ++i)
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
    shutdownServer.store(true);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

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