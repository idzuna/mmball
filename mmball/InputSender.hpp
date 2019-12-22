#pragma once

#include <Windows.h>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

template <class T>
class BlockingQueue
{
private:
    std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<T> queue_;
public:
    void post(const T& item)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(item);
        cv_.notify_one();
    }
    T get()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [&]() { return !queue_.empty(); });
        T result = std::move(queue_.front());
        queue_.pop();
        return result;
    }
};

class InputSender
{
private:
    BlockingQueue<INPUT> queue_;
    std::thread thread_;
    void worker()
    {
        while (true) {
            INPUT input = queue_.get();
            SendInput(1, &input, sizeof(input));
        }
    }
public:
    void send(const INPUT &input)
    {
        queue_.post(input);
    }
    InputSender()
        : thread_([&]() { worker(); })
    {
    }
};
