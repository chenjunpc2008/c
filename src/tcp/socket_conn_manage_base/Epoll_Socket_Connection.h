#ifndef __EPOLL_SOCKET_CONNECTION_H__
#define __EPOLL_SOCKET_CONNECTION_H__

#ifdef _MSC_VER
// linux only
#else

#include <memory.h>
#include <mutex>
#include <stdint.h>
#include <string>
#include <sys/socket.h>
#include <vector>

#include "tcp/socket_conn_manage_base/ConnectionInformation.h"

#include "Epoll_SendQueue.h"

/*
epoll 相关的socket connection类
*/
class Epoll_Socket_Connection
{
    //
    // Members
    //
  public:
    // socket file handle
    int m_socketfd;

    // id for this connection
    uint64_t m_id;

    // conn info
    struct ConnectionInformation m_cinfo;

    // epoll file handle for this socket have been hooked
    int m_epollfd;

    // mutex for NextSendSeq
    std::mutex m_mutex_NextSendSeq;

    // mutex for message Send queue
    std::mutex m_mutex_sendQue;

    // is epoll out event set, true : set, false : not set
    bool m_bIsEpoutSet;

    // 发送消息缓存
    Epoll_SendQueue m_sendQueue;

    // 接收消息缓存
    // struct simutgw::SocketUserMessage m_rcvSocketMsg;

  protected:
    // 下一条发送消息序号
    uint64_t m_ui64NextSendSeq;

    //
    // Functions
    //
  public:
    Epoll_Socket_Connection(const int socketfd, const uint64_t cid,
                            const struct ConnectionInformation &cinfo, const int epollfd);

    virtual ~Epoll_Socket_Connection();

    /*
    取下一条发送消息序号
    */
    bool GetSeq(uint64_t clientId, uint64_t &out_ui64Seq)
    {
        std::lock_guard<std::mutex> lock(m_mutex_NextSendSeq);

        out_ui64Seq = ++m_ui64NextSendSeq;

        return true;
    }

    /*
    将待发送数据加入发送队列的队尾，并通知epoll订阅socket epollout事件

    @param [in] std::shared_ptr<std::vector<uint8_t>>& in_pSendData : 待发送数据
    @param [out] int& out_errno  : 出错的errno
    @param [out] int& out_cid : 出错的client connection id
    @param [out] int& out_sockfd : 出错的client socket fd

    @return 0 : 成功
    @return -1 : 失败，此情况有可能是epoll handle失效，或者socketfd失效(此时需要关闭客户端连接)
    */
    int AddSend(std::shared_ptr<std::vector<uint8_t>> &in_pSendData, int &out_errno, int &out_cid,
                int &out_sockfd);

    int LoopSend(uint64_t &out_byteTransferred, int &out_errno, int &out_cid, int &out_sockfd);

  protected:
    // 禁止使用默认构造函数
    Epoll_Socket_Connection();
};

#endif
#endif
