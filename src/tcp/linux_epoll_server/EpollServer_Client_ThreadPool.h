#ifndef __EPOLL_CLIENT_THREADPOOL_H__
#define __EPOLL_CLIENT_THREADPOOL_H__

#ifdef _MSC_VER
// linux only
#else

#include <vector>
#include <stdint.h>

#include <memory>

#include "threadpool/thread_pool_base/ThreadPoolBase.h"
#include "EpollServer_Client_Thread.h"

class EpollServer_Core;

class EpollServer_Client_ThreadPool : public ThreadPoolBase<EpollServer_Client_Thread>
{
    //
    // Members
    //
protected:
    std::shared_ptr<EpollServer_EventHandler> m_pEventHandler;

    //
    // Functions
    //
public:
    EpollServer_Client_ThreadPool(const unsigned int uiNum, std::shared_ptr<EpollServer_EventHandler> pEventHandler)
        : ThreadPoolBase(uiNum), m_pEventHandler(pEventHandler)
    {
    }

    virtual ~EpollServer_Client_ThreadPool(void)
    {
    }

    /*
    创建线程池对象

    Return :
    0 -- 成功
    -1 -- 失败
    */
    virtual int InitPool(void)
    {
        static const std::string ftag("EpollServer_Client_ThreadPool::InitPool() ");

        m_pool_state = ThreadPool_Conf::STOPPED;

        unsigned int i = 0;
        for (i = 0; i < m_uiThreadMaxNums; ++i)
        {
            std::shared_ptr<EpollServer_Client_Thread> ptrThread =
                std::shared_ptr<EpollServer_Client_Thread>(new EpollServer_Client_Thread(i, m_pEventHandler));
            if (NULL == ptrThread)
            {
                if (nullptr != m_pEventHandler)
                {
                    std::string sDebug = ftag;
                    sDebug += "new ThreadBase() failed, NULL";
                    m_pEventHandler->OnErrorStr(sDebug);
                }

                return -1;
            }

            m_hdThreads.push_back(ptrThread);

            ++m_uiCurrThreadNums;
        }

        return 0;
    }

    /*
    新增用户

    @param const int in_clientfd : accepted client socket fd
    @param const string& in_remoteAddr : remote Internet address
    @param const uint16_t in_remotePort : remote Port number
    @param int& out_epollfd : socketfd挂钩的epoll file handle

    Return :
    0 -- 成功
    -1 -- 失败
    */
    int AddNewClient(const int in_clientfd, const std::string &in_remoteAddr, const uint16_t in_remotePort, int &out_epollfd)
    {
        static const std::string ftag("EpollServer_Client_ThreadPool::AddNewClient() ");

        int iRes = 0;
        int i = 0;

        // 最少任务的线程句柄
        std::vector<std::shared_ptr<EpollServer_Client_Thread>>::iterator itFewerTask;
        // 最少任务的线程任务量
        uint64_t uiFewerTaskNum = 0;

        // 先查找是否可以根据已有历史任务直接插入
        std::vector<std::shared_ptr<EpollServer_Client_Thread>>::iterator it = m_hdThreads.begin();
        for (i = 0, it = m_hdThreads.begin(); it != m_hdThreads.end(); ++it, ++i)
        {
            uint64_t uiTaskNum = 0;
            uiTaskNum = (*it)->GetCurrentClientNums();

            if (0 == i)
            {
                // 首次特殊处理
                uiFewerTaskNum = uiTaskNum;
                itFewerTask = it;
            }
            else
            {
                if (uiTaskNum < uiFewerTaskNum)
                {
                    // 此线程比之前的任务数更少
                    uiFewerTaskNum = uiTaskNum;
                    itFewerTask = it;
                }
            }

            continue;
        }

        // 插入至最少队列中
        iRes = (*itFewerTask)->AddNewClient(in_clientfd, out_epollfd);
        if (0 == iRes)
        {
            return 0;
        }
        else
        {
            if (nullptr != m_pEventHandler)
            {
                std::string sDebug = ftag;
                std::string strTran;
                sDebug += "AddIfHaveTask() failed, res=";
                sDebug += sof_string::itostr(iRes, strTran);

                m_pEventHandler->OnErrorStr(sDebug);
            }

            return -1;
        }
    }
};
#endif
#endif
