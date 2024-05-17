#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <sys/epoll.h>
#include <netinet/tcp.h> // TCP_NODELAY需要包含这个头文件
#include <iostream>
#include "inetAddress.h"
#include "ServerSocket.h"
#include "ServerEpoll.h"
#include "Eventloop.h"
#include "TcpServer.h"
#include "EchoServer.h"
#include <signal.h>
#include <iostream>

EchoServer *eco;

void Stop(int sig)
{
    std::cout << "sig=" << sig << std::endl;
    // 调用EchoServer::Stop()停止服务
    eco->Stop();
    std::cout << "eco已停止" << std::endl;
    delete eco;
    std::cout << "delete eco" << std::endl;
    exit(0);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cout << "请输入 ./程序名称 ip 端口号" << std::endl;
        std::cout << "示例：./server 192.168.0.142 10007" << std::endl;
        return -1;
    }
    signal(SIGTERM, Stop); // 信号15，系统kill或killall命令默认发送的信号。
    signal(SIGINT, Stop);  // 信号2，按Ctrl+C发送的信号。

    eco = new EchoServer(argv[1], atoi(argv[2]),3,3);
    
    eco->Start();

    return 0;
}