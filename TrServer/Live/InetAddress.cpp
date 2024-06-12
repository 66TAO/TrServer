#include "InetAddress.h"

Ipv4Address::Ipv4Address()
{

}

Ipv4Address::Ipv4Address(std::string ip, uint16_t port) :
    mIp(ip),                                        // 将传入的 IP 地址存储到成员变量 mIp 中
    mPort(port)                                     // 将传入的端口号存储到成员变量 mPort 中
{
    mAddr.sin_family = AF_INET;                         // 设置地址族为 IPv4
    mAddr.sin_addr.s_addr = inet_addr(ip.c_str());      // 将 IP 地址转换为网络字节序并存储到 sockaddr_in 结构体的 sin_addr 成员中
    mAddr.sin_port = htons(port);                       // 将端口号转换为网络字节序并存储到 sockaddr_in 结构体的 sin_port 成员中
}

void Ipv4Address::setAddr(std::string ip, uint16_t port)
{
    mIp = ip;
    mPort = port;
    mAddr.sin_family = AF_INET;
    mAddr.sin_addr.s_addr = inet_addr(ip.c_str());
    mAddr.sin_port = htons(port);
}

std::string Ipv4Address::getIp()
{
    return mIp;
}

uint16_t Ipv4Address::getPort()
{
    return mPort;
}

struct sockaddr* Ipv4Address::getAddr()
{
    return (struct sockaddr*)&mAddr;
}
