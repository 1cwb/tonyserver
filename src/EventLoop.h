#pragma once
#include <functional>
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>
#include "noncopyable.h"
#include "Epoll.h"
#include "Timestamp.h"
#include "TimerQueue.h"

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

    TimerId runAt(Timestamp time, TimerCallback cb);
    TimerId runAfter(double delay, TimerCallback cb);
    TimerId runEvery(double interval, TimerCallback cb);
    void cancel(TimerId timerId);

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
    std::thread::id getLoopThreadId() const
    {
        return threadId_;
    }
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
    Timestamp pollReturnTime_;
    Epoll* epoll_;
    TimerQueue* timerQueue_;
    int32_t wakeupFd_;
    Channel* wakeupChannel_;
    ChannelList activeChannel_;
    Channel* currentActiveChannel_;
    std::mutex mutex_;
    std::vector<Functor> pendingFunctors_;
};