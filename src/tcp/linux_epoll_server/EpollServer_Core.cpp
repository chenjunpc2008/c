#include "EpollServer_Core.h"

#ifdef _MSC_VER
// linux only
#else

#include "tcp/socket_conn_manage_base/Epoll_Socket_Connection.h"

#include "tcp/linux_socket_base/LinuxNetUtil.h"

using namespace std;

EpollServer_Core::EpollServer_Core(const int ciClientThreadNums,
                                   std::shared_ptr<EpollServer_EventHandler> pEventHandler)
    : m_listenfd(-1), m_pEventHandler(pEventHandler),
      m_clientProcs(ciClientThreadNums, pEventHandler), m_uiListenPort(0), m_iListenBacklog(10)
{
    // ctor
    if (NULL != pEventHandler)
    {
        pEventHandler->SetServiceOwner(this);
    }
}

EpollServer_Core::~EpollServer_Core()
{
    // dtor
}

int EpollServer_Core::StartServer(const unsigned int in_port, const int in_backlog)
{
    static const std::string ftag("EpollServer_Core::StartServer() ");

    if (nullptr == m_pEventHandler)
    {
        cout << ftag << "nullptr event handler" << endl;
        return -1;
    }

    m_uiListenPort = in_port;

    m_iListenBacklog = in_backlog;

    int iRes = StartThread();
    return iRes;
}

/*
关闭服务器

Return :
0 -- 成功
-1 -- 失败
*/
void EpollServer_Core::StopServer(void)
{
    const string ftag("EpollServer_Core::StopServer() ");

    // 先通知关闭
    NotifyStopThread_Immediate();

    {
        string sDebug("Notified thread, wait a moment");
        cout << ftag << sDebug << endl;
    }

    // Sleep给线程退出时间
    usleep(ThreadPool_Conf::g_dwWaitMS_ThreadExit * 1000L);

    // 确认并强行关闭
    StopThread();

    m_clientProcs.StopPool();
}

/*
启动线程

Return :
0 -- 启动成功
-1 -- 启动失败
*/
int EpollServer_Core::StartThread(void)
{
    static const std::string ftag("EpollServer_Core::StartThread() ");

    if (0 < m_listenfd)
    {
        return 0;
    }

    string sDebug;

    int iRes = m_clientProcs.InitPool();
    if (0 != iRes)
    {
        sDebug = ftag;
        sDebug += "InitPool failed";
        m_pEventHandler->OnErrorStr(sDebug);

        return -1;
    }

    iRes = m_clientProcs.StartPool();
    if (0 != iRes)
    {
        sDebug = ftag;
        sDebug += "StartPool failed";
        m_pEventHandler->OnErrorStr(sDebug);

        return -1;
    }

    int iErr = 0;
    m_listenfd = LinuxNetUtil::CreateListenSocket(m_uiListenPort, m_iListenBacklog, iErr);
    if (0 > m_listenfd)
    {
        sDebug = ftag;
        sDebug += "socket server create error";
        m_pEventHandler->OnErrorCode(sDebug, iErr);

        return -1;
    }

    {
        string sItoa;
        sDebug = "listen socket in port=";
        sDebug += sof_string::itostr(m_uiListenPort, sItoa);
        sDebug += " success";
        m_pEventHandler->OnEvent(sDebug);
    }

    iRes = ThreadBase::StartThread(&EpollServer_Core::Run);
    if (0 != iRes)
    {
        sDebug = ftag;
        sDebug += "StartThread failed";
        m_pEventHandler->OnErrorStr(sDebug);

        return -1;
    }

    return iRes;
}

