#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <vector>
#include <memory>

#include "ThreadBase.h"

/*
线程池
*/
template <typename T>
class ThreadPoolBase
{
    //
    // Members
    //
protected:
    // 最大允许的线程数量
    unsigned int m_uiThreadMaxNums;

    // 现在已有线程数量
    unsigned int m_uiCurrThreadNums;

    // 已有线程的句柄
    std::vector<std::shared_ptr<T>> m_hdThreads;

    volatile enum ThreadPool_Conf::ThreadState m_pool_state;

    // feedback
    pf_THREAD_POOL_ONEVENT m_pfOnEvent;
    pf_THREAD_POOL_ONERROR m_pfOnError;

    //
    // Functions
    //
public:
    explicit ThreadPoolBase(const unsigned int uiNum)
        : m_uiThreadMaxNums(uiNum), m_uiCurrThreadNums(0),
          m_pool_state(ThreadPool_Conf::ThreadState::STOPPED),
          m_pfOnEvent(nullptr),
          m_pfOnError(nullptr)
    {
    }

    virtual ~ThreadPoolBase(void)
    {
        StopPool();
    }

    bool SetMaxThreadNums(const unsigned int uiNum)
    {
        if (ThreadPool_Conf::ThreadState::STARTED == m_pool_state)
        {
            return false;
        }

        m_uiThreadMaxNums = uiNum;
        return true;
    }

    unsigned int GetCurrentThreadNum(void) const
    {
        return m_uiCurrThreadNums;
    }

    enum ThreadPool_Conf::ThreadState GetPoolState()
    {
        return m_pool_state;
    }

    void SetEventCb(pf_THREAD_POOL_ONEVENT pfEvent)
    {
        m_pfOnEvent = pfEvent;
    }

    void SetErrorCb(pf_THREAD_POOL_ONERROR pfError)
    {
        m_pfOnError = pfError;
    }

    /*
    预通知线程退出

    */
    void PreNoticeStop()
    {
        const std::string ftag("ThreadPoolBase::PreNoticeStop() ");

        if (0 == m_hdThreads.size())
        {
            m_pool_state = ThreadPool_Conf::ThreadState::STOPPED;
            return;
        }

        m_pool_state = ThreadPool_Conf::ThreadState::STOPPING;

        // 先通知关闭
        typename std::vector<std::shared_ptr<T>>::iterator it = m_hdThreads.begin();
        for (it = m_hdThreads.begin(); it != m_hdThreads.end(); ++it)
        {
            (*it)->NotifyStopThread_Immediate();
        }

        std::string sMsg(ftag);
        sMsg += "Notified all thread, wait a moment";
        if (nullptr != m_pfOnEvent)
        {
            m_pfOnEvent(sMsg);
        }
        std::cout << sMsg << std::endl;
    }

    /*
    启动线程池

    Return :
    0 -- 成功
    -1 -- 失败
    */
    virtual int StartPool(std::string *out_sErrMsg = nullptr)
    {
        const std::string ftag("ThreadPoolBase::StartPool() ");

        m_pool_state = ThreadPool_Conf::ThreadState::STARTED;

        typename std::vector<std::shared_ptr<T>>::iterator it = m_hdThreads.begin();
        for (it = m_hdThreads.begin(); it != m_hdThreads.end(); ++it)
        {
            if (nullptr != m_pfOnEvent)
            {
                (*it)->SetEventCb(m_pfOnEvent);
            }

            if (nullptr != m_pfOnError)
            {
                (*it)->SetErrorCb(m_pfOnError);
            }

            int iRes = (*it)->StartThread();
            if (0 != iRes)
            {
                if (nullptr != out_sErrMsg)
                {
                    std::string sErrMsg(ftag);
                    sErrMsg += "start error!";
                    (*out_sErrMsg) = std::move(sErrMsg);
                }

                return -1;
            }
        }

#ifdef _MSC_VER
        Sleep(ThreadPool_Conf::g_dwWaitMS_AfterStartThread);
#else
        usleep(ThreadPool_Conf::g_dwWaitMS_AfterStartThread * 1000L);
#endif

        return 0;
    }

    /*
    关闭线程池

    Return :
    0 -- 成功
    -1 -- 失败
    */
    virtual int StopPool(void)
    {
        const std::string ftag("ThreadPoolBase::StopPool() ");

        if (0 == m_hdThreads.size())
        {
            m_pool_state = ThreadPool_Conf::ThreadState::STOPPED;
            return 0;
        }
        // 先通知关闭
        typename std::vector<std::shared_ptr<T>>::iterator it = m_hdThreads.begin();
        for (it = m_hdThreads.begin(); it != m_hdThreads.end(); ++it)
        {
            (*it)->NotifyStopThread_Immediate();
        }

        std::string sMsg(ftag);
        sMsg += "Notified all thread, wait a moment";
        if (nullptr != m_pfOnEvent)
        {
            m_pfOnEvent(sMsg);
        }
        std::cout << sMsg << std::endl;

        // Sleep给线程退出时间
#ifdef _MSC_VER
        Sleep(ThreadPool_Conf::g_dwWaitMS_ThreadExit);
#else
        usleep(ThreadPool_Conf::g_dwWaitMS_ThreadExit * 1000L);
#endif

        // 确认并强行关闭
        it = m_hdThreads.begin();
        while (it != m_hdThreads.end())
        {
            (*it)->StopThread();
            ++it;
        }

        m_hdThreads.clear();

        m_pool_state = ThreadPool_Conf::ThreadState::STOPPED;

        return 0;
    }

    /*
    等待所有的任务都完成，再关闭线程池

    Return :
    0 -- 成功
    -1 -- 失败
    */
    virtual int StopPool_WaitAllFinish(void)
    {
        const std::string ftag("ThreadPoolBase::StopPool_WaitAllFinish() ");

        // 先通知关闭
        typename std::vector<std::shared_ptr<T>>::iterator it = m_hdThreads.begin();
        for (it = m_hdThreads.begin(); it != m_hdThreads.end(); ++it)
        {
            (*it)->NotifyStopThread_TillTaskFinish();
        }

        std::string sMsg(ftag);
        sMsg += "Notified all thread, wait a moment";
        if (nullptr != m_pfOnEvent)
        {
            m_pfOnEvent(sMsg);
        }
        std::cout << sMsg << std::endl;

        for (it = m_hdThreads.begin(); it != m_hdThreads.end(); ++it)
        {
            (*it)->WaitStopThread();
        }

        m_hdThreads.clear();

        m_pool_state = ThreadPool_Conf::ThreadState::STOPPED;

        return 0;
    }

private:
    // 阻止使用默认构造函数
    ThreadPoolBase(void);
};

#endif
