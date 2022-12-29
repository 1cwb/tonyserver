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
    
//epoll
    cout << "server run" << endl;
    EventLoop mevLoop;


    Channel* listenChannel = new Channel(&mevLoop, sesockfd);
    listenChannel->enableReading();
    listenChannel->setReadCallBack([&](){
        cout << "xxxxx sb connect now" << endl;
    });
    cout << "server run ----" << endl;
    mevLoop.loop();
/*
    vector<Channel*> connectChannel;
    while(true)
    { 
        connectChannel = mepoll.poll();
        if(connectChannel.size() > 0)
        {
            for(auto& ch : connectChannel)
            {
                if(ch == listenChannel)
                {
                    int connfd = getconnectfd(ch->getFd());
                    FD_CHECK(connfd);
                    Channel* mch = new Channel(&mepoll, connfd, EPOLLPRI | EPOLLRDHUP| EPOLLIN | EPOLLET);
                    mch->setHandleRead([](Channel* xch){
                        printf("Events = %x\n",xch->getRecvEvents());
                        if(xch->getRecvEvents() & ( EPOLLRDHUP))
                        {
                            char* buff[256] = {0};
                            read(xch->getFd(),buff,256);
                            printf("buff is %s\n",buff);
                            printf("client close  %d\n",xch->getFd());
                            xch->getEpoll()->epollDel(xch);
                            delete xch;
                        }
                        else if(xch->getRecvEvents() & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
                        {
                            
                            char* buff[256] = {0};
                            ssize_t n = read(xch->getFd(),buff,256);
                        printf("buff is %s\n",buff);
                        } 
                    });
                    ch->getEpoll()->epollAdd(mch);
                }
                else
                {
                    ch->handleRead();
                }
            }
        }
    }
    */
    close(sesockfd);
    return 0;
}