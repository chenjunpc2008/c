#include "ThreadBase.h"

#ifdef _MSC_VER

#else
#include <unistd.h>
#endif

#include <exception>
#include <iostream>
#include <stdexcept>

#include "util/tool_string/sof_string.h"

using namespace std;

ThreadBase::ThreadBase(void)
    : m_dThreadId(0),
#ifdef _MSC_VER
      m_hThread(INVALID_HANDLE_VALUE), m_hEvent(INVALID_HANDLE_VALUE),
#else
      m_hThread(0), m_mutexCond(PTHREAD_MUTEX_INITIALIZER), m_condEvent(PTHREAD_COND_INITIALIZER),
#endif
      m_emThreadState(ThreadPool_Conf::STOPPED), m_emThreadRunOp(ThreadPool_Conf::Running),
      m_pfOnEvent(nullptr), m_pfOnError(nullptr)
{
}

ThreadBase::ThreadBase(const uint64_t uiId)
    : m_dThreadId((unsigned long)uiId),
#ifdef _MSC_VER
      m_hThread(INVALID_HANDLE_VALUE), m_hEvent(INVALID_HANDLE_VALUE),
#else
      m_hThread(0), m_mutexCond(PTHREAD_MUTEX_INITIALIZER), m_condEvent(PTHREAD_COND_INITIALIZER),
#endif
      m_emThreadState(ThreadPool_Conf::STOPPED), m_emThreadRunOp(ThreadPool_Conf::Running),
      m_pfOnEvent(nullptr), m_pfOnError(nullptr)
{
}

ThreadBase::~ThreadBase(void)
{
#ifdef _MSC_VER
    CloseHandle(m_hEvent);
    m_hEvent = INVALID_HANDLE_VALUE;
#else
    pthread_mutex_destroy(&m_mutexCond);
    pthread_cond_destroy(&m_condEvent);
#endif
}

/*
启动线程

Return :
0 -- 启动成功
-1 -- 启动失败
*/
#ifdef _MSC_VER
int ThreadBase::StartThread(LPTHREAD_START_ROUTINE lpFunc)
{
    const string ftag("ThreadBase::StartThread() ");

    if (INVALID_HANDLE_VALUE == m_hEvent)
    {
        m_hEvent = CreateEvent(NULL, false, true, NULL);
        if (INVALID_HANDLE_VALUE == m_hEvent)
        {
            cout << ftag << "CreateEvent error" << endl;
            exception e("CreateEvent error");
            throw e;
        }
    }

    if (INVALID_HANDLE_VALUE == m_hThread)
    {
        m_emThreadState = ThreadPool_Conf::STARTED;
        m_emThreadRunOp = ThreadPool_Conf::Running;

        m_hThread = CreateThread(NULL, 0, lpFunc, this, 0, &m_dThreadId);

        if (INVALID_HANDLE_VALUE == m_hThread)
        {
            cout << ftag << "CreateThread error INVALID_HANDLE_VALUE" << endl;
            exception e("CreateThread error INVALID_HANDLE_VALUE");
            throw e;
        }

        return 0;
    }
    else
    {
        return 1;
    }

    return 1;
}
#else
int ThreadBase::StartThread(PTRFUN lpFunc)
{
    const string ftag("ThreadBase::StartThread() ");

    if (0 == m_hThread)
    {
        m_emThreadState = ThreadPool_Conf::STARTED;
        m_emThreadRunOp = ThreadPool_Conf::Running;

        int iRes = pthread_create(&m_hThread, NULL, lpFunc, this);
        if (0 != iRes)
        {
            // reset
            m_hThread = 0;
            m_emThreadState = ThreadPool_Conf::STOPPED;
            m_dThreadId = 0;

            cout << ftag << "pthread_create errorno=" << iRes << endl;
            runtime_error e("pthread_create error");
            throw e;
        }

        return 0;
    }
    else
    {
        return 1;
    }

    return 1;
}
#endif

/*
通知关闭线程，立刻关闭

Return :
0 -- 成功
-1 -- 失败
*/
int ThreadBase::NotifyStopThread_Immediate(void)
{
    m_emThreadRunOp = ThreadPool_Conf::Immediate;

    SetThreadEvent();

    return 0;
}

/*
通知关闭线程，任务完成后关闭

Return :
0 -- 成功
-1 -- 失败
*/
int ThreadBase::NotifyStopThread_TillTaskFinish(void)
{
    m_emThreadRunOp = ThreadPool_Conf::TillTaskFinish;

    SetThreadEvent();

    return 0;
}

