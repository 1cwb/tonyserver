#include "EventLoopThread.h"
#include <iostream>
using namespace std;

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb, const std::string& name)
  : loop_(nullptr),
    exiting_(false),
    thread_(std::bind(&EventLoopThread::threadFunc, this), name),
    callback_(cb)
{
    
}
EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if(loop_)
    {
        loop_->quit();
        thread_.join();
    }
}
EventLoop* EventLoopThread::startLoop()
{
    EventLoop* loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this]()->bool {return loop_ != nullptr;});
        loop = loop_;
    }
    return loop;
}
void EventLoopThread::threadFunc()
{
    EventLoop loop;
    cout << "this thread id " << thread_.get_id();
    if(callback_)
    {
        callback_(&loop);
    }
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }
    loop.loop();
    //exit
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}