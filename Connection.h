#pragma once

#include "ServerSocket.h"
#include "Channel.h"
#include "Eventloop.h"
#include <functional>
#include "inetAddress.h"
#include "Buffer.h"
#include <memory>
#include <atomic>
#include <sys/syscall.h>
#include "Timestamp.h"

class Eventloop;
class Channel;
class Connection;
using spConnection = std::shared_ptr<Connection>;



class Connection : public std::enable_shared_from_this<Connection>
{
private:
    Eventloop *loop_;                                                   // Connection对应的事件循环，在构造函数中传入。
    std::unique_ptr<ServerSocket> clientSocket_;                        // 与客户端通讯的Socket。
    std::unique_ptr<Channel> clientchannel_;                            // Connection对应的channel，在构造函数中创建。
    Buffer inputbuffer_;  // 接收缓冲区。
    Buffer outputbuffer_; // 发送缓冲区。
    std::atomic_bool disconnect_; // 是否已经断开连接

    std::function<void(spConnection)> closeconnection_; // 关闭fd_的回调函数，将回调TcpServer::closeconnection()。
    std::function<void(spConnection)> errorconnection_; // fd_发生了错误的回调函数，将回调TcpServer::errorconnection()。
    std::function<void(spConnection, std::string &)> onmessagecallback_; // 处理报文的回调函数，将回调tcpserver::conmessage();
    std::function<void(spConnection)> sendcompletecallback_;             // 数据发送完毕之后处理函数，回调自tcpserver;
    
    Timestamp lastatime_; // 时间戳，创建connection对象时为当前时间，每接收到一个报文，把时间戳更新为当前时间
public:
    Connection(Eventloop *loop, std::unique_ptr<ServerSocket> clientsocket);
    ~Connection();
    
    int fd() const;         // 返回fd
    std::string ip() const; // 返回ip
    uint16_t port() const;  // 返回端口

    void closecallback();   // tcp连接关闭（断开）的回调函数，供channel类回调
    void errorcallback();   // tcp连接错误的回调函数， 供channel回调i
    void onmessage();       // 处理对端发送过来的消息
    void writecallback();   // 处理写事件的回调函数，供channel回调

    void setcloseconnection(std::function<void(spConnection)> fn);
    void seterrorconnection(std::function<void(spConnection)> fn);
    void setonmessagecallback(std::function<void(spConnection, std::string &)> fn);
    void setsendcompletecallback(std::function<void(spConnection)> fn);

    // 发送数据, 不管在哪个线程中，都调用这个函数
    void send(std::string &data, size_t size);
    // 发送数据，如果当前线程时io线程，直接调用此函数，如果是工作线程，把此函数传给io线程
    void sendinloop(std::string &data, size_t size);

    bool timeout(time_t now, int val); // 判断TCP连接是否超时（空闲太久）。
};

