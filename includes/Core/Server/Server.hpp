#ifndef SERVER_HPP
#define SERVER_HPP

#include <atomic>
#include <string>
#include <vector>
#include <thread>
#include <netinet/in.h>
#include <sys/epoll.h>

#include "Secure/Config/ConfigParser.hpp"
#include "Utils/ThreadSafeQueue.hpp"
#include "Core/Http/Router/Router.hpp"
#include "Core/Http/Handler/StaticFileHandler.hpp"

class Server : public Router
{
public:
    Server();
    ~Server();

    void Start();
    void Stop();

    void Static(const std::string &path, const std::string &directory)
    {
        auto handler = new StaticFileHandler(directory);
        Use(path, *handler);
    }

    void SetConfigFile(std::string path);

    static int initSocket(int port, int listenQueueSize);

private:
    int serverSocket;
    int epollFd;

    ConfigParser config;

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