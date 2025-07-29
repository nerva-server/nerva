#ifndef CLUSTER_HPP
#define CLUSTER_HPP

#include <vector>
#include <atomic>
#include <csignal>

class Cluster
{
public:
    Cluster();
    ~Cluster();

    std::vector<pid_t> forkWorkers(int serverSocket, int cpuCount);

    void sendShutdownSignal(const std::vector<pid_t> &workers);

    void waitForWorkers(const std::vector<pid_t> &workers);

    static void sigchld_handler(int signum);
};

#endif
