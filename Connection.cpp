#include "Connection.h"

Connection::Connection(Eventloop *loop, std::unique_ptr<ServerSocket> clientsocket)
    : loop_(loop), clientSocket_(std::move(clientsocket)), disconnect_(false), clientchannel_(new Channel(loop_, clientSocket_->fd()))
{
    // 为新客户端连接准备读事件，并添加到epoll中。
    clientchannel_->setreadcallback(std::bind(&Connection::onmessage, this));
    clientchannel_->setclosecallback(std::bind(&Connection::closecallback, this));
    clientchannel_->seterrorcallback(std::bind(&Connection::errorcallback, this));
    clientchannel_->setwritecallback(std::bind(&Connection::writecallback, this));
    clientchannel_->useet();         // 设置边缘触发
    clientchannel_->enablereading(); // 让epoll_wait()监视fd_的读事件。
    //std::cout << " connection 构造成功" << std::endl;
}

Connection::~Connection()
{
    //delete clientchannel_;
    //delete clientSocket_;
}

int Connection::fd() const
{
    return clientSocket_->fd();
}// 返回fd
std::string Connection::ip() const
{
    return clientSocket_->ip();
} // 返回ip
uint16_t Connection::port() const
{
    return clientSocket_->port();
} // 返回端口

void Connection::closecallback()   //TCP连接断开的回调函数，供Channel回调
{

    disconnect_ = true;
    clientchannel_->remove();
    closeconnection_(shared_from_this());
} 
void Connection::errorcallback()   // TCP连接发生错误，供Channel回调
{
    disconnect_ = true;
    clientchannel_->remove();
    errorconnection_(shared_from_this());
}

void Connection::setcloseconnection(std::function<void(spConnection)> fn)
{
    closeconnection_ = fn;
}
void Connection::seterrorconnection(std::function<void(spConnection)> fn)
{
    errorconnection_ = fn;
}

// 处理对端发送过来的消息
void Connection::onmessage()
{
    char buffer[1024];
    while (true) // 由于使用非阻塞IO，一次读取buffer大小数据，直到全部的数据读取完毕。
    {

        bzero(&buffer, sizeof(buffer));
        ssize_t reads = read(fd(), buffer, sizeof(buffer));

        if (reads > 0)
        {

            inputbuffer_.append(buffer, reads);
        }
        else if (reads == -1 && errno == EINTR) // 读取数据的时候被信号中断，继续读取。
        {
            continue;
        }
        else if (reads == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) // 全部的数据已读取完毕。
        {
            std::string message;

            while (true) // 从接收缓冲区中拆分出客户端的请求消息。
            {
                if (inputbuffer_.pickmessage(message) == false)
                    break;

                printf("message (fd=%d):%s\n", fd(), message.c_str());
                lastatime_ = Timestamp::now(); // 更新Connection的时间戳。

                onmessagecallback_(shared_from_this(), message); // 回调TcpServer::onmessage()处理客户端的请求消息。
            }
            break;
        }
        else if (reads == 0) // 客户端连接已断开
        {
            // clientchannel_->remove();
            closecallback();
            break;
        }
    }
}

// 发送数据, 不管在哪个线程中，都调用这个函数
void Connection::send(std::string &data, size_t size)
{
    if (disconnect_ == true) { printf("客户端连接已断开了，send直接返回\n"); return; }

    if (loop_->isinloopthread()) //判断当前线程是否为事件循环线程（io线程）
    {
        //如果是，直接执行发送数据的操作
        //std::cout << "send()在事件循环中" << std::endl;
        sendinloop(data, size);
    }
    else
    {
        //如果不是，把发送数据的操作交给io线程去执行
        //std::cout << "send()不在事件循环中" << std::endl;
        //sendinloop(data, size);
        loop_->queueinloop(std::bind(&Connection::sendinloop, this, data, size));
        //std::cout << "执行queueinloop函数" << std::endl;
    }
}
// 发送数据，如果当前线程时io线程，直接调用此函数，如果是工作线程，把此函数传给io线程
void Connection::sendinloop(std::string &data, size_t size)
{
    outputbuffer_.appendwithsep(data.data(), size); // 把需要发送的数据保存到Connection的发送缓冲区中。
    clientchannel_->enablewriting();          // 注册写事件。
    
}


void Connection::setonmessagecallback(std::function<void(spConnection, std::string &)> fn)
{
    onmessagecallback_ = fn;
}

void Connection::setsendcompletecallback(std::function<void(spConnection)> fn)
{
    sendcompletecallback_ = fn;
}

// 处理写事件的回调函数，供channel回调
void Connection::writecallback()
{
    //printf("recv(eventfd=%d):%s\n", fd(), outputbuffer_.data());
    int writen = ::send(fd(), outputbuffer_.data(), outputbuffer_.size(), 0); //尝试把发送缓冲区的数据全部发送出去
    
    if (writen > 0) outputbuffer_.erase(0, writen);
    if (outputbuffer_.size() == 0) 
    {
        clientchannel_->disablewriting();
        sendcompletecallback_(shared_from_this());
    }

} 

// 判断TCP连接是否超时（空闲太久）。
bool Connection::timeout(time_t now, int val)
{
    return now - lastatime_.toint() > val;
}
