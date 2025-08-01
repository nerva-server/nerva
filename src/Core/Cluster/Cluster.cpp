#include "Cluster.hpp"

#include <iostream>
#include <unistd.h>
#include <sys/wait.h>

extern std::atomic<bool> shutdownServer;

Cluster::Cluster()
{
    signal(SIGCHLD, Cluster::sigchld_handler);
}

Cluster::~Cluster()
{
}

std::vector<pid_t> Cluster::forkWorkers(int serverSocket, int cpuCount)
{
    std::vector<pid_t> workers;
    for (int i = 0; i < cpuCount; ++i)
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork");
            for (pid_t existing_pid : workers)
            {
                kill(existing_pid, SIGTERM);
            }
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            return std::vector<pid_t>();
        }
        else
        {
            workers.push_back(pid);
        }
    }
    return workers;
}

void Cluster::sendShutdownSignal(const std::vector<pid_t> &workers)
{
    for (pid_t pid : workers)
    {
        kill(pid, SIGTERM);
    }
}

void Cluster::waitForWorkers(const std::vector<pid_t> &workers)
{
    for (pid_t pid : workers)
    {
        int status;
        if (waitpid(pid, &status, 0) == -1)
        {
            // perror("waitpid");
        }
        else
        {
            // std::cout << "Worker " << pid << " terminated with status " << status << ".\n";
        }
    }
}

void Cluster::sigchld_handler(int)
{
    while (waitpid(-1, nullptr, WNOHANG) > 0)
        ;
}
