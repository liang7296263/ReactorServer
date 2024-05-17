#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>


class inetAddress
{
private:
    sockaddr_in addr_;
public:
    inetAddress() {};
    inetAddress(const std::string &ip, uint16_t port);
    inetAddress(const sockaddr_in addr);
    ~inetAddress();

    const char *ip() const;       
    uint16_t port() const;        
    const sockaddr *addr() const; 

    void setaddr(sockaddr_in clientaddr);
};

