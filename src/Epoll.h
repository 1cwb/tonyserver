#pragma once
#include <sys/epoll.h>
#include <memory>
#include <map>
#include <vector>
#include "Channel.h"
#include "HttpData.h"
#include "Timer.h"
#include <unistd.h>
#include "noncopyable.h"
#include <thread>
#include "Channel.h"
#include "Timestamp.h"

class EventLoop;
class Epoll : public NonCopyAble
{
public:
    using ChannelList = std::vector<Channel*>;
    Epoll(EventLoop* loop);
    ~Epoll();
    Timestamp poll(int32_t timeoutMs, ChannelList* activeChannels);
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel) const;
private:
    using EventList = std::vector<struct epoll_event>;

    static const int32_t kInitEventListSize = 16;

    void fillActiveChannels(size_t numEvents, ChannelList* activeChannels) const;

    void update(int32_t operation, Channel* channel);

    void assertInLoopThread() const
    {
        if(threadId_ != std::this_thread::get_id())
        {
            std::cout << "EventLoop::abortNotInLoopThread - EventLoop " << this
            << " was created in threadId_ = " << threadId_
            << ", current thread id = " <<  std::this_thread::get_id() << std::endl;
            ::abort();
        }
    }

    int32_t epollfd_;
    EventList events_;
    EventLoop* ownerLoop_;
    const std::thread::id threadId_;
    using ChannelMap = std::map<int32_t, Channel*>;
    ChannelMap channels_;
};