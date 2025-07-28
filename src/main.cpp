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

    int cpuCount = std::thread::hardware_concurrency();
    if (cpuCount == 0)
        cpuCount = 4;

    Cluster clusterManager;

    std::vector<pid_t> workers = clusterManager.forkWorkers(serverSocket, cpuCount);

    if (workers.empty() && getpid() != getppid())
    {
        Server workerServer(serverSocket, shutdownServer);

        std::string body = "<!DOCTYPE html>\n"
                           "<html lang='en'>\n"
                           "<head>\n"
                           "    <meta charset='UTF-8'>\n"
                           "    <title>HTML</title>\n"
                           "</head>\n"
                           "<body>\n"
                           "    <h1>Hello World!</h1>\n"
                           "    <p>\n"
                           "        <!-- Aşağıdaki boşluk dolgu içerik 10KB'yi tamamlar -->\n"
                           "        Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.\n"
                           "        Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.\n"
                           "        Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur.\n"
                           "        Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.\n"
                           "    </p>\n"
                           "</body>\n"
                           "</html>";
        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n"
                 << "Content-Type: text/html\r\n"
                 << "Content-Length: " << body.size() << "\r\n"
                 << "\r\n"
                 << body;

        workerServer.get("/", [&response](const std::string &requestPath) {
            return response.str();
        });

        workerServer.startWorker();

        close(serverSocket);
        exit(0);
    }
    else
    {
        std::cout << "Master process PID: " << getpid() << ", " << cpuCount << " worker fork edildi.\n";
        std::cout << "Sunucu çalışıyor. Çıkmak için CTRL+C...\n";

        while (!shutdownServer)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        clusterManager.sendShutdownSignal(workers);

        clusterManager.waitForWorkers(workers);

        close(serverSocket);
        std::cout << "Sunucu kapatıldı.\n";
    }

    return 0;
}
