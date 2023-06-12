#include "SocketIOCPServer.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <stdio.h>
#include <exception>

// Need to link with Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#include "win_iocpbase/WinSockUtil.h"
#include "win_iocpbase/IOCPUtil.h"

SocketIOCPServer::SocketIOCPServer(const string &strIp, const u_short usPort,
                                   std::shared_ptr<IocpEventHandler> pIocpHandler)
    : m_strIp(strIp), m_usPort(usPort),
      m_pIocpdata(std::shared_ptr<IocpData_Server>(new IocpData_Server())),
      m_pIocpHandler(pIocpHandler)
{
    if (NULL != pIocpHandler)
    {
        pIocpHandler->SetServiceOwner(this);
    }
}

SocketIOCPServer::~SocketIOCPServer(void)
{
    CleanUp();
}

int SocketIOCPServer::Init(DWORD dNumOfThreads)
{
    static const string ftag("SocketIOCPServer::Init() ");

    try
    {
        string strTran;
        string sDebug;

        //----------------------------------------
        // Declare and initialize variables

        int iResult = 0;

        if (0 == dNumOfThreads)
        {
            m_dNumOfThreads = GetNumIocpThreads();
        }
        else
        {
            m_dNumOfThreads = dNumOfThreads;
        }

        //----------------------------------------
        // Initialize winsock
        iResult = WinSockUtil::InitializeWinsock();
        if (0 != iResult)
        {
            CleanUp();
            return -1;
        }

        // create IoCompletionPort
        uint64_t iErr = 0;
        iResult = IOCPUtil::InitializeIocp(m_dNumOfThreads, m_pIocpdata->m_hCompletePort, iErr);
        if (0 != iResult)
        {
            if (nullptr != m_pIocpHandler)
            {
                string strTran;
                string sDebug = ftag;
                sDebug += "IOCPUtil::InitializeIocp failed err=";
                sDebug += sof_string::itostr(iErr, strTran);

                m_pIocpHandler->OnErrorStr(sDebug);
            }

            CleanUp();
            return -1;
        }

        // Create a listening socket
        iResult = WinSockUtil::CreateListenSocket(m_strIp, m_usPort, m_pIocpdata);
        if (0 != iResult)
        {
            CleanUp();
            return -1;
        }

        // Associate the listening socket with the completion port
        uint64_t iErr = 0;
        iResult = IOCPUtil::RegisterIocpHandle((HANDLE)m_pIocpdata->m_listenSocket,
                                               (std::shared_ptr<IocpDataBase>)m_pIocpdata, iErr);
        if (0 != iResult)
        {
            if (nullptr != m_pIocpHandler)
            {
                string strTran;
                string sDebug = ftag;
                sDebug += "IOCPUtil::RegisterIocpHandle failed err=";
                sDebug += sof_string::itostr(iErr, strTran);

                m_pIocpHandler->OnErrorStr(sDebug);
            }

            CleanUp();
            return -1;
        }

        //----------------------------------------
        // Start process threadpool
        iResult = InitializeThreadPool(m_dNumOfThreads);
        if (0 != iResult)
        {
            CleanUp();
            return -1;
        }

        iResult = CreateAcceptEvent();
        if (0 != iResult)
        {
            CleanUp();
            return -1;
        }

        m_pIocpdata->m_pIocpHandler = m_pIocpHandler;

        return 0;
    }
    catch (exception &e)
    {
        if (nullptr != m_pIocpHandler)
        {
            string strTran;
            string sDebug = ftag;
            sDebug += "exception ";
            sDebug += e.what();

            m_pIocpHandler->OnErrorStr(sDebug);
        }

        CleanUp();
        return -1;
    }
    catch (...)
    {
        if (nullptr != m_pIocpHandler)
        {
            string strTran;
            string sDebug = ftag;
            sDebug += "unkown exception with error: ";
            sDebug += sof_string::itostr((uint64_t)GetLastError(), strTran);

            m_pIocpHandler->OnErrorStr(sDebug);
        }

        CleanUp();
        return -1;
    }
}

