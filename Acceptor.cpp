#include "Acceptor.h"
#include "Connection.h"

Acceptor::Acceptor(Eventloop *loop, const std::string ip, uint16_t port)
    : loop_(loop), serverSocket_(setlistensock()), acceptchannel_ (loop_, serverSocket_.fd())
{
    //serverSocket_ = new ServerSocket(setlistensock()); // 创建socket类，并创建监听socket，初始化该类

    inetAddress servaddr(ip, port); // 创建封装好的地址类

    // 设置监听socket属性
    serverSocket_.setkeepalive(true);
    serverSocket_.settcpnodelay(true);
    serverSocket_.settreuseaddr(true);
    serverSocket_.settreuseport(true);

    

    // 为监听socket绑定ip和端口
    serverSocket_.bind(servaddr);
    serverSocket_.listen();
    //std::cout << " serverSocket_成功" << std::endl;

    //acceptchannel_ = new Channel(loop_, serverSocket_.fd());
    // std::bind函数返回一个函数对象，参数1填需要绑定的函数，参数2填函数封装在哪里（地址），参数3填绑定函数需要的参数
    acceptchannel_.setreadcallback(std::bind(&Acceptor::newconnection, this));
    //std::cout << "设置newconnection成功" << std::endl;
    acceptchannel_.enablereading();
    //std::cout << "设置enablereading成功" << std::endl;
    //std::cout << " acceptor 构造成功" << std::endl;
}

Acceptor::~Acceptor()
{
    //delete serverSocket_;
    //delete acceptchannel_;
    
}

void Acceptor::newconnection()
{
    inetAddress clientaddr; // 客户端的地址和协议。

    // 注意，clientsock只能new出来，不能在栈上，否则析构函数会关闭fd。
    std::unique_ptr<ServerSocket> clientsock(new ServerSocket(serverSocket_.accept(clientaddr)));
    clientsock->setipport(clientaddr.ip(), clientaddr.port());

    newconnectioncb_(std::move(clientsock));
    
} // 用于处理新客户端连接请求
void Acceptor::setnewconnectioncb(std::function<void(std::unique_ptr<ServerSocket>)> fn) // 设置处理新客户端连接请求的回调函数。
{
    newconnectioncb_ = fn;
}