void *EpollServer_Core::Run(void *pParam)
{
    static const std::string ftag("EpollServer_Core::Run() ");

    EpollServer_Core *pThread = static_cast<EpollServer_Core *>(pParam);

    int iRes = 0;
    int iErr = 0;
    string sDebug;
    string strTran;

    // 事件数组
    struct epoll_event eventList[4];

    // epoll 初始化
    int epollfd = epoll_create(2);
    if (-1 == epollfd)
    {
        iErr = errno;

        sDebug = ftag;
        sDebug += "epoll_create error";
        pThread->m_pEventHandler->OnErrorCode(sDebug, iErr);

        return 0;
    }

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET | EPOLLERR | EPOLLRDHUP;
    event.data.fd = pThread->m_listenfd;

    // add Event
    iRes = epoll_ctl(epollfd, EPOLL_CTL_ADD, pThread->m_listenfd, &event);
    if (0 > iRes)
    {
        iErr = errno;

        sDebug = ftag;
        sDebug += "epoll add fail fd=";
        sDebug += sof_string::itostr(pThread->m_listenfd, strTran);

        pThread->m_pEventHandler->OnErrorCode(sDebug, iErr);

        return 0;
    }

    // epoll
    while ((ThreadPool_Conf::Running == pThread->m_emThreadRunOp) ||
           (ThreadPool_Conf::TillTaskFinish == pThread->m_emThreadRunOp))
    {
        // epoll_wait
        int epwRet = epoll_wait(epollfd, eventList, 4, ThreadPool_Conf::g_dwWaitMS_Event);

        if (0 > epwRet)
        {
            iErr = errno;
            if (EINTR == iErr)
            {
                continue;
            }

            sDebug = ftag;
            sDebug += "epoll_wait error";
            pThread->m_pEventHandler->OnErrorCode(sDebug, iErr);

            break;
        }
        else if (0 == epwRet)
        {
            continue;
        }

        // 严重错误
        bool bFatalError = false;
        // 活动的事件
        int i = 0;
        for (i = 0; i < epwRet; ++i)
        {
            // 监测到一个SOCKET用户连接到了绑定的SOCKET端口，建立新的连接
            if (eventList[i].data.fd == pThread->m_listenfd)
            {
                if ((EPOLLERR & eventList[i].events) || (EPOLLHUP & eventList[i].events))
                {
                    // 发生错误
                    bFatalError = true;
                    continue;
                }

                // accept client socket
                int iClientFd = -1;
                string strClientAddr;
                uint16_t uiClientPort = 0;
                iRes = LinuxNetUtil::AcceptConn(pThread->m_listenfd, iClientFd, strClientAddr,
                                                uiClientPort, iErr);
                if (0 == iRes)
                {
                    // add to thread pool to process
                    int iClient_Epollfd = -1;
                    iRes = pThread->m_clientProcs.AddNewClient(iClientFd, strClientAddr,
                                                               uiClientPort, iClient_Epollfd);
                    if (0 == iRes)
                    {
                        // add connection to manager
                        struct ConnectionInformation cinfo;
                        cinfo.strRemoteIpAddress = strClientAddr;
                        cinfo.remotePortNumber = uiClientPort;

                        shared_ptr<Epoll_Socket_Connection> c(new Epoll_Socket_Connection(
                            iClientFd, iClientFd, cinfo, iClient_Epollfd));

                        pThread->m_connectionManager.AddConnection(c);

                        pThread->m_pEventHandler->OnNewConnection(c->m_id, c->m_cinfo, c);
                    }
                    else
                    {
                        {
                            sDebug = ftag;
                            sDebug += "ip=";
                            sDebug += strClientAddr;
                            sDebug += " port=";
                            sDebug += sof_string::itostr(uiClientPort, strTran);
                            sDebug += " fd=";
                            sDebug += sof_string::itostr(iClientFd, strTran);
                            sDebug += " add to client thread pool failed, closing";

                            pThread->m_pEventHandler->OnErrorCode(sDebug, iErr);
                        }

                        close(iClientFd);
                    }
                }
                else
                {
                    // fd already closed by lower.
                    pThread->m_pEventHandler->OnErrorCode("LinuxNetUtil::AcceptConn failed", iErr);
                }
            }
            else if ((EPOLLERR & eventList[i].events) || (EPOLLHUP & eventList[i].events))
            {
                iErr = errno;
                int sfd = eventList[i].data.fd;

                close(eventList[i].data.fd);

                {
                    sDebug = ftag;
                    sDebug += "fd=";
                    sDebug += sof_string::itostr(sfd, strTran);

                    pThread->m_pEventHandler->OnErrorCode(sDebug, iErr);
                }

                continue;
            }
        }

        if (bFatalError)
        {
            // 发生错误，退出监听线程
            break;
        }
    }

    close(epollfd);

    close(pThread->m_listenfd);

    pThread->m_listenfd = -1;

    pThread->m_clientProcs.StopPool_WaitAllFinish();

    /*
    {
    string strTran;
    string sDebug("thread finished id=");
    sDebug += sof_string::itostr((uint64_t)pThread->m_dThreadId, strTran);

    cout << ftag << sDebug << endl;
    }
    */

    pThread->m_emThreadState = ThreadPool_Conf::STOPPED;
    pThread->m_dThreadId = 0;

    return 0;
}

