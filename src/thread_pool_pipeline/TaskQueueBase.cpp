#include "TaskQueueBase.h"

#include <limits>
#include <iostream>

#include "util/tool_string/sof_string.h"

using namespace std;

const uint64_t TaskQueueBase::MAX_QUEUE_SIZE = (uint64_t)(std::numeric_limits<uint64_t>::max)() - 10;

TaskQueueBase::TaskQueueBase(const uint64_t uiId)
    : m_uiTaskQueId(uiId), m_uiTaskNums(0)
{
}

TaskQueueBase::~TaskQueueBase(void)
{
}

/*
插入任务

Param :
uint64_t& out_CurrentTaskNum : 当前此线程已有任务总数

Return :
0 -- 插入成功
2 -- 数量已满，不允许插入
-1 -- 插入失败
*/
int TaskQueueBase::AddTask(std::shared_ptr<TaskBase> &in_ptrTask, uint64_t &out_CurrentTaskNum)
{
    static const string ftag("TaskQueueBase::AddTask()");

    std::lock_guard<std::mutex> lock(m_mutexlock);

    // 检查当前任务数是否已超标
    if (MAX_QUEUE_SIZE <= m_uiTaskNums)
    {
        // 有相同的任务，但是数量已满，不允许插入
        // 避免数量太大，越界

        out_CurrentTaskNum = m_uiTaskNums;
        return 2;
    }

    int iRes = PushBack(in_ptrTask);
    if (0 != iRes)
    {
        // 插入错误
        return -1;
    }

    ++m_uiTaskNums;

    out_CurrentTaskNum = m_uiTaskNums;

    return 0;
}

/*
获取任务

Return :
0 -- 获取成功
1 -- 无任务
-1 -- 失败
*/
int TaskQueueBase::GetTask(std::shared_ptr<TaskBase> &out_ptrTask)
{
    static const string ftag("TaskQueueBase::GetTask()");

    std::lock_guard<std::mutex> lock(m_mutexlock);

    int iRes = PopFront(out_ptrTask);
    if (0 != iRes)
    {
        return 1;
    }

    if (NULL == out_ptrTask)
    {
        return -1;
    }

    --m_uiTaskNums;
    return 0;
}

/*
获取当前任务情况

Return :
0 -- 成功
-1 -- 失败
*/
int TaskQueueBase::GetTaskAssignInfo(string &out_strTaskInfo)
{
    static const string ftag("TaskQueueBase::GetTaskAssignInfo()");

    string strTran;

    out_strTaskInfo = "Queue[";
    out_strTaskInfo += sof_string::itostr(m_uiTaskQueId, strTran);
    out_strTaskInfo += "] size[";
    out_strTaskInfo += sof_string::itostr(QueueSize(), strTran);
    out_strTaskInfo += "]";

    return 0;
}