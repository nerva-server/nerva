#ifndef THREAD_SAFE_QUEUE_HPP
#define THREAD_SAFE_QUEUE_HPP

#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <atomic> // For shutdownServer flag

// Forward declaration of the global shutdown flag defined in main.cpp.
// Each process will have its own copy of this global variable.
extern std::atomic<bool> shutdownServer;

class ThreadSafeQueue
{
    std::queue<int> queue;
    std::mutex mtx;
    std::condition_variable cv;

public:
    void push(int socket)
    {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(socket);
        cv.notify_one();
    }

    bool pop(int &socket)
    {
        std::unique_lock<std::mutex> lock(mtx);
        // Wait for an item or for the shutdown signal.
        // The `shutdownServer` flag here refers to the copy in the current process.
        if (cv.wait_for(lock, std::chrono::milliseconds(100),
                        [this]
                        { return !queue.empty() || shutdownServer; }))
        {
            if (queue.empty())
                return false; // If woken by shutdown and queue is empty
            socket = queue.front();
            queue.pop();
            return true;
        }
        return false; // Timeout or shutdown
    }
};

#endif // THREAD_SAFE_QUEUE_HPP
