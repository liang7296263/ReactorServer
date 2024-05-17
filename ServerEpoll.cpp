#include "ServerEpoll.h"
#include <iostream>

ServerEpoll::ServerEpoll()
{
    if ((epollfd_ = epoll_create(1)) == -1)
    {
        printf("epoll_create() failed(%d).\n", errno);
        exit(-1);
    }

    //std::cout << " epoll 构造成功" << std::endl;
}

ServerEpoll::~ServerEpoll()
{
    close(epollfd_);
}


void ServerEpoll::updatechannel(Channel *ch)
{
    epoll_event ev;
    ev.data.ptr = ch;
    ev.events = ch->events();
    //std::cout << "channel 结构设置成功" << std::endl;


    if (ch->inpoll())  //如果ch已经在树上 了， 
    {
        if (epoll_ctl(epollfd_, EPOLL_CTL_MOD, ch->fd(), &ev) == -1)
        {
            perror("epoll_ctl() failed.\n");
            exit(-1);
        }
    } 
    else
    {
        if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, ch->fd(), &ev) == -1)
        {
            perror("epoll_ctl() failed.\n");
            exit(-1);
        }
    }

    ch->setinepoll(true);
}
void ServerEpoll::removeChannel(Channel *ch) // 把channel从红黑树上删除
{
    if (ch->inpoll()) // 如果ch已经在树上 了，
    {
        // printf("removechannel()\n");
        if (epoll_ctl(epollfd_, EPOLL_CTL_DEL, ch->fd(), 0) == -1)
        {
            printf("epoll_ctl() failed(%d).\n", errno);
            exit(-1);
        }
    }
}

std::vector<Channel*> ServerEpoll::loop(int timeout)
{
    std::vector<Channel *> Channels;
    bzero(evs_, sizeof(evs_));
    int infds = ::epoll_wait(epollfd_, evs_, MaxEvents, timeout); //等待监视的fd有事件发生

    //返回失败
    if (infds < 0)
    {
        perror("epoll_wwit()");
        exit(-1);
    }

    // 返回超时
    if (infds == 0) 
    {
        return Channels;
    }

    // 若有事件发生，遍历evs数组
    for (int i = 0; i < infds; i++)
    {
        //evs.push_back(evs_[i]);
        Channel *ch = (Channel *) evs_[i].data.ptr;
        ch->setrevents(evs_[i].events);
        Channels.push_back(ch);
    }
    return Channels;
}


