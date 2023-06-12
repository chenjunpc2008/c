#include <iostream>
#include <string>
#include <stdint.h>
#include <string.h>

#include "threadpool/thread_pool_base/ThreadPoolBase.h"
#include "threadpool/thread_pool_pipeline/PipeLine_ThreadPool.h"

using namespace std;

class TestTask : public TaskBase
{
public:
    TestTask(uint64_t uiId) : TaskBase(uiId)
    {
    }

    virtual ~TestTask(void)
    {
    }

    virtual int TaskProc(void)
    {
        static const std::string ftag("TestTask::TaskProc() ");

        {
            string strTran;
            string sDebug("task id=");
            sDebug += sof_string::itostr(m_uiTaskId, strTran);
            std::cout << ftag << sDebug << std::endl;
        }

        // Sleep(2000);

        return 0;
    }

private:
    TestTask(void);
};

int main(int argc, char *argv[])
{
    cout << sizeof(int *) * 8 << "bit program" << endl;

    PipeLine_ThreadPool pool(20);
    int iRes = pool.InitPool();

    for (uint64_t i = 0; i < 10000; ++i)
    {
        for (uint64_t j = 0; j < 80; ++j)
        {
            TestTask *t = new TestTask(i * 100 + j);

            std::shared_ptr<TaskBase> ptr(dynamic_cast<TaskBase *>(t));

            iRes = pool.AssignTask(ptr);
        }
    }

    iRes = pool.StartPool();

    // Sleep(10000);

    iRes = pool.StopPool_WaitAllFinish();

    return iRes;
}