int SocketIOCPServer::CleanUp(void)
{
    static const string ftag("SocketIOCPServer::CleanUp() ");

    try
    {
        //! @remark
        //! See Network Programming for Microsoft Windows, SE Chapter 5
        //! for the following shutdown strategy.

        // Close all socket handles to flush out all pending overlapped
        // I/O operation.
        if (nullptr != m_pIocpdata)
        {
            m_pIocpdata->m_connectionManager.CloseAllConnections();
        }

        // Give out a NULL completion status to help unblock all worker
        // threads. This is retract all I/O request made to the threads, and
        // it may not be a graceful shutdown. It is the user's job to
        // graceful shutdown all connection before shutting down the server.
        if (nullptr != m_pIocpThreadPool)
        {
            unsigned int uiThreadNum = m_pIocpThreadPool->GetCurrentThreadNum();
            for (unsigned int i = 0; i < uiThreadNum; ++i)
            {
                // Help threads get out of blocking - GetQueuedCompletionStatus()
                PostQueuedCompletionStatus(
                    m_pIocpdata->m_hCompletePort,
                    0,
                    (DWORD)
                        NULL,
                    NULL);
            }

            //! @remark
            //! Some background information from Windows via CC++ by Mr. Richter.
            //!
            //! Before Windows Vista, when a thread issued an I/O request against
            //! a device associated with a completion port, it was mandatory that
            //! the thread remain alive until the request completed; otherwise,
            //! Windows canceled any outstanding requests made by the thread. With
            //! Windows Vista, this is no longer necessary: threads can now issue
            //! requests and terminate; the request will still be processed and
            //! the result will be queued to the completion port.
            m_pIocpThreadPool->StopPool_WaitAllFinish();
        }

        if (nullptr != m_pIocpdata)
        {
            if (INVALID_SOCKET != m_pIocpdata->m_listenSocket)
            {
                closesocket(m_pIocpdata->m_listenSocket);
                m_pIocpdata->m_listenSocket = INVALID_SOCKET;
            }

            if (INVALID_HANDLE_VALUE != m_pIocpdata->m_hCompletePort)
            {
                CloseHandle(m_pIocpdata->m_hCompletePort);
                m_pIocpdata->m_hCompletePort = INVALID_HANDLE_VALUE;
            }

            if (NULL != m_pIocpdata->m_pIocpHandler)
            {
                m_pIocpdata->m_pIocpHandler->OnServerClose(0);
                m_pIocpdata->m_pIocpHandler.reset();
            }
        }

        return 0;
    }
    catch (exception &e)
    {
        if (nullptr != m_pIocpHandler)
        {
            string strTran;
            string sDebug = ftag;
            sDebug += "exception ";
            sDebug += e.what();

            m_pIocpHandler->OnErrorStr(sDebug);
        }

        return -1;
    }
    catch (...)
    {
        if (nullptr != m_pIocpHandler)
        {
            string strTran;
            string sDebug = ftag;
            sDebug += "unkown exception with error: ";
            sDebug += sof_string::itostr((uint64_t)GetLastError(), strTran);

            m_pIocpHandler->OnErrorStr(sDebug);
        }

        return -1;
    }
}

// 获取线程数
DWORD SocketIOCPServer::GetNumIocpThreads(void)
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwNumberOfProcessors * 2;
}

int SocketIOCPServer::InitializeThreadPool(DWORD numThread)
{
    static const string ftag("SocketIOCPServer::InitializeThreadPool() ");

    if (0 == numThread)
    {
        m_dNumOfThreads = GetNumIocpThreads();
    }
    else
    {
        m_dNumOfThreads = numThread;
    }

    m_pIocpThreadPool = std::shared_ptr<IOCP_ThreadPool<IocpData_Server, IOCP_Thread_Server>>(
        new IOCP_ThreadPool<IocpData_Server, IOCP_Thread_Server>(m_dNumOfThreads, m_pIocpdata));
    if (NULL == m_pIocpThreadPool)
    {
        if (nullptr != m_pIocpHandler)
        {
            string strTran;
            string sDebug = ftag;
            sDebug += "new IOCP_ThreadPool() with error: ";
            sDebug += sof_string::itostr((uint64_t)GetLastError(), strTran);

            m_pIocpHandler->OnErrorStr(sDebug);
        }

        return -1;
    }

    int iRes = m_pIocpThreadPool->InitPool();
    if (0 != iRes)
    {
        if (nullptr != m_pIocpHandler)
        {
            string strTran;
            string sDebug = ftag;
            sDebug += "InitPool() failed";

            m_pIocpHandler->OnErrorStr(sDebug);
        }

        return -1;
    }

    iRes = m_pIocpThreadPool->StartPool();
    if (0 != iRes)
    {
        if (nullptr != m_pIocpHandler)
        {
            string strTran;
            string sDebug = ftag;
            sDebug += "StartPool() failed";

            m_pIocpHandler->OnErrorStr(sDebug);
        }

        return -1;
    }

    return 0;
}

