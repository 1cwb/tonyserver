#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/fcntl.h>
#include "EventLoop.h"
#include "Timestamp.h"
#include "EventLoopThreadPoll.h"
using namespace std;

#define ERROR_CHECK(ret) \
    if(ret != 0) \
    { \
         printf("Error (%s)in %s()%d \n",strerror(errno),__FUNCTION__,__LINE__); \
         exit(1);\
    }

#define FD_CHECK(ret) \
    if(ret < 0) \
    { \
         printf("Error FD(%d) %s()%d \n",ret ,__FUNCTION__,__LINE__); \
         exit(1); \
    }

int getconnectfd(int listenfd)
{
    struct sockaddr_in claddr;
    socklen_t claddrsize = sizeof(struct sockaddr_in);
    return accept(listenfd, (struct sockaddr*)&claddr, &claddrsize);
}

void setnonblocking(int sock)
{
    int opts;
    opts=fcntl(sock,F_GETFL);
    if(opts<0)
    {
        perror("fcntl(sock,GETFL)");
        exit(1);
    }
    opts = opts|O_NONBLOCK;
    if(fcntl(sock,F_SETFL,opts)<0)
    {
        perror("fcntl(sock,SETFL,opts)");
        exit(1);
    }
}

int epollctrl(const int epfd, int op, const int fd, const uint32_t ev)
{
    struct epoll_event sev;
    sev.data.fd = fd;
    sev.events = ev;
    return epoll_ctl(epfd, op, fd, &sev);
}

void reuseaddr(int sfd)
{
    int val = 1;
    int ret = setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,(void *)&val,sizeof(int));
     if(ret == -1)
     {
         printf("setsockopt");
         exit(1);
     }   
}

int main(int argc, char** argv)
{
    int sesockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    FD_CHECK(sesockfd);
    //setnonblocking(sesockfd);
    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(5879);
    reuseaddr(sesockfd);
    ERROR_CHECK(bind(sesockfd, (const struct sockaddr*)&servAddr, sizeof(struct sockaddr)));

    ERROR_CHECK(listen(sesockfd, 20));
    
    EventLoop mevLoop;
    EventLoopThreadPoll poll(&mevLoop, "server");
    poll.setThreadNum(3);
    poll.start();
    Channel* listenChannel = new Channel(&mevLoop, sesockfd);
    listenChannel->enableReading();
    listenChannel->setReadCallBack([&listenChannel,&poll](Timestamp timestamp){
        cout << "xxxxx sb connect now" << endl;
        int32_t connfd = getconnectfd(listenChannel->fd());
        if(connfd > 0)
        {
            EventLoop* const iloop = poll.getNextLoop();
            iloop->runInLoop([=](){
                Channel* newch = new Channel(iloop,connfd);
                newch->enableReading();
                newch->setReadCallBack([=](Timestamp timestamp){
                    char buf[1024] = {0};
                    ::read(newch->fd(),buf,1024);
                    cout << "revieve data: " << buf << endl;
                });
                newch->setCloseCallback([=](){
                    cout << newch->fd() << " disconnect " << endl;
                });
            });
        }
    });
    mevLoop.loop();
    delete listenChannel;
    close(sesockfd);
    return 0;
}