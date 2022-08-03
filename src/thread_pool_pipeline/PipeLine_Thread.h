#ifndef __PIPELINE_THREAD_H__
#define __PIPELINE_THREAD_H__

#include "thread_pool_base/ThreadBase.h"
#include "TaskQueue_queue.h"

class PipeLine_Thread :
	public ThreadBase
{
	//
	// Members
	//
protected:
	//
	// 线程资源
	//
	TaskQueue_queue m_prtQueTask;

	//
	// Functions
	//
public:
	PipeLine_Thread(void);
	virtual ~PipeLine_Thread(void);

	/*
	启动线程

	Return :
	0 -- 启动成功
	-1 -- 启动失败
	*/
	virtual int StartThread(void);

	/*
	插入任务

	Return :
	0 -- 插入成功
	-1 -- 插入失败
	*/
	int AddTask(std::shared_ptr<TaskBase>& ptr_task);

	/*
	获取任务

	Return :
	0 -- 插入成功
	-1 -- 插入失败
	*/
	int GetTask(std::shared_ptr<TaskBase>& ptr_task);

	/*
	获取当前任务情况

	Return :
	0 -- 成功
	-1 -- 失败
	*/
	int GetTaskAssignInfo(std::string& out_strTaskInfo);

	/*
	获取队列数据大小

	Return :

	*/
	uint64_t GetQueueSize(void)
	{
		return m_prtQueTask.QueueSize();
	}

protected:	
#ifdef _MSC_VER
	static DWORD WINAPI Run(LPVOID  pParam);
#else
	static void* Run(void* pParam);
#endif
};

#endif