void EpollServer_Core::Send(uint64_t cid, std::shared_ptr<std::vector<uint8_t>> &pdata)
{
    static const string ftag("EpollServer_Core::Send() ");

    if (nullptr == pdata)
    {
        string strTran;
        string sDebug = ftag;
        sDebug += "trying to send empty message id=";
        sDebug += sof_string::itostr(cid, strTran);

        m_pEventHandler->OnErrorStr(sDebug);

        return;
    }

    /*
    coud << ftag
    << "ptrVectNetData [" << ptrVectNetData << "]";

    for (int i = 0; i < ptrVectNetData->size(); ++i)
    {
    cout << ptrVectNetData->at(i) << ",";
    }

    cout<<endl;
    */

    int iErr = 0;
    int iConnId = 0;
    int iClientSockfd = 0;
    int iRes = m_connectionManager.Send(cid, pdata, iErr, iConnId, iClientSockfd);
    if (0 != iRes)
    {
        m_connectionManager.RemoveConnection(cid);

        m_pEventHandler->OnClientDisconnect(cid, iErr);
    }
}

/*
@details
Shutdown certain operation on the socket.

@param [in] cid : The id of the connection to shut down.
@param [in] how :
A flag that describes what types of operation will no longer be
allowed. Possible values for this flag are listed in the Winsock2.h
header file.

SD_RECEIVE - Shutdown receive operations.
SD_SEND - Shutdown send operations.
SD_BOTH - Shutdown both send and receive operations.

@post :
If the function completes successfully, the specified operation
on the socket will no longer be allowed.

@remark :
Shutdown should not be called on the same connection simultaneously from
different threads. Otherwise, the post condition is undefined.
*/
void EpollServer_Core::Shutdown(uint64_t cid, int how)
{
    static const string ftag("EpollServer_Core::Shutdown() ");

    std::shared_ptr<Epoll_Socket_Connection> pClientConn = m_connectionManager.GetConnection(cid);
    if (nullptr == pClientConn)
    {
        string strTran;
        string sDebug = ftag;
        sDebug += "connection manage couldn't find id=";
        sDebug += sof_string::itostr(cid, strTran);

        m_pEventHandler->OnErrorStr(sDebug);

        return;
    }

    shutdown(pClientConn->m_epollfd, SHUT_RDWR);
}

/*
@details :
Fully disconnect from a connected client. Once all outstanding sends
are completed, a corresponding OnDisconnect callback will be invoked.

@param [in] cid : The connection to disconnect from.

@post :
If this function completes successfully, the socket will no longer
be capable of sending or receiving new data (queued data prior to
disconnect will be sent).

@remark :
If the server is initiating the disconnect, it is recommended to call
Shutdown(cid, SD_BOTH) and wait for OnClientDisconnected() callback prior
to calling Disconnect(). Otherwise, it is possible OnReceiveData() callback
to be called simultaneously with OnDisconnect() callback. After all, the
client may simultaneously send data to this server during the
Disconnect() call. In such scenario, you need to provide mutexes and
additional logic to ignore the last sets of packets.

Shutdown should not be called on the same connection simultaneously from
different threads. Otherwise, the post condition is undefined.
*/
void EpollServer_Core::Disconnect(uint64_t cid)
{
    static const string ftag("EpollServer_Core::Disconnect() ");

    std::shared_ptr<Epoll_Socket_Connection> pClientConn = m_connectionManager.GetConnection(cid);
    if (nullptr == pClientConn)
    {
        string strTran;
        string sDebug = ftag;
        sDebug += "connection manage couldn't find id=";
        sDebug += sof_string::itostr(cid, strTran);

        m_pEventHandler->OnErrorStr(sDebug);

        return;
    }

    m_connectionManager.RemoveConnection(cid);

    m_pEventHandler->OnClientDisconnect(cid, 0);
}

#endif
