#include "inetAddress.h"


inetAddress::inetAddress(const std::string &ip, uint16_t port)
{
    addr_.sin_family = AF_INET;                    // IPv4网络协议的套接字类型。
    addr_.sin_addr.s_addr = inet_addr(ip.c_str()); // 服务端用于监听的ip地址。
    addr_.sin_port = htons(port);                  // 服务端用于监听的端口。
}

inetAddress::inetAddress(const sockaddr_in addr) : addr_(addr)
{

}

inetAddress::~inetAddress()
{
    
}

const char *inetAddress::ip() const
{
    return inet_ntoa(addr_.sin_addr);
}

uint16_t inetAddress::port() const
{
    return ntohs(addr_.sin_port);
}

const sockaddr *inetAddress::addr() const
{
    return (sockaddr *)&addr_;
}

void inetAddress::setaddr(sockaddr_in clientaddr)
{
    addr_ = clientaddr;
}