#ifndef SERVER_HPP
#define SERVER_HPP

#include <atomic>
#include <string>
#include <vector>
#include <thread>
#include <netinet/in.h>

#include "Utils/ThreadSafeQueue.hpp"
#include "Secure/Config/ServerConfig.hpp"
#include "Core/Http/Router/Router.hpp"

class Server : public Router
{
public:
    Server(int serverSocket, std::atomic<bool> &shutdownFlag);
    ~Server();

    void startWorker();
    void stopWorker();

    static int initSocket(int port, int listenQueueSize);

private:
    int serverSocket;
    std::atomic<bool> &shutdownServer;
    ThreadSafeQueue socketQueue;
    std::atomic<int> activeConnections;

    std::vector<std::thread> acceptThreads;
    std::vector<std::thread> threadPool;

    void acceptConnections();
    void handleClient(int clientSocket);

    static int SetNonBlocking(int fd);
};

#endif
