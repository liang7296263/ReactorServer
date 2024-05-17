#include "TcpServer.h"

TcpServer::TcpServer(const std::string &ip, const uint16_t port, int threadnum)
    : threadnum_(threadnum), mainloop_(new Eventloop(true)),
      acceptor_(mainloop_.get(), ip, port), threadpool_(threadnum, "IO")
{
    //mainloop_ = new Eventloop; // 创建主事件循环
    mainloop_->setepolltimeoutcallback(std::bind(&TcpServer::epolltimeout, this, std::placeholders::_1));

    //acceptor_ = new Acceptor(mainloop_, ip, port);
    acceptor_.setnewconnectioncb(std::bind(&TcpServer::newconnection, this, std::placeholders::_1 ));


    //创建从事件循环
    for (int i = 0; i < threadnum_; i++)
    {
        subloops_.emplace_back(new Eventloop(false, 5, 10));
        subloops_[i]->setepolltimeoutcallback(std::bind(&TcpServer::epolltimeout, this, std::placeholders::_1));
        // 设置清理空闲TCP连接的回调函数。
        subloops_[i]->settimercallback(std::bind(&TcpServer::removeconn, this, std::placeholders::_1));
        threadpool_.addtask(std::bind(&Eventloop::run, subloops_[i].get())); // 在线程池中运行从事件循环。
    }
}

TcpServer::~TcpServer()
{
}

void TcpServer::start()
{
    mainloop_->run();
}

void TcpServer::stop()
{
    //停止主事件循环
    mainloop_->stop();
    printf("主事件循环已停止 \n");
    //停止从事件循环
    for (int ii = 0; ii < threadnum_; ii++)
    {
        subloops_[ii]->stop();
    }
    printf("从事件循环已停止 \n");
    //停止io线程
    threadpool_.stop();
    printf("IO线程已停止 \n");
}                                                 // 停止事件循环
void TcpServer::newconnection(std::unique_ptr<ServerSocket> clientsock) // 处理客户端连接请求
{
    // 把新建的conn分配给从事件循环。
    spConnection conn(new Connection(subloops_[clientsock->fd() % threadnum_].get(), std::move(clientsock)));
    conn->setcloseconnection(std::bind(&TcpServer::closeconnection, this, std::placeholders::_1));
    conn->seterrorconnection(std::bind(&TcpServer::errorconnection, this, std::placeholders::_1));
    conn->setonmessagecallback(std::bind(&TcpServer::message, this, std::placeholders::_1, std::placeholders::_2));
    conn->setsendcompletecallback(std::bind(&TcpServer::sendcomplete, this, std::placeholders::_1));
    //printf("new connection(fd=%d,ip=%s,port=%d) ok.\n", conn->fd(), conn->ip().c_str(), conn->port());

    {
        std::lock_guard<std::mutex> gd(mmutex_);
        conns_[conn->fd()] = conn;   //把new出来的conn放入map容器中
    }
    subloops_[conn->fd() % threadnum_]->newconnection(conn);    //把conn存放到Eventloop的map容器中
    //printf("TcpServer::newconnection() thread is %d.\n", syscall(SYS_gettid));
    if (newconnection_) newconnection_(conn);

    //std::cout << " tcpserver 构造成功" << std::endl;
}

// 关闭客户端的连接，在connection类中回调此函数
void TcpServer::closeconnection(spConnection conn)
{
    //printf("client(eventfd=%d) disconnected.\n", conn->fd());
    if (closeconnection_) closeconnection_(conn);

    {
        std::lock_guard<std::mutex> gd(mmutex_);
        conns_.erase(conn->fd());
    }
    //delete conn;
}
// 客户端的连接错误，在connection类中回调此函数
void TcpServer::errorconnection(spConnection conn)
{
    //printf("client(eventfd=%d) error.\n", conn->fd());
    if (errorconnection_) errorconnection_(conn);
    {
        std::lock_guard<std::mutex> gd(mmutex_);
        conns_.erase(conn->fd());
    }
    //delete conn;
} 

void TcpServer::message(spConnection conn, std::string message)
{
    if(message_)message_(conn, message);
} // 处理客户端请求报文，在connect中回调

void TcpServer::sendcomplete(spConnection conn)
{
    //printf("send complete.\n");
    if(sendcomplete_) sendcomplete_(conn);

    //可根据业务需求添加其他代码
}

void TcpServer::epolltimeout(Eventloop *loop)
{
    //printf("epoll_wait()timeout.\n");
    if (epolltimeout_)
        epolltimeout_(loop);
    // 可根据业务需求添加其他代码
}

void TcpServer::setnewconnection(std::function<void(spConnection)> fn) // 用于处理新客户端连接请求
{
    newconnection_ = fn;
}
void TcpServer::setcloseconnection(std::function<void(spConnection)> fn) // 关闭客户端的连接，在connection类中回调此函数
{
    closeconnection_ = fn;
}
void TcpServer::seterrorconnection(std::function<void(spConnection)> fn) // 客户端的连接错误，在connection类中回调此函数
{
    errorconnection_ = fn;
}
void TcpServer::setmessage(std::function<void(spConnection, std::string &)> fn) // 处理客户端请求报文，在connect中回调
{
    message_ = fn;
}
void TcpServer::setsendcomplete(std::function<void(spConnection)> fn) // 数据发送完成后，在Connection类中回调此函数
{
    sendcomplete_ = fn;
}
void TcpServer::setepolltimeout(std::function<void(Eventloop *)> fn)                  // 处理epoll超时事件
{
    epolltimeout_ = fn;
}

// 删除conns_中的Connection对象，在EventLoop::handletimer()中将回调此函数。
void TcpServer::removeconn(int fd)
{
    {
        std::lock_guard<std::mutex> gd(mmutex_);
        conns_.erase(fd); // 从map中删除conn。
    }
}