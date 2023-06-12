#include "Epoll_SendQueue.h"

#ifdef _MSC_VER
// linux only
#else

Epoll_SendQueue::Epoll_SendQueue(void)
{
}

Epoll_SendQueue::~Epoll_SendQueue(void)
{
}

/*
将待发送数据加入发送队列的队尾

@param [in] std::shared_ptr<std::vector<uint8_t>>& in_pSendData : 待发送数据

@return
*/
void Epoll_SendQueue::AddSend(std::shared_ptr<std::vector<uint8_t>> &in_pSendData)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    m_dataNeedSend.push_back(in_pSendData);
}

/*
由发送队列队首获取待发送数据

@param [out] std::shared_ptr<std::vector<uint8_t>>& out_pSendData : 待发送数据

@return 1 : 获取了待发送数据
@return 0 : 无待发送数据
*/
int Epoll_SendQueue::GetSend(std::shared_ptr<std::vector<uint8_t>> &out_pSendData)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (0 == m_dataNeedSend.size())
    {
        return 0;
    }

    out_pSendData = m_dataNeedSend.front();
    m_dataNeedSend.pop_front();

    return 1;
}

/*
将因缓冲区容量不够的待发送数据加入发送队列的队首

@param [in] std::shared_ptr<std::vector<uint8_t>>& in_pSendData : 待发送数据

@return
*/
void Epoll_SendQueue::FailSendRestore(std::shared_ptr<std::vector<uint8_t>> &in_pSendData)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    m_dataNeedSend.push_front(in_pSendData);
}

/*
清除所有待发送数据

@param

@return
*/
void Epoll_SendQueue::CloseAllSends(void)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    m_dataNeedSend.clear();
}

/*
获取待发送数据数量

@param

@return
*/
size_t Epoll_SendQueue::NumOutstandingSends(void)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    return m_dataNeedSend.size();
}

#endif
