#include "Epoll.h"
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <cstring>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <deque>
#include <queue>
#include "Util.h"
#include "Logging.h"

#include <arpa/inet.h>
#include <iostream>

using namespace std;

const int32_t kNew = -1;
const int32_t kAdded = 1;
const int32_t kDeleted = 2;

Epoll::Epoll(EventLoop* loop)
: epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
  events_(kInitEventListSize),
  ownerLoop_(loop),
  threadId_(this_thread::get_id())
{
    if(epollfd_ < 0)
    {
        cout << "Epoll error fd = " << epollfd_ << endl;
    }
}

Epoll::~Epoll()
{
    ::close(epollfd_);
}

Timestamp Epoll::poll(int32_t timeoutMs, ChannelList* activeChannels)
{
    size_t numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int32_t>(events_.size()),timeoutMs);
    int savedErrno = errno;
    Timestamp now(Timestamp::now());
    if(numEvents > 0)
    {
        fillActiveChannels(numEvents, activeChannels);
        if(numEvents == events_.size())
        {
            events_.resize(events_.size() * 2);
        }
    }
    else if(numEvents == 0)
    {
        cout << " nothing happend " << endl;
    }
    else
    {
        if(savedErrno != EINTR)
        {
            errno = savedErrno;
            cout << "Epoll error happend" << strerror(errno) << endl;
        }
    }
    return now;
}

void Epoll::fillActiveChannels(size_t numEvents, ChannelList* activeChannels) const
{
    assert(numEvents <= events_.size());
    for(size_t i = 0; i < numEvents; ++i)
    {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

void Epoll::update(int32_t operation, Channel* channel)
{
    struct  epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events = channel->events();
    event.data.ptr = channel;
    int32_t fd = channel->fd();
    if(::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        if(operation == EPOLL_CTL_DEL)
        {
            cout << "Error : epoll_ctl op = DEL " << "fd = " << fd << endl;
        }
        cout << "Error : epoll_ctl op " << "fd = " << fd << endl;
    }
}
void Epoll::updateChannel(Channel* channel)
{
    assertInLoopThread();
    const int32_t index = channel->index();
    if(index == kNew || index == kDeleted)
    {
        int32_t fd = channel->fd();
        if(index == kNew)
        {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        }
        else
        {
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }
        channel->setIndex(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else
    {
        int32_t fd = channel->fd();
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(index == kAdded);
        if(channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->setIndex(kDeleted);
        }
        else
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}
void Epoll::removeChannel(Channel* channel)
{
    assertInLoopThread();
    int32_t fd = channel->fd();
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->isNoneEvent());
    const int32_t index = channel->index();
    assert(index == kAdded || index == kDeleted);
    size_t n = channels_.erase(fd);
    assert(n == 1);
    if(index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->setIndex(kNew);
}
bool Epoll::hasChannel(Channel* channel) const
{
    ChannelMap::const_iterator it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel;
}
