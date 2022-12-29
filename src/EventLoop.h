#pragma once
#include <functional>
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>
#include "noncopyable.h"
#include "Epoll.h"

class Channel;
class Epoll;
class EventLoop : NonCopyAble
{
public:
    using Functor = std::function<void ()>;
    using ChannelList = std::vector<Channel*>;
    EventLoop();
    ~EventLoop();
    void loop();
    void quit();
    int64_t iteration() const
    {
        return iteration_;
    }
    void runInLoop(Functor cb);
    void queueInLoop(Functor cb);
    size_t queueSize() const;
    void wakeup();
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);
    void assertInLoopThread()
    {
        if(!isInLoopThread())
        {
            abortNotInLoopThread();
        }
    }
    bool isInLoopThread() const {return threadId_ == std::this_thread::get_id();}
    bool eventHandling() const {return eventHandling_;}
    static EventLoop* getEventLoopOfCurrentThread();
private:
    void abortNotInLoopThread();
    void handleRead();
    void doPendingFunctors();
    void printActiveChannels() const;

private:
    bool looping_;
    std::atomic<bool> quit_;
    std::atomic<bool> eventHandling_;
    std::atomic<bool> callingPendingFunctors_;
    int64_t iteration_;
    const std::thread::id threadId_;
    Epoll* epoll_;
    int32_t wakeupFd_;
    Channel* wakeupChannel_;
    ChannelList activeChannel_;
    Channel* currentActiveChannel_;
    std::mutex mutex_;
    std::vector<Functor> pendingFunctors_;
};