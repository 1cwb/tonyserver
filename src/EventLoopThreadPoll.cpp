#include "EventLoopThreadPoll.h"
#include <iostream>
#include <assert.h>
using namespace std;

EventLoopThreadPoll::EventLoopThreadPoll(EventLoop* baseLoop, const std::string& name)
 : baseLoop_(baseLoop),
   name_(name),
   started_(false),
   numThreads_(0),
   next_(0)
{

}
EventLoopThreadPoll::~EventLoopThreadPoll()
{
    for(auto& it : threads_)
    {
        if(it != nullptr)
        {
            delete it;
        }
    }
    started_ = false;
}

void EventLoopThreadPoll::start(const ThreadInitCallback& cb)
{
    assert(!started_);
    baseLoop_->assertInLoopThread();
    started_ = true;

    for(int32_t i = 0; i < numThreads_; i++)
    {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof(buf), "%s%d", name_.c_str(), i);
        EventLoopThread* t = new EventLoopThread(cb, buf);
        threads_.push_back(t);
        loops_.push_back(t->startLoop());
    }
    if(numThreads_ == 0 && cb)
    {
        cb(baseLoop_);
    }
}
EventLoop* EventLoopThreadPoll::getNextLoop()
{
    baseLoop_->assertInLoopThread();
    assert(started_);

    EventLoop* loop = baseLoop_;

    if(!loops_.empty())
    {
        loop = loops_[next_];
        ++ next_;
        if(next_ >= loops_.size())
        {
            next_ = 0;
        }
    }
    return loop;
}
EventLoop* EventLoopThreadPoll::getLoopForHash(size_t hashCode)
{
    baseLoop_->assertInLoopThread();
    EventLoop* loop = baseLoop_;

    if(!loops_.empty())
    {
        loop = loops_[hashCode % loops_.size()];
    }
    return loop;
}
const std::vector<EventLoop*>* EventLoopThreadPoll::getAllLoops() const
{
    baseLoop_->assertInLoopThread();
    assert(started_);
    if(loops_.empty())
    {
        return nullptr;
    }
    else
    {
        return &loops_;
    }
}