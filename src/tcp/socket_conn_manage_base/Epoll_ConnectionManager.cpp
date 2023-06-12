#include "Epoll_ConnectionManager.h"

#ifdef _MSC_VER
// linux only
#else

#include <sys/socket.h>

#include <unistd.h>

using namespace std;

Epoll_ConnectionManager::Epoll_ConnectionManager(void) {}

Epoll_ConnectionManager::~Epoll_ConnectionManager(void) {}

void Epoll_ConnectionManager::AddConnection(std::shared_ptr<Epoll_Socket_Connection> client)
{
    static const string ftag("AddConnection() ");

    std::lock_guard<std::mutex> lock(m_mutex);

    std::pair<EpollConnMap_t::iterator, bool> ret_cid =
        m_connMap.insert(std::make_pair(client->m_id, client));

    bool inserted = ret_cid.second;
    if (inserted)
    {
        // cout << ftag << " id=" << client->m_id << " info " <<
        // client->m_cinfo.ToString() << endl;
    }
}

bool Epoll_ConnectionManager::RemoveConnection(uint64_t clientId)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    EpollConnMap_t::const_iterator cit = m_connMap.find(clientId);
    if (m_connMap.end() == cit)
    {
        return false;
    }

    m_connMap.erase(clientId);

    return true;
}

std::shared_ptr<Epoll_Socket_Connection> Epoll_ConnectionManager::GetConnection(uint64_t clientId)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    EpollConnMap_t::iterator itr = m_connMap.find(clientId);

    if (m_connMap.end() != itr)
    {
        return itr->second;
    }

    return shared_ptr<Epoll_Socket_Connection>();
}

void Epoll_ConnectionManager::CloseAllConnections()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    EpollConnMap_t::iterator itr = m_connMap.begin();
    while (m_connMap.end() != itr)
    {
        shutdown(itr->second->m_socketfd, SHUT_RDWR);
        close(itr->second->m_socketfd);
        itr->second->m_socketfd = -1;
        ++itr;
    }

    m_connMap.clear();
}

int Epoll_ConnectionManager::GetAllConnectionIds(std::vector<uint64_t> &out_vIds)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    EpollConnMap_t::iterator itr = m_connMap.begin();
    while (m_connMap.end() != itr)
    {
        out_vIds.push_back(itr->first);
        ++itr;
    }

    return 0;
}

/*
将待发送数据加入发送队列的队尾，并通知epoll订阅socket epollout事件

@param [in] uint64_t cid : client connection id
@param [in] std::shared_ptr<std::vector<uint8_t>>& in_pSendData : 待发送数据
@param [out] int& out_errno  : 出错的errno
@param [out] int& out_cid : 出错的client connection id
@param [out] int& out_sockfd : 出错的client socket fd

@return 0 : 成功
@return -1 : 失败，此情况有可能是epoll
handle失效，或者socketfd失效(此时需要关闭客户端连接)
*/
int Epoll_ConnectionManager::Send(uint64_t cid, std::shared_ptr<std::vector<uint8_t>> &in_data,
                                  int &out_errno, int &out_cid, int &out_sockfd)
{
    static const string ftag("Epoll_ConnectionManager::Send() ");

    std::shared_ptr<Epoll_Socket_Connection> pClientConn = GetConnection(cid);
    if (nullptr == pClientConn)
    {
        // cout << ftag << "connection manage couldn't find id=" << cid << endl;
        return -1;
    }

    int iRes = pClientConn->AddSend(in_data, out_errno, out_cid, out_sockfd);
    if (0 != iRes)
    {
        return -1;
    }

    return 0;
}

#endif
