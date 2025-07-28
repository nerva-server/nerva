#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <csignal>
#include <sys/wait.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <cstring>
#include <system_error>
#include <sstream>

#include "Secure/Config/ServerConfig.hpp"
#include "Core/Cluster/Cluster.hpp"
#include "Core/Server/Server.hpp"

std::atomic<bool> shutdownServer(false);

void signal_handler(int signum)
{
    std::cout << "Process " << getpid() << " received signal " << signum << ", initiating graceful shutdown...\n";
    shutdownServer = true;
}

int main()
{
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    ServerConfig config;

    int serverSocket = Server::initSocket(config.PORT, config.ACCEPT_QUEUE_SIZE);
    if (serverSocket < 0)
    {
        std::cerr << "Failed to initialize server socket. Exiting.\n";
        return EXIT_FAILURE;
    }

    std::cout << "Sunucu " << config.PORT << " portunda dinleniyor...\n";

    int cpuCount = 6;
    if (cpuCount == 0)
        cpuCount = 4;

    Cluster clusterManager;
    std::vector<pid_t> workers = clusterManager.forkWorkers(serverSocket, cpuCount);

    if (workers.empty()) {
        Server workerServer(serverSocket, shutdownServer);
        workerServer.startWorker();
        close(serverSocket);
        return 0;
    }

    while (!shutdownServer) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    clusterManager.sendShutdownSignal(workers);
    clusterManager.waitForWorkers(workers);
    close(serverSocket);
    std::cout << "Sunucu kapatıldı.\n";

    return 0;
}