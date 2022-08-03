#include "TaskQueue_queue.h"

TaskQueue_queue::TaskQueue_queue(void)
: TaskQueueBase(0)
{
}


TaskQueue_queue::TaskQueue_queue(const uint64_t uiId)
: TaskQueueBase(uiId)
{
}

TaskQueue_queue::~TaskQueue_queue(void)
{
}

/*
将任务由队列的首端弹出

Return :
0 -- 成功
1 -- 无任务
*/
int TaskQueue_queue::PopFront(std::shared_ptr<TaskBase>& out_ptr_task)
{
	if( m_taskQueue.empty() )
	{
		return 1;
	}

	// 由队列的首端弹出
	out_ptr_task = m_taskQueue.front();

	m_taskQueue.pop();

	return 0;
}

/*
将任务压入队列的末端

Return :
0 -- 成功
-1 -- 失败
*/
int TaskQueue_queue::PushBack(std::shared_ptr<TaskBase>& in_ptr_task)
{
	// 压入队列的末端
	m_taskQueue.push(in_ptr_task);

	return 0;
}
