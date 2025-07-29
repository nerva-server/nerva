#ifndef THREAD_SAFE_QUEUE_HPP
#define THREAD_SAFE_QUEUE_HPP

#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <atomic>

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
        if (cv.wait_for(lock, std::chrono::milliseconds(100),
                        [this]
                        { return !queue.empty() || shutdownServer; }))
        {
            if (queue.empty())
                return false;
            socket = queue.front();
            queue.pop();
            return true;
        }
        return false;
    }
};

#endif