/*
关闭线程，等待所有的任务都完成

Return :
0 -- 成功
-1 -- 失败
*/
#ifdef _MSC_VER
int ThreadBase::WaitStopThread(void)
{
    // const string ftag("ThreadBase::WaitStopThread() ");

    if (ThreadPool_Conf::STOPPED == m_emThreadState)
    {
        return 0;
    }

    if (ThreadPool_Conf::Running == m_emThreadRunOp)
    {
        // 如果退出状态未设，则先行通知
        m_emThreadRunOp = ThreadPool_Conf::TillTaskFinish;

        SetThreadEvent();
    }

    Sleep(ThreadPool_Conf::g_dwWaitMS_ThreadExit);

    if (ThreadPool_Conf::STOPPED != m_emThreadState)
    {
        // 等待线程返回
        if (INVALID_HANDLE_VALUE != m_hThread)
        {
            WaitForSingleObject(m_hThread, INFINITE);

            CloseHandle(m_hThread);
            m_hThread = INVALID_HANDLE_VALUE;
            m_emThreadState = ThreadPool_Conf::STOPPED;
            m_dThreadId = 0;
        }
    }

    return 0;
}
#else
int ThreadBase::WaitStopThread(void)
{
    const string ftag("ThreadBase::WaitStopThread() ");

    if (ThreadPool_Conf::STOPPED == m_emThreadState)
    {
        return 0;
    }

    if (ThreadPool_Conf::Running == m_emThreadRunOp)
    {
        // 如果退出状态未设，则先行通知
        m_emThreadRunOp = ThreadPool_Conf::TillTaskFinish;

        SetThreadEvent();
    }

    usleep(ThreadPool_Conf::g_dwWaitMS_ThreadExit * 1000);

    if (ThreadPool_Conf::STOPPED != m_emThreadState)
    {
        // 等待线程返回
        if (0 != m_hThread)
        {
            int iRes = pthread_join(m_hThread, NULL);
            if (0 != iRes)
            {
                string strTran;
                string sErrMsg(ftag);
                sErrMsg += "pthread_join errorno=";
                sErrMsg += sof_string::itostr(iRes, strTran);
                if (nullptr != m_pfOnError)
                {
                    m_pfOnError(sErrMsg);
                }
                cout << sErrMsg << endl;

                return -1;
            }

            m_hThread = 0;
            m_emThreadState = ThreadPool_Conf::STOPPED;
            m_dThreadId = 0;
        }
    }

    return 0;
}
#endif

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
#ifdef _MSC_VER
int ThreadBase::StopThread(bool bForceClose)
{
    const string ftag("ThreadBase::StopThread() ");

    if (ThreadPool_Conf::STOPPED == m_emThreadState)
    {
        return 0;
    }

    if (ThreadPool_Conf::Running != m_emThreadRunOp)
    {
        // 如果退出状态已设，则强行退出
        if (ThreadPool_Conf::STOPPED != m_emThreadState && bForceClose)
        {
            TerminateThread(m_hThread, 9);

            {
                string strTran;
                string sMsg(ftag);
                sMsg += "thread Terminated id=";
                sMsg += sof_string::itostr((uint64_t)m_dThreadId, strTran);
                if (nullptr != m_pfOnEvent)
                {
                    m_pfOnEvent(sMsg);
                }
                cout << sMsg << endl;
            }
        }
    }
    else
    {
        // 如果退出状态未设，则先行通知
        m_emThreadRunOp = ThreadPool_Conf::Immediate;

        SetThreadEvent();

        // 等待线程返回
        DWORD dwWait = WaitForSingleObject(m_hThread, ThreadPool_Conf::g_dwWaitMS_ThreadExit);
        if (WAIT_TIMEOUT == dwWait && bForceClose)
        {
            TerminateThread(m_hThread, 9);

            {
                string strTran;
                string sMsg(ftag);
                sMsg += "thread Terminated id=";
                sMsg += sof_string::itostr((uint64_t)m_dThreadId, strTran);
                if (nullptr != m_pfOnEvent)
                {
                    m_pfOnEvent(sMsg);
                }
                cout << sMsg << endl;
            }
        }
    }

    m_hThread = INVALID_HANDLE_VALUE;
    m_emThreadState = ThreadPool_Conf::STOPPED;
    m_dThreadId = 0;

    return 0;
}
#else
int ThreadBase::StopThread(bool bForceClose)
{
    const string ftag("ThreadBase::StopThread() ");

    if (ThreadPool_Conf::STOPPED == m_emThreadState)
    {
        TimedJoinClean(bForceClose);

        m_hThread = 0;
        m_emThreadState = ThreadPool_Conf::STOPPED;
        m_dThreadId = 0;
        return 0;
    }

    if (ThreadPool_Conf::Running != m_emThreadRunOp)
    {
        // 如果退出状态已设，则强行退出
        if (ThreadPool_Conf::STOPPED != m_emThreadState && bForceClose)
        {
            int iRes = pthread_cancel(m_hThread);
            if (0 != iRes)
            {
                string strTran;
                string sErrMsg(ftag);
                sErrMsg += "pthread_cancel in tid=";
                sErrMsg += sof_string::itostr((uint64_t)m_dThreadId, strTran);
                sErrMsg += " errno=";
                sErrMsg += sof_string::itostr((uint64_t)iRes, strTran);
                if (nullptr != m_pfOnError)
                {
                    m_pfOnError(sErrMsg);
                }
                cout << sErrMsg << endl;
            }

            JoinClean();
        }
    }
    else
    {
        // 如果退出状态未设，则先行通知
        m_emThreadRunOp = ThreadPool_Conf::Immediate;

        SetThreadEvent();

        TimedJoinClean(bForceClose);
    }

    m_hThread = 0;
    m_emThreadState = ThreadPool_Conf::STOPPED;
    m_dThreadId = 0;

    return 0;
}

