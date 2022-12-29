#include "EventLoop.h"
#include <algorithm>
#include <sys/eventfd.h>
#include <iostream>
#include <unistd.h>
#include <assert.h>
#include <mutex>
using namespace std;

__thread EventLoop* t_loopInThisThread = nullptr;
const int kPollTimeMs = 10000;

int32_t createEventfd()
{
    int32_t evtfd  = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtfd < 0)
    {
        cout << "Error Failed in eventfd";
        abort();
    }
    return evtfd;
}

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
    return t_loopInThisThread;
}

EventLoop::EventLoop()
 : looping_(false),
 quit_(false),
 eventHandling_(false),
 callingPendingFunctors_(false),
 iteration_(0),
 threadId_(this_thread::get_id()),
 epoll_(new Epoll(this)),
 wakeupFd_(createEventfd()),
 wakeupChannel_(new Channel(this, wakeupFd_)),
 currentActiveChannel_(nullptr)
{
    cout << "EventLoop Created " << this << "in thread " << threadId_ << endl;
    if(t_loopInThisThread)
    {
        cout << "Another EventLoop " << t_loopInThisThread << "exists in this thread" << threadId_ << endl;
    }
    else
    {
        t_loopInThisThread = this;
    }
    if(wakeupChannel_)
    {
        wakeupChannel_->setReadCallBack(std::bind(&EventLoop::handleRead, this));
        wakeupChannel_->enableReading();
    }
}

EventLoop::~EventLoop()
{
    cout << "EventLoop " << this << " of thread " << threadId_ << " destruct in thread " << this_thread::get_id() << endl;
    if(wakeupChannel_)
    {
        wakeupChannel_->disableAll();
        wakeupChannel_->remove();
        delete wakeupChannel_;
        wakeupChannel_ = nullptr;
    }
    if(epoll_)
    {
        delete epoll_;
        epoll_ = nullptr;
    }

    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop()
{
    cout << "EventLoop " << this << "start looping" << endl;
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;

    while(!quit_)
    {
        activeChannel_.clear();
        epoll_->poll(kPollTimeMs, &activeChannel_);
        ++ iteration_;
        eventHandling_ = true;
        for(Channel* channel : activeChannel_)
        {
            currentActiveChannel_ = channel;
            currentActiveChannel_->handleEvent();
        }
        currentActiveChannel_ = nullptr;
        eventHandling_ = false;
        doPendingFunctors();
    }
    looping_ = false;
}

void EventLoop::quit()
{
    quit_ = true;
    if(!isInLoopThread())
    {
        wakeup();
    }
}

void EventLoop::runInLoop(Functor cb)
{
    if(isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(Functor cb)
{
    {
        lock_guard<mutex> lock(mutex);
        pendingFunctors_.push_back(std::move(cb));
    }
    if(!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();
    }
}

size_t EventLoop::queueSize() const
{
    lock_guard<mutex> lock(mutex);
    return pendingFunctors_.size();
}

void EventLoop::updateChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    epoll_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    if(eventHandling_)
    {
        assert(currentActiveChannel_ == channel ||
            std::find(activeChannel_.begin(), activeChannel_.end(), channel) == activeChannel_.end());
    }
    epoll_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    return epoll_->hasChannel(channel);     
}

void EventLoop::abortNotInLoopThread()
{
  cout << "EventLoop::abortNotInLoopThread - EventLoop " << this
            << " was created in threadId_ = " << threadId_
            << ", current thread id = " <<  this_thread::get_id() << endl;
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
    if(n != sizeof(one))
    {
        cout << "EventLoop::wakeup() writes " << n << " bytes instead of 8" << endl;
    }
}

void EventLoop::handleRead()
{
  uint64_t one = 1;
  ssize_t n = ::read(wakeupFd_, &one, sizeof one);
  if (n != sizeof one)
  {
    cout << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
  }
}

void EventLoop::doPendingFunctors()
{
  std::vector<Functor> functors;
  callingPendingFunctors_ = true;

  {
  lock_guard<mutex> lock(mutex_);
  functors.swap(pendingFunctors_);
  }

  for (const Functor& functor : functors)
  {
    functor();
  }
  callingPendingFunctors_ = false;
}

void EventLoop::printActiveChannels() const
{
  for (const Channel* channel : activeChannel_)
  {
    cout << "{" << channel->revensToString() << "} ";
  }
}
