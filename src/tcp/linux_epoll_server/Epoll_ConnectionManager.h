#ifndef __EPOLL_CONNECTION_MANAGER_H__
#define __EPOLL_CONNECTION_MANAGER_H__

#ifdef _MSC_VER
// linux only
#else

#include <map>
#include <memory>

#include "Epoll_Socket_Connection.h"

/*
Connection Manager.
*/
class Epoll_ConnectionManager
{
    //
    // Members
    //
protected:
    typedef std::map<uint64_t, std::shared_ptr<Epoll_Socket_Connection>> EpollConnMap_t;

    EpollConnMap_t m_connMap;

    std::mutex m_mutex;

    //
    // Functions
    //
public:
    Epoll_ConnectionManager(void);
    virtual ~Epoll_ConnectionManager(void);

    void AddConnection(std::shared_ptr<Epoll_Socket_Connection> client);

    bool RemoveConnection(uint64_t clientId);

    std::shared_ptr<Epoll_Socket_Connection> GetConnection(uint64_t clientId);

    void CloseAllConnections();

    int GetAllConnectionIds(std::vector<uint64_t> &out_vIds);

    /*
    将待发送数据加入发送队列的队尾，并通知epoll订阅socket epollout事件

    @param [in] uint64_t cid : client connection id
    @param [in] std::shared_ptr<std::vector<uint8_t>>& in_pSendData : 待发送数据
    @param [out] int& out_errno  : 出错的errno
    @param [out] int& out_cid : 出错的client connection id
    @param [out] int& out_sockfd : 出错的client socket fd

    @return 0 : 成功
    @return -1 : 失败，此情况有可能是epoll handle失效，或者socketfd失效(此时需要关闭客户端连接)
    */
    int Send(uint64_t cid, std::shared_ptr<std::vector<uint8_t>> &in_data, int &out_errno, int &out_cid, int &out_sockfd);
};

#endif
#endif
