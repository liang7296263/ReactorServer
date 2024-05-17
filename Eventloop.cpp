#include "Eventloop.h"
#include <iostream>

int createtimerfd(int sec = 30) //创建定时器的函数
{
    int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK); // 创建timerfd。
    struct itimerspec timeout;                                             // 定时时间的数据结构。
    memset(&timeout, 0, sizeof(struct itimerspec));
    timeout.it_value.tv_sec = sec; // 定时时间，固定为5，方便测试。
    timeout.it_value.tv_nsec = 0;
    timerfd_settime(tfd, 0, &timeout, 0);
    return tfd;
}

Eventloop::Eventloop(bool mainloop, int timetvl, int timeout): ep_(new ServerEpoll), mainloop_(mainloop), 
            timetvl_(timetvl), timeout_(timeout),
            wakeupfd_(eventfd(0, EFD_NONBLOCK)), wakechannel_(new Channel(this, wakeupfd_)),
            timefd_(createtimerfd(timeout_)), timechannel_(new Channel(this, timefd_)), stop_(false)
{
    wakechannel_->setreadcallback(std::bind(&Eventloop::handwakeup, this));
    wakechannel_->enablereading();
   
    timechannel_->setreadcallback(std::bind(&Eventloop::handletimer, this));
    timechannel_->enablereading();
}

Eventloop::~Eventloop() 
{
    //delete ep_;
}

void Eventloop::run()
{
    threadid_ = syscall(SYS_gettid); //获取事件循环所在线程的id
    while (stop_ == false)
    {
        std::vector<Channel *> channels = ep_->loop(10*1000);
        
        //判断返回的channel是否为空，如果为空，表示超时，回调itcpserver中的epolltimeout函数
        if (channels.size() == 0)
        {
            epolltimeoutcallback_(this);
        }
        else
        {
            // 若有事件发生，遍历向量
            for (auto &ch : channels)
            {
            ch->handleevent();
            }
        }
    }
}

void Eventloop::stop() // 停止事件循环
{
    stop_ = true;
    wakeup();
}
bool Eventloop::isinloopthread() // 判断当前线程是否为事件循环线程
{
    return threadid_ == syscall(SYS_gettid);
}

//更新或添加channel
void Eventloop::updatechannel(Channel *ch)
{
    ep_->updatechannel(ch);
}

void Eventloop::setepolltimeoutcallback(std::function<void(Eventloop *)> fn)
{
    epolltimeoutcallback_ = fn;
}

void Eventloop::removeChannel(Channel *ch) // 把channel从红黑树上删除
{
    ep_->removeChannel(ch);
}

void Eventloop::queueinloop(std::function<void()> fn) // 把任务添加到队列中
{
    {
        std::lock_guard<std::mutex> gd(mutex_); //给任务队列加锁
        taskqueue_.push(fn);                    //任务入队
    }
    //唤醒事件循环
    wakeup();
}

void Eventloop::wakeup() // 用eventfd唤醒事件循环的函数
{
    uint64_t val = 1;
    write(wakeupfd_, &val, sizeof(val));
    
}

void Eventloop::handwakeup() // 唤醒之后的操作
{
    uint64_t val;
    read(wakeupfd_, &val, sizeof(val)); // 从eventfd中读取出数据，如果不读取，eventfd的读事件会一直触发。

    std::function<void()> fn;

    std::lock_guard<std::mutex> gd(mutex_); // 给任务队列加锁。

    // 执行队列中全部的发送任务。
    while (taskqueue_.size() > 0)
    {
        fn = std::move(taskqueue_.front()); // 出队一个元素。
        taskqueue_.pop();
        fn(); // 执行任务。
    }
}

// 闹钟响时执行的函数。
void Eventloop::handletimer()
{
    // 重新计时。
    struct itimerspec timeout; // 定时时间的数据结构。
    memset(&timeout, 0, sizeof(struct itimerspec));
    timeout.it_value.tv_sec = timetvl_; // 定时时间，固定为5，方便测试。
    timeout.it_value.tv_nsec = 0;
    timerfd_settime(timefd_, 0, &timeout, 0);

    if (mainloop_)
    {
        
    }
    else
    {
        time_t now = time(0); // 获取当前时间。

        for (auto ii = conns_.begin(); ii != conns_.end();)
        {
            if (ii->second->timeout(now, timeout_))
            {
                {
                    std::lock_guard<std::mutex> gd(mmutex_);
                    conns_.erase(ii++);
                }
                timercallback_(ii->first);    
            }else
            {
                ii++;
            }
        }
    }
    
    
}

// 把Connection对象保存在conns_中。
void Eventloop::newconnection(spConnection conn)
{
    std::lock_guard<std::mutex> gd(mmutex_);
    conns_[conn->fd()] = conn;
}

// 将被设置为TcpServer::removeconn()
void Eventloop::settimercallback(std::function<void(int)> fn)
{
    timercallback_ = fn;
}