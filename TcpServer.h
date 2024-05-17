#pragma once

#include "ServerSocket.h"
#include "Eventloop.h"
#include "Channel.h"
#include "Acceptor.h" 
#include <map>
#include "ThreadPool.h"
#include "Connection.h"
#include <memory>
#include <mutex>

class TcpServer
{
private:
    std::unique_ptr<Eventloop> mainloop_;                 // 主事件循环
    std::vector<std::unique_ptr<Eventloop>> subloops_;    // 存放从事件的容器
    Acceptor acceptor_;
    

    int threadnum_;                                    // 线程池的大小，即从事件循环的个数
    ThreadPool threadpool_;              // 线程池

    std::mutex mmutex_; // 保护conns_的互斥锁。
    std::map<int, spConnection> conns_;

    std::function<void(spConnection)> newconnection_;
    std::function<void(spConnection)> closeconnection_;
    std::function<void(spConnection)> errorconnection_;
    std::function<void(spConnection, std::string &message)> message_;
    std::function<void(spConnection)> sendcomplete_;
    std::function<void(Eventloop *)> epolltimeout_;

    

public:
    TcpServer(const std::string &ip, const uint16_t port, int threadnum = 3);
    ~TcpServer();

    void start();
    void stop(); //停止事件循环
    void newconnection(std::unique_ptr<ServerSocket> clientsock); // 用于处理新客户端连接请求
    void closeconnection(spConnection conn);                 // 关闭客户端的连接，在connection类中回调此函数
    void errorconnection(spConnection conn);                  // 客户端的连接错误，在connection类中回调此函数
    void message(spConnection conn, std::string message);     // 处理客户端请求报文，在connect中回调
    void sendcomplete(spConnection conn);                       // 数据发送完成后，在Connection类中回调此函数
    void epolltimeout(Eventloop *loop);                     // 处理epoll超时事件

    void setnewconnection(std::function<void(spConnection)> fn);   // 用于处理新客户端连接请求
    void setcloseconnection(std::function<void(spConnection)> fn); // 关闭客户端的连接，在connection类中回调此函数
    void seterrorconnection(std::function<void(spConnection)> fn); // 客户端的连接错误，在connection类中回调此函数
    void setmessage(std::function<void(spConnection, std::string &)> fn);          // 处理客户端请求报文，在connect中回调
    void setsendcomplete(std::function<void(spConnection)> fn);                    // 数据发送完成后，在Connection类中回调此函数
    void setepolltimeout(std::function<void(Eventloop *)> fn);                     // 处理epoll超时事件
    void removeconn(int fd);                                                       // 删除conns_中的Connection对象，在EventLoop::handletimer()中将回调此函数。
};


