#ifndef __TASK_BASE_H__
#define __TASK_BASE_H__

#include <stdint.h>

/*
任务分配的元素类
*/
class TaskBase
{
    //
    // Members
    //
protected:
    uint64_t m_uiTaskId;

    //
    // Functions
    //
public:
    TaskBase(const uint64_t uiId)
        : m_uiTaskId(uiId)
    {
    }

    virtual ~TaskBase(void)
    {
    }

    void SetTaskId(const uint64_t uiId)
    {
        m_uiTaskId = uiId;
    }

    const uint64_t GetTaskId(void) const
    {
        return m_uiTaskId;
    }

    virtual int TaskProc(void) = 0;

private:
    // 阻止使用默认构造函数
    TaskBase(void);
};

#endif