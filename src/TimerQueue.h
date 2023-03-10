#pragma once
#include <vector>
#include <set>
#include "Timestamp.h"
#include "Channel.h"
#include "noncopyable.h"
#include "Callbacks.h"

class EventLoop;
class Timer;
class TimerId;

class TimerQueue : NonCopyAble
{
public: 
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    TimerId addTimer(TimerCallback cb, Timestamp when, double interval);

    void cancel(TimerId timerId);

private:
    using Entry = std::pair<Timestamp, Timer*>;
    using TimerList = std::set<Entry>;
    using ActiveTimer = std::pair<Timer*, int64_t>;
    using ActiveTimerSet = std::set<ActiveTimer>;

    void addTimerInLoop(Timer* timer);
    void cancelInLoop(TimerId timerId);

    void handleRead();

    std::vector<Entry> getExpired(Timestamp now);
    void reset(const std::vector<Entry>& expired, Timestamp now);
    bool insert(Timer* timer);

    EventLoop* loop_;
    const int timerfd_;
    Channel timerfdChannel_;
    TimerList timers_;

    ActiveTimerSet activeTimers_;
    bool callingExpiredTimers_;
    ActiveTimerSet cancelingTimers_;
};

 