#ifndef __EPOLL_SEND_QUEUE_H__
#define __EPOLL_SEND_QUEUE_H__

#ifdef _MSC_VER
// linux only
#else

#include <stdint.h>
#include <memory>
#include <mutex>
#include <vector>
#include <list>

/*
发送的缓存
*/
class Epoll_SendQueue
{
    //
    // Members
    //
protected:
    std::list<std::shared_ptr<std::vector<uint8_t>>> m_dataNeedSend;

    std::mutex m_mutex;

    //
    // Functions
    //
public:
    Epoll_SendQueue(void);
    virtual ~Epoll_SendQueue(void);

    /*
    将待发送数据加入发送队列的队尾

    @param [in] std::shared_ptr<std::vector<uint8_t>>& in_pSendData : 待发送数据

    @return
    */
    void AddSend(std::shared_ptr<std::vector<uint8_t>> &in_pSendData);

    /*
    由发送队列队首获取待发送数据

    @param [out] std::shared_ptr<std::vector<uint8_t>>& out_pSendData : 待发送数据

    @return 0 : 获取了待发送数据
    @return 1 : 无待发送数据
    */
    int GetSend(std::shared_ptr<std::vector<uint8_t>> &out_pSendData);

    /*
    将因缓冲区容量不够的待发送数据加入发送队列的队首

    @param [in] std::shared_ptr<std::vector<uint8_t>>& in_pSendData : 待发送数据

    @return
    */
    void FailSendRestore(std::shared_ptr<std::vector<uint8_t>> &in_pSendData);

    /*
    清除所有待发送数据

    @param

    @return
    */
    void CloseAllSends(void);

    /*
    获取待发送数据数量

    @param

    @return
    */
    size_t NumOutstandingSends(void);
};

#endif
#endif
