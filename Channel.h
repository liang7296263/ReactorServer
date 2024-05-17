#pragma once

#include <sys/epoll.h>

#include "ServerSocket.h"
#include "inetAddress.h"
#include <functional>
#include "Eventloop.h"
#include <memory>


class Eventloop;

class Channel
{
private:
    int fd_ = -1;                // channel拥有的fd，channel和fd时一对一的关系
    Eventloop *loop_ = nullptr; // Channel对应的事件循环，Channel与EventLoop是多对一的关系，一个Channel只对应一个EventLoop.
    bool inepoll_ = false;       // channel是否已添加到红黑树上，如果一添加，调用epoll_ctl()的时候用add，否则用mod
    uint32_t events_ = 0;        // fd_需要监视的事件。listenfd和clientfd需要监视EPOLLIN，clientfd还可能需要监视EPOLLOUT。
    uint32_t revents_ = 0;       // fd_已发生的事件。
    std::function<void()> readcallback_; // fd_读事件的回调函数（函数对象）

    std::function<void()> closecallback_;
    std::function<void()> errorcallback_;

    std::function<void()> writecallback_; // fd_x写事件的回调函数（函数对象）

public:
    Channel(Eventloop *loop, int fd);
    ~Channel();

    int fd();                     // 返回fd_成员。
    void useet();                 // 采用边缘触发。
    void enablereading();         // 让epoll_wait()监视fd_的读事件。
    void disablereading();         // 取消读事件。
    void enablewriting();         // 让epoll_wait()监视fd_的写事件。
    void disablewriting();         // 取消写事件。
    void disableall();             // 取消全部的事件
    void remove();                 // 从事件循环中删除Channel
    void setinepoll(bool inepoll); // 把inepoll_成员的值设置为true。
    void setrevents(uint32_t ev); // 设置revents_成员的值为参数ev。
    bool inpoll();                // 返回inepoll_成员。
    uint32_t events();            // 返回events_成员。
    uint32_t revents();           // 返回revents_成员。
    void handleevent();           // 事件处理函数，epoll_wait()返回的时候，执行它
    //void onmessage();                                 //处理对端发送过来的消息
    void setreadcallback(std::function<void()> fn);   //给函数对象成员赋值

    void setclosecallback(std::function<void()> fn);
    void seterrorcallback(std::function<void()> fn);

    void setwritecallback(std::function<void()> fn);

    
    
};