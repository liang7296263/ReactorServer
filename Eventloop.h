#pragma once
#include "ServerEpoll.h"
#include <functional>
#include <memory>
#include <unistd.h>
#include <sys/syscall.h>
#include <queue>
#include <mutex>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <map>
#include "Connection.h"
#include <atomic>

class Channel;
class ServerEpoll;
class Connection;
using spConnection = std::shared_ptr<Connection>;

class Eventloop
{
private:
    int timetvl_; // 闹钟时间间隔，单位：秒。。
    int timeout_; // Connection对象超时的时间，单位：秒。
    std::unique_ptr<ServerEpoll> ep_;
    std::function<void (Eventloop *)> epolltimeoutcallback_;
    pid_t threadid_;   //事件循环所在线程的id
    std::queue<std::function<void()>> taskqueue_; //事件循环线程被eventfd唤醒后执行的任务队列
    std::mutex mutex_;                            //任务队列同步的互斥锁

    int wakeupfd_;                                //用于唤醒事件循环线程的eventfd

    std::unique_ptr<Channel> wakechannel_;        //eventfd的Channel;

    int timefd_;                                  //声明定时器的fd
    std::unique_ptr<Channel> timechannel_;        //定时器的channel
    bool mainloop_;                               //true 是主事件循环，false是从事件循环
    std::mutex mmutex_;                           // 保护conns_的互斥锁。
    std::map<int, spConnection> conns_;           // 存放运行在该事件循环上全部的Connection对象。
    std::function<void(int)> timercallback_;      // 删除TcpServer中超时的Connection对象，将被设置为TcpServer::removeconn()
    std::atomic_bool stop_;                       //用于停止事件循环的标志位，初始值为false，如果设置为true，表示停止循环

public:
    Eventloop(bool mainloop, int timetvl = 30, int timeout = 80);
    ~Eventloop();

    void run();
    void stop(); //停止事件循环
    //ServerEpoll *ep();

    void updatechannel(Channel *ch);
    void removeChannel(Channel *ch); // 把channel从红黑树上删除
    void setepolltimeoutcallback(std::function<void(Eventloop *)> fn); // 设置epoll_wait()超时的回调函数。

    bool isinloopthread();           //判断当前线程是否为事件循环线程

    void queueinloop(std::function<void()> fn); //把任务添加到队列中

    void wakeup();                      //用eventfd唤醒事件循环的函数

    void handwakeup();                  //唤醒之后的操作

    void handletimer();                 // 闹钟响时执行的函数。
    void newconnection(spConnection conn); // 把Connection对象保存在conns_中。
    void settimercallback(std::function<void(int)> fn); // 将被设置为TcpServer::removeconn()
};


