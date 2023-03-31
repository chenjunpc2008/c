#include "Epoll_Socket_Connection.h"

#ifdef _MSC_VER
// linux only
#else

#include <sys/socket.h>

#include "tcp/linux_socket_base/LinuxNetUtil.h"

Epoll_Socket_Connection::Epoll_Socket_Connection(const int socketfd,
                                                 const uint64_t cid, const struct ConnectionInformation &cinfo, const int epollfd)
    : m_socketfd(socketfd), m_id(cid), m_cinfo(cinfo), m_epollfd(epollfd),
      m_bIsEpoutSet(false),
      m_ui64NextSendSeq(0)
{
}

Epoll_Socket_Connection::~Epoll_Socket_Connection()
{
    // cout << "object dismember id=" << m_id << ", socketfd=" << m_socketfd
    //      << ", ip=" << m_cinfo.strRemoteIpAddress
    //      << ", port=" << m_cinfo.remotePortNumber << endl;

    if (0 < m_socketfd)
    {
        shutdown(m_socketfd, SHUT_RDWR);
        close(m_socketfd);
    }
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
int Epoll_Socket_Connection::AddSend(std::shared_ptr<std::vector<uint8_t>> &in_pSendData, int &out_errno, int &out_cid, int &out_sockfd)
{
    std::lock_guard<std::mutex> lock(m_mutex_sendQue);

    m_sendQueue.AddSend(in_pSendData);

    // 如果没有设置过out event
    if (!m_bIsEpoutSet)
    {
        int iErr = 0;

        int iRes = LinuxNetUtil::Mod_EpollWatch_Send(m_epollfd, m_socketfd, iErr);
        if (0 != iRes)
        {
            out_errno = iErr;
            out_cid = m_id;
            out_sockfd = m_socketfd;
            return -1;
        }

        m_bIsEpoutSet = true;
    }

    return 0;
}

int Epoll_Socket_Connection::LoopSend(uint64_t &out_byteTransferred, int &out_errno, int &out_cid, int &out_sockfd)
{
    std::lock_guard<std::mutex> lock(m_mutex_sendQue);

    uint64_t uiTotal_ByteTransferred = 0;

    std::shared_ptr<std::vector<uint8_t>> pSendData;
    int iRes = 0;
    int iErr = 0;

    // 循环发送直至清空缓冲区
    do
    {
        iRes = m_sendQueue.GetSend(pSendData);
        if (1 == iRes)
        {
            // 有待发送数据
            uint64_t uiByteTransferred = 0;

            int iResSend = LinuxNetUtil::SendData(m_socketfd, *pSendData, uiByteTransferred, iErr);
            if (0 == iResSend)
            {
                // 写入发送缓冲区成功
                uiTotal_ByteTransferred += uiByteTransferred;
            }
            else if (1 == iResSend)
            {
                // 缓冲区长度不足
                return 1;
            }
            else
            {
                out_errno = iErr;
                out_cid = m_id;
                out_sockfd = m_socketfd;
                return -1;
            }
        }
        else
        {
            // 无待发送数据
            // 取消out event
            iRes = LinuxNetUtil::Mod_EpollWatch_CancellSend(m_epollfd, m_socketfd, iErr);
            if (0 != iRes)
            {
                out_errno = iErr;
                out_cid = m_id;
                out_sockfd = m_socketfd;
                return -1;
            }

            m_bIsEpoutSet = false;
            break;
        }
    } while (0 < iRes);

    out_byteTransferred = uiTotal_ByteTransferred;

    return 0;
}

#endif
