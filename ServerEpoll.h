#pragma once

#include <stdio.h>
#include <sys/epoll.h>
#include "ServerSocket.h"
#include <vector>
#include "Channel.h"

class Channel;

class ServerEpoll
{
private:
    static const int MaxEvents = 100;
    int epollfd_ = -1;
    epoll_event evs_[MaxEvents];
public:
    ServerEpoll();
    ~ServerEpoll();

    //void addfd(int fd, uint32_t evern1);
    void updatechannel(Channel *ch); // 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件。
    void removeChannel(Channel *ch); // 把channel从红黑树上删除
    std::vector<Channel *> loop(int timeout = -1); // 运行epoll_wait()，等待事件的发生，已发生的事件用vector容器返回。
};
