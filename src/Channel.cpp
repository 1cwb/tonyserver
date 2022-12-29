#include "Channel.h"
#include <unistd.h>
#include <assert.h>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <queue>
#include "Epoll.h"
#include "EventLoop.h"
#include "Util.h"


using namespace std;

Channel::Channel(EventLoop* loop, int32_t fd)
  : loop_(loop),
    fd_(fd),
    events_(kNoneEvent),
    revents_(kNoneEvent),
    index_(-1),
    logHup_(true),
    eventHandling_(false),
    addedToLoop_(false)
{
}
Channel::~Channel()
{
    assert(!eventHandling_);
    assert(!addedToLoop_);
    if(loop_->isInLoopThread())
    {
        assert(!loop_->hasChannel(this));
    }
}
void Channel::update()
{
    addedToLoop_ = true;
    loop_->updateChannel(this);
}
void Channel::remove()
{
    assert(isNoneEvent());
    addedToLoop_ = false;
    loop_->removeChannel(this);
}
void Channel::handleEvent()
{
    handleEventWithGuard();
}
void Channel::handleEventWithGuard()
{
    eventHandling_ = true;
    cout << revensToString() << endl;
    if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
    {
        if(logHup_)
        {
            cout << "Warninig fd = " << fd_ << "Channel::handle_event() POLLHUP" << endl;
        }
        if(closeCallback_)
        {
            closeCallback_();
        }
    }
    if(revents_ & EPOLLERR)
    {
        if(errorCallback_)
        {
            errorCallback_();
        }
    }
    if(revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
    {
        if(readCallback_)
        {
            readCallback_();
        }
    }
    if(revents_ & EPOLLOUT)
    {
        if(writeCallback_)
        {
            writeCallback_();
        }
    }
    eventHandling_ = false;
}
string Channel::revensToString() const
{
    return eventsToString(fd_, revents_);
}
string Channel::eventsToString() const
{
    return eventsToString(fd_, events_);
}
string Channel::eventsToString(int32_t fd, int32_t ev)
{
    std::ostringstream oss;
    oss << fd << ": ";
    if (ev & EPOLLIN)
        oss << "IN ";
    if (ev & EPOLLPRI)
        oss << "PRI ";
    if (ev & EPOLLOUT)
        oss << "OUT ";
    if (ev & EPOLLHUP)
        oss << "HUP ";
    if (ev & EPOLLRDHUP)
        oss << "RDHUP ";
    if (ev & EPOLLERR)
        oss << "ERR ";

    return oss.str();
}
