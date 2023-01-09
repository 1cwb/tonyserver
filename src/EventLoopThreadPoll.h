#pragma once
#include "EventLoopThread.h"

class EventLoopThreadPoll : public NonCopyAble
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    EventLoopThreadPoll(EventLoop* baseLoop, const std::string& name);
    ~EventLoopThreadPoll();
    void setThreadNum(int32_t numThreads) {numThreads_ = numThreads;}
    void start(const ThreadInitCallback& cb = ThreadInitCallback());
    EventLoop* getNextLoop();
    EventLoop* getLoopForHash(size_t hashCode);
    const std::vector<EventLoop*>* getAllLoops() const;
    bool started() const
    {
        return started_;
    }
    const std::string& name() const
    {
        return name_;
    }
private:
    EventLoop* baseLoop_;
    std::string name_;
    bool started_;
    int32_t numThreads_;
    size_t next_;
    std::vector<EventLoopThread*> threads_;
    std::vector<EventLoop*> loops_;
};