#ifndef __LINUX_NET_UTIL_H__
#define __LINUX_NET_UTIL_H__

#ifdef _MSC_VER
// linux only
#else

#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace LinuxNetUtil
{
    /*
    SetNonblocking

    @param [out] int& out_errno : 出错时的错误码

    @return 0 : success
    @return -1 : failed, check error reason from out_errno, normally it will need to shutdown sockfd
    */
    int SetNonblocking(const int in_socketfd, int &out_errno);

    /*
    CreateListenSocket

    @param [out] int& out_errno : 出错时的错误码

    @return 0 : success
    @return -1 : failed, check error reason from out_errno
    */
    int CreateListenSocket(const unsigned int in_port, const int in_backlog, int &out_errno);

    /*
    CreateConnectSocket

    @param [out] int& out_errno : 出错时的错误码

    @return 0 : success
    @return -1 : failed, check error reason from out_errno
    */
    int CreateConnectSocket(const string &in_serverIp, const unsigned int in_port, int &out_errno);

    /*
    Accept client Connection

    @param [in] const int in_listenfd : listen socket fd
    @param [out] int& out_clientfd : accepted client socket fd
    @param [out] string& out_remoteAddr : remote Internet address
    @param [out] uint16_t& out_remotePort : remote Port number
    @param [out] int& out_errno : 出错时的错误码
    */
    int AcceptConn(const int in_listenfd, int &out_clientfd, string &out_remoteAddr, uint16_t &out_remotePort);

    /*
    读取数据

    @param const int sockFd : socket file handler
    @param std::vector<uint8_t>& out_recvData : 接收到的数据
    @param [out] int& out_errno : 出错时的错误码

    @return 0 : 读取正常
    @return -1 : 读取失败，错误原因查看返回的errno，一般情况需关闭sockfd
    */
    int RecvData(const int sockFd, std::vector<uint8_t> &out_recvData, int &out_errno);

    /*
    发送数据

    @param [in] const int sockFd : socket file handler
    @param [in] std::vector<uint8_t>& in_sendData : 待发送的数据
    @param [out] size_t& out_byteTransferred : 发送的数据长度
    @param [out] int& out_errno : 出错时的错误码

    @return 0 : 发送数据已写入缓冲区
    @return 1 : 缓冲区长度不足
    @return -1 : 失败，错误原因查看返回的errno，一般情况需关闭sockfd
    */
    int SendData(const int sockFd, std::vector<uint8_t> &in_sendData, uint64_t &out_byteTransferred, int &out_errno);

    int Add_EpollWatch_NewClient(const int in_epollfd, const int in_sockfd, int &out_errno);

    int Mod_EpollWatch_Send(const int in_epollfd, const int in_sockfd, int &out_errno);

    int Mod_EpollWatch_CancellSend(const int in_epollfd, const int in_sockfd, int &out_errno);
}

#endif
#endif // __LINUX_NET_UTIL_H__
