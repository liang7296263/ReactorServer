#pragma once

#include "ServerSocket.h"
#include "Channel.h"
#include "Eventloop.h"
#include <functional>
#include "inetAddress.h"
#include <memory>


class Acceptor
{
private:
    Eventloop *loop_;
    ServerSocket serverSocket_;
    Channel acceptchannel_;
    std::function<void(std::unique_ptr<ServerSocket>)> newconnectioncb_;

public:
    Acceptor(Eventloop *loop, const std::string ip, uint16_t port);
    ~Acceptor();

    void newconnection(); // 用于处理新客户端连接请求

    void setnewconnectioncb(std::function<void(std::unique_ptr<ServerSocket>)> fn); // 设置处理新客户端连接请求的回调函数
};


