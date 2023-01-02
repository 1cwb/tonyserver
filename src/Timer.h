#pragma once
#include <unistd.h>
#include <deque>
#include <memory>
#include <queue>
#include <sys/time.h>
#include "HttpData.h"
#include <limits>
#include <iostream>
#include <functional>
#include <noncopyable.h>
#include <atomic>
#include "Timestamp.h"
#include "Callbacks.h"

class Timer : NonCopyAble
{
public:
    Timer(TimerCallback cb, const Timestamp& when, double interval)
    :callback_(std::move(cb)),
    expiration_(when),
    interval_(interval),
    repeat_(interval > 0.0),
    sequence_(sNumCreated_++)
    {}
    void run() const
    {
        callback_();
    }
    Timestamp expiration() const {return expiration_;}
    bool repeat() const {return repeat_;}
    int64_t sequence() const {return sequence_;}
    void restart(Timestamp now);
    static int64_t sNumCreated() {return sNumCreated_;}
private:
    const TimerCallback callback_;
    Timestamp expiration_;
    const double interval_;
    const bool repeat_;
    const int64_t sequence_;

    static std::atomic<int64_t> sNumCreated_;
};

class TimerId
{
public:
    TimerId():timer_(nullptr),sequence_(0)
    {
        
    }
    TimerId(Timer* timer, int64_t seq) : timer_(timer), sequence_(seq)
    {
        
    }
private:
    Timer* timer_;
    int64_t sequence_;
};