// pthread join clean
void ThreadBase::JoinClean(void)
{
    const string ftag("ThreadBase::JoinClean() ");

    if (0 == m_hThread)
    {
        return;
    }

    void *result;
    /* Join with thread to see what its exit status was */
    int iRes = pthread_join(m_hThread, &result);
    if (0 != iRes)
    {
        string strTran;
        string sErrMsg(ftag);
        sErrMsg += "pthread_join in tid=";
        sErrMsg += sof_string::itostr((uint64_t)m_dThreadId, strTran);
        sErrMsg += " res=";
        sErrMsg += sof_string::itostr((uint64_t)iRes, strTran);
        if (nullptr != m_pfOnError)
        {
            m_pfOnError(sErrMsg);
        }
        cout << sErrMsg << endl;
    }

    if (PTHREAD_CANCELED == result)
    {
        string strTran;
        string sMsg(ftag);
        sMsg += "thread was canceled id=";
        sMsg += sof_string::itostr((uint64_t)m_dThreadId, strTran);
        if (nullptr != m_pfOnEvent)
        {
            m_pfOnEvent(sMsg);
        }
        cout << sMsg << endl;
    }
    else
    {
        string strTran;
        string sErrMsg(ftag);
        sErrMsg += "thread wasn't canceled (shouldn't happen!) tid=";
        sErrMsg += sof_string::itostr((uint64_t)m_dThreadId, strTran);
        sErrMsg += " result=";
        sErrMsg += sof_string::itostr((uint64_t)result, strTran);
        if (nullptr != m_pfOnError)
        {
            m_pfOnError(sErrMsg);
        }
        cout << sErrMsg << endl;
    }

    /* Free memory allocated by thread */
    free(result);
}

// pthread join clean
void ThreadBase::TimedJoinClean(bool bForceClose)
{
    const string ftag("ThreadBase::TimedJoinClean() ");

    if (0 == m_hThread)
    {
        return;
    }

    struct timespec ts;
    // 等待线程返回
    ts.tv_sec = 0;
    ts.tv_nsec = ThreadPool_Conf::g_dwWaitMS_ThreadExit * 1000L;

    int iRes = pthread_timedjoin_np(m_hThread, NULL, &ts);
    if (0 != iRes && bForceClose)
    {
        iRes = pthread_cancel(m_hThread);
        if (0 != iRes)
        {
            string strTran;
            string sErrMsg(ftag);
            sErrMsg += "pthread_cancel in tid=";
            sErrMsg += sof_string::itostr((uint64_t)m_dThreadId, strTran);
            sErrMsg += " res=";
            sErrMsg += sof_string::itostr((uint64_t)iRes, strTran);
            if (nullptr != m_pfOnError)
            {
                m_pfOnError(sErrMsg);
            }
            cout << sErrMsg << endl;
        }
    }
}

#endif

// 有消息，通知线程处理
bool ThreadBase::SetThreadEvent(void)
{
#ifdef _MSC_VER
    if (INVALID_HANDLE_VALUE != m_hEvent)
    {
        SetEvent(m_hEvent);
    }
#else
    pthread_cond_signal(&m_condEvent);
#endif

    return true;
}
