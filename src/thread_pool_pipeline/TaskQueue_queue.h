#ifndef __TASK_QUEUE_QUEUE_H__
#define __TASK_QUEUE_QUEUE_H__

#include <queue>

#include "TaskQueueBase.h"

class TaskQueue_queue :
	public TaskQueueBase
{
	//
	// Members
	//
protected:
	std::queue<std::shared_ptr<TaskBase>> m_taskQueue;

public:
	TaskQueue_queue(void);
	explicit TaskQueue_queue(const uint64_t uiId);
	virtual ~TaskQueue_queue(void);

protected:
	/*
	将任务由队列的首端弹出

	Return :
	0 -- 成功
	1 -- 无任务
	*/
	virtual int PopFront(std::shared_ptr<TaskBase>& out_ptr_task);

	/*
	将任务压入队列的末端

	Return :
	0 -- 成功
	-1 -- 失败
	*/
	virtual int PushBack(std::shared_ptr<TaskBase>& in_ptr_task);

private:
	
};

#endif