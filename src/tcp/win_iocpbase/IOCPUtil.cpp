#include "IOCPUtil.h"

#include "WinSockUtil.h"

using namespace std;

namespace IOCPUtil
{
    int InitializeIocp(const DWORD dwNumConcurrentThreads, HANDLE &out_hCompPort, uint64_t &out_iErr)
    {
        static const string ftag("InitializeIocp() ");

        // see https://msdn.microsoft.com/en-us/library/aa363862(VS.85).aspx
        // https://msdn.microsoft.com/en-us/library/ms737524(v=vs.85).aspx
        // Create a handle for the completion port
        HANDLE hCompPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, dwNumConcurrentThreads);
        if (NULL == hCompPort)
        {
            out_iErr = (uint64_t)GetLastError();

            return -1;
        }

        out_hCompPort = hCompPort;

        return 0;
    }

    // Associate the socket with the completion port
    int RegisterIocpHandle(HANDLE h, std::shared_ptr<IocpDataBase> &pIocpdata, uint64_t &out_iErr)
    {
        static const string ftag("RegisterIocpHandle() ");

        // see
        // https://msdn.microsoft.com/en-us/library/aa364986(v=vs.85).aspx
        HANDLE hCompPort = CreateIoCompletionPort(h, pIocpdata->m_hCompletePort, (ULONG_PTR)&pIocpdata, 0);
        if (NULL == hCompPort)
        {
            out_iErr = (uint64_t)GetLastError();

            return -1;
        }

        return 0;
    }

};