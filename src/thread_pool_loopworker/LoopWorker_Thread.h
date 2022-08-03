#ifndef __LOOP_WORKER_THREAD_H__
#define __LOOP_WORKER_THREAD_H__

#include <memory>

#include "thread_pool_base/ThreadBase.h"

#include "thread_pool_base/TaskBase.h"

class LoopWorker_Thread :
	public ThreadBase
{
	//
	// Members
	//
protected:
	std::shared_ptr<TaskBase> m_pTask;

	//
	// Functions
	//
public:
	LoopWorker_Thread(void);
	~LoopWorker_Thread(void);

	int AddTask(std::shared_ptr<TaskBase>& ptr_task);

	/*
	启动线程

	Return :
	0 -- 启动成功
	-1 -- 启动失败
	*/
	virtual int StartThread(void);


protected:
#ifdef _MSC_VER
	static DWORD WINAPI Run(LPVOID  pParam);
#else
	static void* Run(void* pParam);
#endif
};

#endif