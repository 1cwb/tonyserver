#pragma once
#include "noncopyable.h"
#include <functional>
#include <memory>
#include <sys/epoll.h>
#include "Timestamp.h"

class EventLoop;
class Channel : public NonCopyAble
{
public:
    using EventCallback = std::function<void ()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop* loop, int32_t fd);
    ~Channel();

    void handleEvent(Timestamp receiveTime);
    void setReadCallBack(ReadEventCallback cb)
    {
        readCallback_ = std::move(cb);
    }
    void setWriteCallback(EventCallback cb)
    {
        writeCallback_ = std::move(cb);
    }
    void setCloseCallback(EventCallback cb)
    {
        closeCallback_ = std::move(cb);
    }
    void setErrorCallback(EventCallback cb)
    {
        errorCallback_ = std::move(cb);
    }

    int32_t fd() const
    {
        return fd_;
    }
    int32_t events() const
    {
        return events_;
    }
    void set_revents(int32_t revent)
    {
        revents_ = revent;
    }
    bool isNoneEvent() const
    {
        return events_ == kNoneEvent;
    }

    void enableReading()
    {
        events_ |= kReadEvent;
        update();
    }
    void disableReading()
    {
        events_ &= ~kReadEvent;
        update();
    }
    void enableWriting()
    {
        events_ |= kWriteEvent;
        update();
    }
    void disableWriting()
    {
        events_ &= ~kWriteEvent;
        update();
    }
    void disableAll()
    {
        events_ = kNoneEvent;
        update();
    }
    bool isWriting() const
    {
        return events_ & kWriteEvent;
    }
    bool isReading() const
    {
        return events_ & kReadEvent;
    }

    std::string revensToString() const;
    std::string eventsToString() const;

    void doNotLogHup()
    {
        logHup_ = false;
    }
    EventLoop* ownerLoop()
    {
        return loop_;
    }
    void remove();
    int32_t index() const {return index_;}
    void setIndex(int32_t idx) {index_ = idx;}
private:
    static std::string eventsToString(int32_t fd, int32_t ev);
    void update();
    void handleEventWithGuard(Timestamp receiveTime);
private:
    EventLoop*          loop_;
    const int32_t       fd_;
    int32_t             events_;
    int32_t             revents_;
    int32_t             index_;
    bool                logHup_;

    bool                eventHandling_;
    bool                addedToLoop_;
    ReadEventCallback   readCallback_;
    EventCallback       writeCallback_;
    EventCallback       closeCallback_;
    EventCallback       errorCallback_;
private:
    static const int32_t kNoneEvent = 0;
    static const int32_t kReadEvent = EPOLLIN | EPOLLPRI | EPOLLET;
    static const int32_t kWriteEvent = EPOLLOUT;
};