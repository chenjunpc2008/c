#ifndef __LOOP_WORKER_THREADPOOL_H__
#define __LOOP_WORKER_THREADPOOL_H__

#include <vector>
#include <iostream>

#include "thread_pool_base/ThreadPoolBase.h"
#include "LoopWorker_Thread.h"
#include "thread_pool_base/TaskBase.h"

class LoopWorker_ThreadPool : public ThreadPoolBase<LoopWorker_Thread>
{
    //
    // Members
    //
protected:
    //
    // Functions
    //
public:
    LoopWorker_ThreadPool(const unsigned int uiNum)
        : ThreadPoolBase(uiNum)
    {
    }

    virtual ~LoopWorker_ThreadPool(void)
    {
    }

    /*
    分配任务

    Return :
    0 -- 成功
    -1 -- 失败
    */
    int AssignTask(std::shared_ptr<TaskBase> &ptr_task, std::string *out_sErrMsg)
    {
        static const std::string ftag("LoopWorker_ThreadPool::AssignTask() ");

        if (m_uiCurrThreadNums > m_uiThreadMaxNums)
        {
            // 已到线程数上限
            if (nullptr != out_sErrMsg)
            {
                std::string sErrMsg(ftag);
                sErrMsg += "MaxThreadNums reached!";
                (*out_sErrMsg) = std::move(sErrMsg);
            }

            return -1;
        }

        std::shared_ptr<LoopWorker_Thread> ptrThread(new LoopWorker_Thread());
        ptrThread->AddTask(ptr_task);

        m_hdThreads.push_back(ptrThread);

        ++m_uiCurrThreadNums;

        // 看当前的ThreadPool运行状态决定是否立即启动线程
        if (ThreadPool_Conf::Running == m_pool_state)
        {
            ptrThread->StartThread();
        }

        return 0;
    }

private:
    // 阻止使用默认构造函数
    LoopWorker_ThreadPool(void);
};
#endif
