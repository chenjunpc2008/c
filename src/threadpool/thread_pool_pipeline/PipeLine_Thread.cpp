#include "PipeLine_Thread.h"

#ifdef _MSC_VER

#else
#include <sys/time.h>
#endif

PipeLine_Thread::PipeLine_Thread(void)
    : ThreadBase()
{
}

PipeLine_Thread::~PipeLine_Thread(void)
{
}

/*
插入任务

Return :
0 -- 插入成功
-1 -- 插入失败
*/
int PipeLine_Thread::AddTask(std::shared_ptr<TaskBase> &ptr_task)
{
    uint64_t uiCurrentTaskNum;
    int iRes = m_prtQueTask.AddTask(ptr_task, uiCurrentTaskNum);

    if (2 >= uiCurrentTaskNum)
    {
        // 如果当前的任务数太少，有可能是线程还在等待Event
        SetThreadEvent();
    }

    return iRes;
}

/*
获取任务

Return :
0 -- 插入成功
-1 -- 插入失败
*/
int PipeLine_Thread::GetTask(std::shared_ptr<TaskBase> &ptr_task)
{
    int iRes = m_prtQueTask.GetTask(ptr_task);

    return iRes;
}

/*
获取当前任务情况

Return :
0 -- 成功
-1 -- 失败
*/
int PipeLine_Thread::GetTaskAssignInfo(std::string &out_strTaskInfo)
{
    int iRes = m_prtQueTask.GetTaskAssignInfo(out_strTaskInfo);

    return iRes;
}

/*
启动线程

Return :
0 -- 启动成功
-1 -- 启动失败
*/
int PipeLine_Thread::StartThread(void)
{
    // static const std::string ftag("PipeLine_Thread::StartThread() ");

    int iRes = ThreadBase::StartThread(&PipeLine_Thread::Run);

    return iRes;
}

#ifdef _MSC_VER
DWORD WINAPI PipeLine_Thread::Run(LPVOID pParam)
{
    static const std::string ftag("PipeLine_Thread::Run() ");

    PipeLine_Thread *pThread = static_cast<PipeLine_Thread *>(pParam);

    pThread->m_prtQueTask.SetQueueId(pThread->m_dThreadId);

    while ((ThreadPool_Conf::Running == pThread->m_emThreadRunOp) ||
           (ThreadPool_Conf::TillTaskFinish == pThread->m_emThreadRunOp))
    {
        //等待线程信号，或超时
        if (INVALID_HANDLE_VALUE != pThread->m_hEvent)
        {
            WaitForSingleObject(pThread->m_hEvent, ThreadPool_Conf::g_dwWaitMS_Event);
        }
        else
        {
            if (nullptr != pThread->m_pfOnError)
            {
                std::string sErrMsg(ftag);
                sErrMsg += "m_hEvent INVALID_HANDLE_VALUE";
                pThread->m_pfOnError(sErrMsg);
            }
        }

        int iRes = 0;
        while ((0 == iRes) &&
               ((ThreadPool_Conf::Running == pThread->m_emThreadRunOp) ||
                (ThreadPool_Conf::TillTaskFinish == pThread->m_emThreadRunOp)))
        {
            // 取队列任务
            std::shared_ptr<TaskBase> out_ptrTask;
            iRes = pThread->m_prtQueTask.GetTask(out_ptrTask);
            if (0 == iRes)
            {
                if (NULL == out_ptrTask)
                {
                    continue;
                }

                // Run task
                out_ptrTask->TaskProc();
            }
        }

        if (ThreadPool_Conf::TillTaskFinish == pThread->m_emThreadRunOp)
        {
            // 任务处理完毕，退出
            break;
        }
    }

    /*
    std::cout << ftag << "thread finished id=" << pThread->m_dThreadId << std::endl;
    */

    pThread->m_emThreadState = ThreadPool_Conf::STOPPED;
    pThread->m_dThreadId = 0;

    return 0;
}
#else
void *PipeLine_Thread::Run(void *pParam)
{
    static const std::string ftag("PipeLine_Thread::Run() ");

    PipeLine_Thread *pThread = static_cast<PipeLine_Thread *>(pParam);

    pThread->m_prtQueTask.SetQueueId(pThread->m_dThreadId);

    while ((ThreadPool_Conf::Running == pThread->m_emThreadRunOp) ||
           (ThreadPool_Conf::TillTaskFinish == pThread->m_emThreadRunOp))
    {
        //等待线程信号，或超时
        pthread_mutex_lock(&pThread->m_mutexCond);

        // The  pthread_cond_timedwait()   function   shall   be   equivalent   to
        // pthread_cond_wait(),  except  that an error is returned if the absolute
        // time specified by abstime  passes  (that  is,  system  time  equals  or
        // exceeds  abstime) before the condition cond is signaled or broadcasted,
        // or if the absolute time specified by abstime has already been passed at
        // the time of the call.
        struct timeval now;
        struct timespec tmRestrict;

        gettimeofday(&now, NULL);

        tmRestrict.tv_sec = now.tv_sec;
        tmRestrict.tv_nsec = now.tv_usec * 1000 + ThreadPool_Conf::g_dwWaitMS_Event * 1000L;
        pthread_cond_timedwait(&pThread->m_condEvent, &pThread->m_mutexCond, &tmRestrict);

        pthread_mutex_unlock(&pThread->m_mutexCond);

        int iRes = 0;
        while ((0 == iRes) &&
               ((ThreadPool_Conf::Running == pThread->m_emThreadRunOp) ||
                (ThreadPool_Conf::TillTaskFinish == pThread->m_emThreadRunOp)))
        {
            // 取队列任务
            std::shared_ptr<TaskBase> out_ptrTask;
            iRes = pThread->m_prtQueTask.GetTask(out_ptrTask);
            if (0 == iRes)
            {
                if (NULL == out_ptrTask)
                {
                    continue;
                }

                out_ptrTask->TaskProc();
            }
        }

        if (ThreadPool_Conf::TillTaskFinish == pThread->m_emThreadRunOp)
        {
            // 任务处理完毕，退出
            break;
        }
    }

    /*
    std::cout << ftag << "thread finished id=" << pThread->m_dThreadId << std::endl;
    */

    pThread->m_emThreadState = ThreadPool_Conf::STOPPED;
    pThread->m_dThreadId = 0;

    return 0;
}
#endif
