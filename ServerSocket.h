#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "inetAddress.h"


int setlistensock();

class ServerSocket
{
private:
    const int fd_;
    std::string ip_;
    uint16_t port_;
public:
    ServerSocket(int fd);
    ~ServerSocket();

    int fd() const; //返回fd

    void settreuseaddr(bool on);  //设置linentfd属性
    void settreuseport(bool on);  // 设置linentfd属性
    void settcpnodelay(bool on);  // 设置linentfd属性
    void setkeepalive(bool on);   // 设置linentfd属性

    void bind(const inetAddress &); // 绑定ip/端口
    void listen(int nn = 128);            // 设置监听
    int accept(inetAddress &clientaddr);  // 接受连接

    std::string ip() const;         // 返回ip
    uint16_t port() const;          // 返回端口

    void setipport(const std::string &ip, uint16_t port);               //设置客户端ip和端口成员
};

