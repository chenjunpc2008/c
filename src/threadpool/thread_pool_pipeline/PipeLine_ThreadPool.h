#ifndef __PIPELINE_THREADPOOL_H__
#define __PIPELINE_THREADPOOL_H__

#include <vector>
#include <iostream>

#include "threadpool/thread_pool_base/TaskBase.h"
#include "threadpool/thread_pool_base/ThreadPoolBase.h"
#include "PipeLine_Thread.h"
#include "util/tool_string/sof_string.h"

using namespace std;

class PipeLine_ThreadPool : public ThreadPoolBase<PipeLine_Thread>
{
protected:
public:
    PipeLine_ThreadPool(const unsigned int uiNum)
        : ThreadPoolBase(uiNum)
    {
    }

    virtual ~PipeLine_ThreadPool(void)
    {
    }

    /*
    创建线程池对象

    Return :
    0 -- 成功
    -1 -- 失败
    */
    virtual int InitPool(std::string *out_sErrMsg = nullptr)
    {
        static const string ftag("PipeLine_ThreadPool::InitPool() ");

        m_pool_state = ThreadPool_Conf::STOPPED;

        unsigned int i = 0;
        for (i = 0; i < m_uiThreadMaxNums; ++i)
        {
            std::shared_ptr<PipeLine_Thread> ptrThread = std::shared_ptr<PipeLine_Thread>(new PipeLine_Thread());
            if (NULL == ptrThread)
            {
                if (nullptr != out_sErrMsg)
                {
                    std::string sErrMsg(ftag);
                    sErrMsg += "new ThreadBase() failed, NULL";
                    (*out_sErrMsg) = std::move(sErrMsg);
                }

                return -1;
            }

            m_hdThreads.push_back(ptrThread);

            ++m_uiCurrThreadNums;
        }

        return 0;
    }

    /*
    分配任务 取最小的队列

    Return :
    0 -- 成功
    -1 -- 失败
    */
    int AssignTask(std::shared_ptr<TaskBase> &ptr_task, std::string *out_sErrMsg = nullptr)
    {
        static const string ftag("PipeLine_ThreadPool::AssignTask() ");

        if (0 == m_hdThreads.size())
        {
            if (nullptr != out_sErrMsg)
            {
                std::string sErrMsg(ftag);
                sErrMsg += "not init failed";
                (*out_sErrMsg) = std::move(sErrMsg);
            }

            return -1;
        }

        int iRes = 0;
        int i = 0;

        // 最少任务的线程句柄
        std::vector<std::shared_ptr<PipeLine_Thread>>::iterator itFewerTask = m_hdThreads.begin();
        // 最少任务的线程任务量
        uint64_t uiFewerTaskNum = 0;

        // 先查找最小的任务队列插入
        std::vector<std::shared_ptr<PipeLine_Thread>>::iterator it = m_hdThreads.begin();
        for (i = 0; it != m_hdThreads.end(); ++it, ++i)
        {
            uint64_t uiTaskNum = 0;
            uiTaskNum = (*it)->GetQueueSize();

            if (0 == uiTaskNum)
            {
                itFewerTask = it;
                break;
            }

            if (uiTaskNum < uiFewerTaskNum)
            {
                uiFewerTaskNum = uiTaskNum;
                itFewerTask = it;
            }
        }

        //空保护
        if (!(*itFewerTask))
        {
            if (nullptr != out_sErrMsg)
            {
                std::string sErrMsg(ftag);
                sErrMsg += "empty Thread handler";
                (*out_sErrMsg) = std::move(sErrMsg);
            }

            return -1;
        }

        // 插入至任务最少队列中
        iRes = (*itFewerTask)->AddTask(ptr_task);
        if (0 == iRes)
        {
            return 0;
        }
        else
        {
            if (nullptr != out_sErrMsg)
            {
                std::string sTran;
                std::string sErrMsg(ftag);
                sErrMsg += "AddTask() failed, res=";
                sErrMsg += sof_string::itostr(iRes, sTran);
                (*out_sErrMsg) = std::move(sErrMsg);
            }

            return -1;
        }
    }

    /*
    获取当前任务情况

    Return :
    0 -- 成功
    -1 -- 失败
    */
    int GetTaskAssignInfo(std::string &out_strTaskInfo)
    {
        static const std::string ftag("ThreadPool::AssignTask() ");

        int iRes = 0;

        std::vector<std::shared_ptr<PipeLine_Thread>>::const_iterator it = m_hdThreads.begin();
        for (it = m_hdThreads.begin(); it != m_hdThreads.end(); ++it)
        {
            std::string strThreadTasks;
            iRes = (*it)->GetTaskAssignInfo(strThreadTasks);
            if (0 == iRes)
            {
                //
                out_strTaskInfo += strThreadTasks;
                out_strTaskInfo += " ";
            }
            else
            {
                return -1;
            }
        }

        return 0;
    }

private:
    // 阻止使用默认构造函数
    PipeLine_ThreadPool(void);
};

#endif