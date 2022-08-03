#include "LoopWorker_Thread.h"

LoopWorker_Thread::LoopWorker_Thread(void)
{
}

LoopWorker_Thread::~LoopWorker_Thread(void)
{
}

/*
插入任务

Return :
0 -- 插入成功
-1 -- 插入失败
*/
int LoopWorker_Thread::AddTask(std::shared_ptr<TaskBase> &ptr_task)
{
    m_pTask = ptr_task;

    return 0;
}

/*
启动线程

Return :
0 -- 启动成功
-1 -- 启动失败
*/
int LoopWorker_Thread::StartThread(void)
{
    // static const std::string ftag( "LoopWorker_Thread::StartThread() " );

    int iRes = ThreadBase::StartThread(&LoopWorker_Thread::Run);

    return iRes;
}

#ifdef _MSC_VER
DWORD WINAPI LoopWorker_Thread::Run(LPVOID pParam)
{
    static const std::string ftag("LoopWorker_Thread::Run() ");

    LoopWorker_Thread *pThread = static_cast<LoopWorker_Thread *>(pParam);

    while ((ThreadPool_Conf::Running == pThread->m_emThreadRunOp) ||
           (ThreadPool_Conf::TillTaskFinish == pThread->m_emThreadRunOp))
    {
        int iRes = 0;

        iRes = pThread->m_pTask->TaskProc();

        if (ThreadPool_Conf::TillTaskFinish == pThread->m_emThreadRunOp)
        {
            // 任务处理完毕，退出
            break;
        }
    }

    /*
    std::coud << ftag <<  "thread finished id="<< pThread->m_dThreadId << std::endl;
    */

    pThread->m_emThreadState = ThreadPool_Conf::STOPPED;
    pThread->m_dThreadId = 0;

    return 0;
}
#else
void *LoopWorker_Thread::Run(void *pParam)
{
    static const std::string ftag("LoopWorker_Thread::Run() ");

    LoopWorker_Thread *pThread = static_cast<LoopWorker_Thread *>(pParam);

    while ((ThreadPool_Conf::Running == pThread->m_emThreadRunOp) ||
           (ThreadPool_Conf::TillTaskFinish == pThread->m_emThreadRunOp))
    {
        pThread->m_pTask->TaskProc();

        if (ThreadPool_Conf::TillTaskFinish == pThread->m_emThreadRunOp)
        {
            // 任务处理完毕，退出
            break;
        }
    }

    /*
    std::coud << ftag <<  "thread finished id="<< pThread->m_dThreadId << std::endl;
    */

    pThread->m_emThreadState = ThreadPool_Conf::STOPPED;
    pThread->m_dThreadId = 0;

    return 0;
}
#endif
