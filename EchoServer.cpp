#include "EchoServer.h"
#include <iostream>

EchoServer::EchoServer(const std::string &ip, const uint16_t port, int subthreadnum, int worktheadnum)
    : tcpserver_(ip, port, subthreadnum), threadpool_(worktheadnum, "WORKS")
{
    tcpserver_.setnewconnection(std::bind(&EchoServer::HandleNewConnection, this,  std::placeholders::_1));
    tcpserver_.setcloseconnection(std::bind(&EchoServer::HandleClose, this, std::placeholders::_1));
    tcpserver_.seterrorconnection(std::bind(&EchoServer::HandleError, this, std::placeholders::_1));
    tcpserver_.setmessage(std::bind(&EchoServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
    tcpserver_.setsendcomplete(std::bind(&EchoServer::HandleSendComplete, this, std::placeholders::_1));
    //tcpserver_.setepolltimeout(std::bind(&EchoServer::HandleTimeOut, this, std::placeholders::_1));
    
    //std::cout << "业务类构造成功" << std::endl;
}

EchoServer::~EchoServer()
{
    //printf("EchoServer已析构");
}

void EchoServer::Start()
{
    tcpserver_.start();
}
void EchoServer::Stop() //停止服务
{
    //停止工作线程
    threadpool_.stop();
    printf("工作线程已停止 \n");
    //停止io线程（事件循环）
    tcpserver_.stop();
}
// 处理新客户端连接请求，在TcpServer类中回调此函数。
void EchoServer::HandleNewConnection(spConnection conn)
{
    //printf("HandleNewConnection() thear is %d.\n", syscall(SYS_gettid));
    std::cout << "New Connention Come in" << std::endl;
}
void EchoServer::HandleClose(spConnection conn) // 关闭客户端的连接，在TcpServer类中回调此函数。
{
    std::cout << "EchoServer conn closed." << std::endl;
}
void EchoServer::HandleError(spConnection conn) // 客户端的连接错误，在TcpServer类中回调此函数。
{
    std::cout << "EchoServer conn error." << std::endl;
}

//处理客户端的请求报文，在tcpserver类中回调此函数
void EchoServer::HandleMessage(spConnection conn, std::string &message) // 处理客户端的请求报文，在TcpServer类中回调此函数。
{
    //printf("EchoServer::HandleMessage() thear is %d.\n", syscall(SYS_gettid));

    
    if (threadpool_.size() == 0) //如果没有工作线程，直接调用处理函数
    {
        OnMessage(conn, message);
    }
    else
    {
        threadpool_.addtask(std::bind(&EchoServer::OnMessage, this, conn, message));
    }
}
void EchoServer::HandleSendComplete(spConnection conn) // 数据发送完成后，在TcpServer类中回调此函数。
{
    std::cout << "Message send complete." << std::endl;
}

void EchoServer::HandleTimeOut(Eventloop *loop) // epoll_wait()超时，在TcpServer类中回调此函数。
{
    std::cout << "EchoServer timeout." << std::endl;
}

void EchoServer::OnMessage(spConnection conn, std::string &message) // 处理客户端的请求报文
{
    message = "reply:" + message; // 回显业务。
    conn->send(message, message.size());
}