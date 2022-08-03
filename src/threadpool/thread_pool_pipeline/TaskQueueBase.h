#ifndef __TASK_QUEUE_BASE_H__
#define __TASK_QUEUE_BASE_H__

#include <map>
#include <stdint.h>
#include <mutex>
#include <memory>

#include "threadpool/thread_pool_base/TaskBase.h"

/*

*/
class TaskQueueBase
{
    //
    // Members
    //
public:
    // 队列允许最大总任务数量
    static const uint64_t MAX_QUEUE_SIZE;

protected:
    // id
    uint64_t m_uiTaskQueId;

    // 锁对象
    std::mutex m_mutexlock;

    // 当前队列任务总数
    uint64_t m_uiTaskNums;

    //
    // Functions
    //
public:
    TaskQueueBase(void);
    explicit TaskQueueBase(const uint64_t uiId);
    virtual ~TaskQueueBase(void);

    void SetQueueId(const uint64_t uiId)
    {
        m_uiTaskQueId = uiId;
    }

    const uint64_t GetQueueId(void) const
    {
        return m_uiTaskQueId;
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
    int AddTask(std::shared_ptr<TaskBase> &in_ptrTask, uint64_t &out_CurrentTaskNum);

    /*
    获取任务

    Return :
    0 -- 插入成功
    1 -- 无任务
    -1 -- 失败
    */
    int GetTask(std::shared_ptr<TaskBase> &out_ptrTask);

    /*
    获取当前任务情况

    Return :
    0 -- 成功
    -1 -- 失败
    */
    int GetTaskAssignInfo(std::string &out_strTaskInfo);

    /*
    获取队列数据大小

    Return :

    */
    virtual uint64_t QueueSize(void) const
    {
        return m_uiTaskNums;
    }

protected:
    /*
    将任务由队列的首端弹出

    Return :
    0 -- 成功
    -1 -- 失败
    */
    virtual int PopFront(std::shared_ptr<TaskBase> &out_ptr_task) = 0;

    /*
    将任务压入队列的末端

    Return :
    0 -- 成功
    -1 -- 失败
    */
    virtual int PushBack(std::shared_ptr<TaskBase> &ptr_task) = 0;

private:
};

#endif