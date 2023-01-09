#pragma once
#include "EventLoop.h"
#include "noncopyable.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

class EventLoopThread : public NonCopyAble
{
public:
    using ThreadInitCallback = std::function<void (EventLoop*)>;
    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                    const std::string& name = std::string());
    ~EventLoopThread();
    EventLoop* startLoop();
private:
    void threadFunc();

    EventLoop* loop_;
    bool exiting_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};
