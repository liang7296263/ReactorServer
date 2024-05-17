#include "ServerSocket.h"

int setlistensock()  //创建监听socket
{
    int listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    if (listenfd < 0)
    {
        // perror("socket() failed"); exit(-1);
        printf("%s:%s:%d listen socket create error:%d\n", __FILE__, __FUNCTION__, __LINE__, errno);
        exit(-1);
    }
    return listenfd;
}

ServerSocket::ServerSocket(int fd) : fd_(fd)  //构造函数
{
}

ServerSocket::~ServerSocket()                 //析构函数
{
    ::close(fd_);
}

int ServerSocket::fd() const //返回fd
{
    return fd_;
}

void ServerSocket::settreuseaddr(bool on)
{
    ::setsockopt(fd_, SOL_SOCKET,SO_REUSEADDR,&on,static_cast<socklen_t>(sizeof on));
}// 设置linentfd属性

void ServerSocket::settreuseport(bool on)
{
    setsockopt(fd_, SOL_SOCKET, TCP_NODELAY, &on, static_cast<socklen_t>(sizeof on));
} // 设置linentfd属性
void ServerSocket::settcpnodelay(bool on)
{
    setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &on, static_cast<socklen_t>(sizeof on));
} // 设置linentfd属性
void ServerSocket::setkeepalive(bool on)
{
    setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &on, static_cast<socklen_t>(sizeof on));
}  // 设置linentfd属性

void ServerSocket::bind(const inetAddress &servaddr)
{
    if (::bind(fd_, servaddr.addr(), sizeof(sockaddr)) < 0)
    {
        perror("bind() failed");
        close(fd_);
        exit(-1);
    }

    setipport(servaddr.ip(), servaddr.port());
}
void ServerSocket::listen(int nn)
{
    if (::listen(fd_, nn) != 0) // 在高并发的网络服务器中，第二个参数要大一些。
    {
        perror("listen() failed");
        close(fd_);
        exit(-1);
    }
}
int ServerSocket::accept(inetAddress & clientaddr)
{
    sockaddr_in peeraddr;
    socklen_t len = sizeof(peeraddr);
    int clientfd = accept4(fd_, (sockaddr *)&peeraddr, &len, SOCK_NONBLOCK);

    clientaddr.setaddr(peeraddr); // 客户端的地址和协议。



    return clientfd;
}

std::string ServerSocket::ip() const
{
    return ip_;
}// 返回ip
uint16_t ServerSocket::port() const
{
    return port_;
} // 返回端口

void ServerSocket::setipport(const std::string &ip, uint16_t port)
{
    ip_ = ip;
    port_ = port;
}