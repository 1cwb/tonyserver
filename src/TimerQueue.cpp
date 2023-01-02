#include "TimerQueue.h"
#include <iostream>
#include <sys/timerfd.h>
#include <unistd.h>
#include <cstring>
#include <assert.h>
#include "Timer.h"
#include "EventLoop.h"

using namespace std;
int32_t createTimerfd()
{
    int32_t timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if(timerfd < 0)
    {
        cout << "Error failed in timerfd_create" << endl;
    }
    return timerfd;
}
struct timespec howMuchTimeFromNow(Timestamp when)
{
    int64_t microseconds = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
    if(microseconds < 100)
    {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(microseconds / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>((microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
    return ts;
}
void readTimerfd(int32_t timerfd, Timestamp now)
{
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof(howmany));
    cout << "TimerQueue::handleRead() " << howmany << " at " << now.toString() << endl;
    if(n != sizeof(howmany))
    {
        cout << "TimerQueue::handleRead() reads " << n << "bytes instead of 8" << endl;
    }
}

void resetTimerfd(int timerfd, Timestamp expiration)
{
    struct itimerspec newValue;
    struct itimerspec oldValue;
    memset(&newValue, 0, sizeof(newValue));
    memset(&oldValue, 0, sizeof(oldValue));
    newValue.it_value = howMuchTimeFromNow(expiration);
    int32_t ret = ::timerfd_settime(timerfd, 0, &newValue, & oldValue);
    if(ret)
    {
        cout << "Error " << "timerfd_settime()" << endl;
    }
}
TimerQueue::TimerQueue(EventLoop* loop) 
: loop_(loop),
  timerfd_(createTimerfd()),
  timerfdChannel_(loop, timerfd_),
  timers_(),
  callingExpiredTimers_(false)
{
    timerfdChannel_.setReadCallBack(
        std::bind(&TimerQueue::handleRead, this));
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue()
{
    timerfdChannel_.disableAll();
    timerfdChannel_.remove();
    ::close(timerfd_);
    for(const auto& timer : timers_)
    {
        delete timer.second;
    }
}

TimerId TimerQueue::addTimer(TimerCallback cb, Timestamp when, double interval)
{
    Timer* timer = new Timer(std::move(cb), when, interval);
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
    return TimerId(timer, timer->sequence());
}
void TimerQueue::addTimerInLoop(Timer* timer)
{
    loop_->assertInLoopThread();
    bool earliestChanged = insert(timer);
    if(earliestChanged)
    {
        resetTimerfd(timerfd_, timer->expiration());
    }
}

void TimerQueue::cancelInLoop(TimerId timerId)
{
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());
    
}

bool TimerQueue::insert(Timer* timer)
{
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());
    bool earliestChanged = false;
    Timestamp when = timer->expiration();
    TimerList::iterator it = timers_.begin();
    if(it == timers_.end() || when < it->first)
    {
        earliestChanged = true;
    }
    {
        std::pair<TimerList::iterator, bool> result = timers_.insert(Entry(when, timer));
        assert(result.second);
        (void) result;
    }
    {
        std::pair<ActiveTimerSet::iterator, bool> result = activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
        assert(result.second);
        (void)result;
    }
    assert(timers_.size() == activeTimers_.size());
    return earliestChanged;
}
void TimerQueue::handleRead()
{

}
void TimerQueue::cancel(TimerId timerId)
{

}