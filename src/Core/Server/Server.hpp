#ifndef SERVER_HPP
#define SERVER_HPP

#include <atomic>
#include <string>
#include <vector>
#include <thread>
#include <netinet/in.h>
#include <sys/epoll.h>

#include "Utils/ThreadSafeQueue.hpp"
#include "Secure/Config/ServerConfig.hpp"
#include "Core/Http/Router/Router.hpp"

class Server : public Router
{
public:
    Server(ServerConfig &config);
    ~Server();

    void Start();
    void Stop();

    static int initSocket(int port, int listenQueueSize);

private:
    int serverSocket;
    int epollFd;

    ServerConfig config;

    std::thread acceptThread;

    ThreadSafeQueue socketQueue;
    std::atomic<int> activeConnections;

    std::vector<std::thread> acceptThreads;
    std::vector<std::thread> threadPool;

    void acceptConnections();
    void handleClient(int clientSocket);
    void StartWorker();
    static int SetNonBlocking(int fd);
};

#endif