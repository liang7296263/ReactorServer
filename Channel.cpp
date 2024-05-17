#include "Channel.h"
#include <iostream>

Channel::Channel(Eventloop *loop, int fd) : loop_(loop), fd_(fd)
{
    //std::cout << " Channel 构造成功" << std::endl;
}
Channel::~Channel()
{

}

int Channel::fd()
{
    return fd_;
}            // 返回fd_成员。
void Channel::useet()
{
    events_ = events_ | EPOLLET;
}        // 采用边缘触发。
void Channel::enablereading()
{
    events_ |= EPOLLIN;
    loop_->updatechannel(this);
} // 让epoll_wait()监视fd_的读事件。
void Channel::disablereading()
{
    events_ &= ~EPOLLIN;
    loop_->updatechannel(this);
} // 取消读事件。
void Channel::enablewriting()
{
    events_ |= EPOLLOUT;
    loop_->updatechannel(this);
}  // 让epoll_wait()监视fd_的写事件。
void Channel::disablewriting()
{
    events_ &= ~EPOLLOUT;
    loop_->updatechannel(this);
} // 取消写事件。

void Channel::disableall() // 取消全部的事件
{
    events_ = 0;
    loop_->updatechannel(this);
}
void Channel::remove() // 从事件循环中删除Channel
{
    disableall(); //可不写
    loop_->removeChannel(this);
}
void Channel::setinepoll(bool inepoll)
{
    inepoll_ = true;
}    // 把inepoll_成员的值设置为true。
void Channel::setrevents(uint32_t ev)
{
    revents_ = ev;
} // 设置revents_成员的值为参数ev。
bool Channel::inpoll()
{
    return inepoll_;
}                // 返回inepoll_成员。
uint32_t Channel::events()
{
    return events_;
}            // 返回events_成员。
uint32_t Channel::revents()
{
    return revents_;
}           // 返回revents_成员。



//事件处理函数，epoll_wait返回的时候，执行它
void Channel::handleevent()
{
    if (revents_ & EPOLLRDHUP) // 如果对方已关闭，有些系统检测不到，可以使用EPOLLIN，recv()返回0。
    {

        closecallback_();
    }                                              //  普通数据  带外数据
    else if (revents_ & (EPOLLIN | EPOLLPRI)) // 接收缓冲区中有数据可以读。
    {
        readcallback_();
    }
    else if (revents_ & EPOLLOUT) // 有数据需要写，暂时没有代码，以后再说。
    {
        writecallback_();
    }
    else // 其它事件，都视为错误。
    {
        errorcallback_();
    }
}


void Channel::setreadcallback(std::function<void()> fn)
{
    readcallback_ = fn;
}


void Channel::setclosecallback(std::function<void()> fn)
{
    closecallback_ = fn;
}
void Channel::seterrorcallback(std::function<void()> fn)
{
    errorcallback_ = fn;
}

void Channel::setwritecallback(std::function<void()> fn)
{
    writecallback_ = fn;
}


