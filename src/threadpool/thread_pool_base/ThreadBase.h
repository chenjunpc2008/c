#ifndef __THREAD_BASE_H__
#define __THREAD_BASE_H__

#include "ThreadConfig.h"

#ifdef _MSC_VER

#else
#include <pthread.h>
#include <unistd.h>
#endif

/*
Thread 基础类
*/
class ThreadBase
{
    //
    // Members
    //
protected:
    //
    // 线程属性
    //

    unsigned long m_dThreadId;

#ifdef _MSC_VER
    // 已有线程的句柄
    HANDLE m_hThread;

    // 通知线程从阻塞中唤醒
    HANDLE m_hEvent;
#else
    // 已有线程的句柄
    pthread_t m_hThread;

    // 通知线程从阻塞中唤醒
    pthread_mutex_t m_mutexCond;
    pthread_cond_t m_condEvent;
#endif

    // 当前线程状态
    volatile enum ThreadPool_Conf::ThreadState m_emThreadState;

    // 当前线程运行时策略
    volatile enum ThreadPool_Conf::ThreadRunningOption m_emThreadRunOp;

    // feedback
    pf_THREAD_POOL_ONEVENT m_pfOnEvent;
    pf_THREAD_POOL_ONERROR m_pfOnError;

    //
    // Functions
    //
public:
    ThreadBase(void);
    explicit ThreadBase(const uint64_t uiId);
    virtual ~ThreadBase(void);

    void SetThreadId(const uint64_t uiId)
    {
        m_dThreadId = (unsigned long)uiId;
    }

    const uint64_t GetThreadId(void) const
    {
        return m_dThreadId;
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
    启动线程

    Return :
    0 -- 启动成功
    -1 -- 启动失败
    */
#ifdef _MSC_VER
    virtual int StartThread(LPTHREAD_START_ROUTINE lpFunc);
#else
    typedef void *(*PTRFUN)(void *);
    virtual int StartThread(PTRFUN lpFunc);
#endif
    /*
    通知关闭线程，立刻关闭

    Return :
    0 -- 成功
    -1 -- 失败
    */
    int NotifyStopThread_Immediate(void);

    /*
    通知关闭线程，任务完成后关闭

    Return :
    0 -- 成功
    -1 -- 失败
    */
    int NotifyStopThread_TillTaskFinish(void);

    /*
    关闭线程，如果有必要，则强制关闭

    Param :
    bool bForceClose :
    true -- 强制关闭
    使线程退出最好使线程的While循环break出来，不要使用TerminateThread，TerminateThread会造成很多未知因素
    false -- 不强制关闭

    Return :
    0 -- 成功
    -1 -- 失败
    */
    int StopThread(bool bForceClose = false);

    /*
    关闭线程，等待所有的任务都完成

    Return :
    0 -- 成功
    -1 -- 失败
    */
    int WaitStopThread(void);

    //有消息，通知线程处理
    bool SetThreadEvent(void);

protected:
#ifdef _MSC_VER
#else
    /*
    // pthread_cleanup_push的回调函数
    static void PthreadCleanup(void* arg)
    {
        pthread_mutex_unlock((pthread_mutex_t*)arg);
    }
    */
    /*
    pthread_cleanup_push(PthreadCleanup, &pThread->m_mutexCond);

    //等待线程信号，或超时
    pthread_mutex_lock(&pThread->m_mutexCond);

    struct timespec tmRestrict;
    tmRestrict.tv_sec = 0;
    tmRestrict.tv_nsec += ThreadPool_Conf::g_dwWaitMS_Event * 1000L;
    pthread_cond_timedwait(&pThread->m_condEvent, &pThread->m_mutexCond, &tmRestrict);

    pthread_mutex_unlock(&pThread->m_mutexCond);

    // do something
    pthread_cleanup_pop(0);

    // NOTE:
    // 必须要注意的是，如果线程处于pthread_CANCEL_ASYNCHRONOUS状态，上述代码段就有可能出错，因为CANCEL事件有可能在pthread_cleanup_push()和pthread_mutex_lock()之间发生，
    // 或者在pthread_mutex_unlock()和pthread_cleanup_pop()之间发生，从而导致清理函数unlock一个并没有加锁的mutex变量，造成错误。
    // 因此，在使用清理函数的时候，都应该暂时设置成pthread_CANCEL_DEFERRED模式。为此，POSIX的Linux实现中还提供了
    // 一对不保证可移植的pthread_cleanup_push_defer_np()/pthread_cleanup_pop_defer_np()扩展函数
    */
#endif
};

#endif