int SocketIOCPServer::CreateAcceptEvent(void)
{
    static const string ftag("SocketIOCPServer::CreateAcceptEvent() ");

    int iRes = WinSockUtil::CreateOverlappedSocket(m_pIocpdata->m_acceptContext.m_socket);
    if (0 != iRes)
    {
        if (nullptr != m_pIocpHandler)
        {
            string strTran;
            string sDebug = ftag;
            sDebug += "WinSockUtil::CreateOverlappedSocket() failed";

            m_pIocpHandler->OnErrorStr(sDebug);
        }

        return -1;
    }

    WinSockUtil::PostAccept(m_pIocpdata);

    return 0;
}

void SocketIOCPServer::Send(uint64_t cid, std::shared_ptr<std::vector<uint8_t>> &pdata)
{
    static const string ftag("SocketIOCPServer::Send() ");

    std::shared_ptr<Connection> connection =
        m_pIocpdata->m_connectionManager.GetConnection(cid);

    if (NULL == connection)
    {
        if (nullptr != m_pIocpHandler)
        {
            string strTran;
            string sDebug = ftag;
            sDebug += "Connection does not exist cid=";
            sDebug += sof_string::itostr(cid, strTran);
            cout << ftag << "error " << sDebug << endl;

            m_pIocpHandler->OnErrorStr(sDebug);
        }

        return;
    }

    std::shared_ptr<IocpContext> sendContext =
        connection->CreateSendContext();

    // Take over user's data here and post it to the completion port.
    sendContext->m_data.swap(*pdata);
    sendContext->ResetWsaBuf();

    int lastError = WinSockUtil::PostSend(*sendContext);
    if (WSA_IO_PENDING != lastError)
    {
        connection->m_sendQueue.RemoveSendContext(sendContext.get());

        // Undo the swap here before throwing. This way, the user's
        // data is untouched and they may proceed to recover.
        pdata->swap(sendContext->m_data);

        if (nullptr != m_pIocpHandler)
        {
            string strTran;
            string sDebug = ftag;
            sDebug += "send to cid=";
            sDebug += sof_string::itostr(cid, strTran);
            sDebug += " failed with error: ";
            sDebug += sof_string::itostr((uint64_t)GetLastError(), strTran);

            m_pIocpHandler->OnErrorStr(sDebug);
        }
    }
}

void SocketIOCPServer::Shutdown(uint64_t cid, int how)
{
    static const string ftag("SocketIOCPServer::Shutdown() ");

    std::shared_ptr<Connection> connection =
        m_pIocpdata->m_connectionManager.GetConnection(cid);

    if (NULL == connection)
    {
        if (nullptr != m_pIocpHandler)
        {
            string strTran;
            string sDebug = ftag;
            sDebug += "Connection does not exist cid=";
            sDebug += sof_string::itostr(cid, strTran);

            m_pIocpHandler->OnErrorStr(sDebug);
        }

        return;
    }

    ::shutdown(connection->m_socket, how);
}

void SocketIOCPServer::Disconnect(uint64_t cid)
{
    static const string ftag("SocketIOCPServer::Disconnect() ");

    std::shared_ptr<Connection> connection =
        m_pIocpdata->m_connectionManager.GetConnection(cid);

    if (NULL == connection)
    {
        if (nullptr != m_pIocpHandler)
        {
            string strTran;
            string sDebug = ftag;
            sDebug += "Connection does not exist cid=";
            sDebug += sof_string::itostr(cid, strTran);

            m_pIocpHandler->OnErrorStr(sDebug);
        }

        return;
    }

    Shutdown(cid, SD_BOTH);

    ::InterlockedIncrement(&connection->m_disconnectPending);

    // Disconnect context is special (hacked) because it is not
    // tied to a connection. During graceful shutdown, it is very
    // difficult to determine when exactly is a good time to
    // remove the connection. For example, a disconnect context
    // might have already been sent by the IOCP thread send handler,
    // and you wouldn't know it unless mutex are used. To keep it as
    // lock-free as possible, this disconnect context may be redundant.
    // The disconnect handler will gracefully reject the redundant
    // disconnect context.
    WinSockUtil::PostDisconnect((std::shared_ptr<IocpDataBase>)m_pIocpdata, *connection);
}