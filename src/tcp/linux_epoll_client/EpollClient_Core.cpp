﻿#include "EpollClient_Core.h"

#ifdef _MSC_VER
// linux only
#else

#include "tcp/socket_conn_manage_base/Epoll_Socket_Connection.h"

#include "tcp/linux_socket_base/LinuxNetUtil.h"

using namespace std;

// max epoll size
const int EpollClient_Core::MAX_EPOLL_EVENTS = 500;

// max epoll client events
const int EpollClient_Core::MAX_CLIENT_EVENTS = 100;

EpollClient_Core::EpollClient_Core(std::shared_ptr<EpollClient_EventHandler> pEventHandler)
    : m_iClientStatus(CLIENT_STATUS::CLIENT_STATUS_CLOSED), m_epollfd(-1), m_clientfd(-1),
      m_pEventHandler(pEventHandler), m_strServerIp(""), m_uiServerPort(0), m_bReconnect(false)
{
    // ctor
    if (NULL != pEventHandler)
    {
        pEventHandler->SetServiceOwner(this);
    }
}

EpollClient_Core::~EpollClient_Core()
{
    // dtor
}

int EpollClient_Core::Init(void)
{
    static const std::string ftag("EpollClient_Core::Init() ");

    if (nullptr == m_pEventHandler)
    {
        cout << ftag << "nullptr Event Handler" << endl;

        return -1;
    }

    int iRes = StartThread();
    if (0 != iRes)
    {
        cout << ftag << "StartThread failed" << endl;

        return -1;
    }

    return 0;
}

int EpollClient_Core::Connect(const string &in_serverip, const unsigned int in_port)
{
    static const std::string ftag("EpollClient_Core::Connect() ");

    m_strServerIp = in_serverip;
    m_uiServerPort = in_port;

    int iWaitnum = 0;

    do
    {
        if (-1 == m_epollfd)
        {
            // 线程还未启动，等待
            // Sleep给线程启动时间
            usleep(ThreadPool_Conf::g_dwWaitMS_Event * 1000L);
            ++iWaitnum;
            continue;
        }

        int iRes = AddNewClient();
        if (0 != iRes)
        {
            if (nullptr != m_pEventHandler)
            {
                m_pEventHandler->OnErrorStr("AddNewClient failed");
            }
            return -1;
        }
        else
        {
            return 0;
        }

    } while (100 >= iWaitnum);

    return -1;
}

/*
关闭线程

Return :
0 -- 成功
-1 -- 失败
*/
void EpollClient_Core::Disconnect(void)
{
    const string ftag("EpollClient_Core::Disconnect() ");

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

    shutdown(m_clientfd, SHUT_RDWR);
    close(m_clientfd);
    m_clientfd = -1;
    m_iClientStatus = CLIENT_STATUS::CLIENT_STATUS_CLOSED;
}

/*
启动线程

Return :
0 -- 启动成功
-1 -- 启动失败
*/
int EpollClient_Core::StartThread(void)
{
    static const std::string ftag("EpollClient_Core::StartThread() ");

    if (0 < m_clientfd)
    {
        return 0;
    }

    int iRes = 0;

    iRes = ThreadBase::StartThread(&EpollClient_Core::Run);
    if (0 != iRes)
    {
        if (nullptr != m_pEventHandler)
        {
            m_pEventHandler->OnErrorStr("StartThread failed");
        }

        return -1;
    }

    return iRes;
}

void *EpollClient_Core::Run(void *pParam)
{
    static const std::string ftag("EpollClient_Core::Run() ");

    EpollClient_Core *pThread = static_cast<EpollClient_Core *>(pParam);

    int sockfd = 0;
    int iRes = 0;
    int iErr = 0;
    uint64_t uiByteTransferred = 0;
    int iCid = 0;
    int errorSockfd = 0;
    // 事件数组
    struct epoll_event eventList[MAX_CLIENT_EVENTS];

    // epoll 初始化
    int epollfd = epoll_create(MAX_EPOLL_EVENTS);
    if (-1 == epollfd)
    {
        iErr = errno;

        if (nullptr != pThread->m_pEventHandler)
        {
            pThread->m_pEventHandler->OnErrorCode("EPOLL_CTL_MOD error", iErr);
        }
    }

    pThread->m_epollfd = epollfd;

    // epoll
    while ((ThreadPool_Conf::Running == pThread->m_emThreadRunOp) ||
           (ThreadPool_Conf::TillTaskFinish == pThread->m_emThreadRunOp))
    {
        // pThread->AddNewClient();

        // epoll_wait
        int ret = epoll_wait(epollfd, eventList, EpollClient_Core::MAX_CLIENT_EVENTS,
                             ThreadPool_Conf::g_dwWaitMS_Event);
        if (0 > ret)
        {
            iErr = errno;
            if (EINTR == iErr)
            {
                continue;
            }

            if (nullptr != pThread->m_pEventHandler)
            {
                pThread->m_pEventHandler->OnErrorCode("epoll_wait error", iErr);
            }

            break;
        }
        else if (0 == ret)
        {
            continue;
        }

        // 活动的事件
        int i = 0;
        for (i = 0; i < ret; ++i)
        {
            // 读写事件可能同时触发
            if ((EPOLLIN & eventList[i].events) || (EPOLLOUT & eventList[i].events))
            {
                // 收到数据
                if (EPOLLIN & eventList[i].events)
                {
                    // cout << ftag << "EPOLLIN" << endl;
                    sockfd = eventList[i].data.fd;
                    if (0 > sockfd)
                    {
                        continue;
                    }

                    std::vector<uint8_t> rcvData;
                    iErr = 0;
                    iRes = LinuxNetUtil::RecvData(sockfd, rcvData, iErr);
                    if (0 == iRes)
                    {
                        if (nullptr != pThread->m_pEventHandler)
                        {
                            // Invoke the callback for the client
                            pThread->m_pEventHandler->OnReceiveData(sockfd, rcvData);
                        }
                    }
                    else
                    {
                        // 从 connection 删除
                        pThread->m_connectionManager.RemoveConnection(sockfd);

                        if (nullptr != pThread->m_pEventHandler)
                        {
                            // Invoke the callback for the client
                            pThread->m_pEventHandler->OnClientDisconnect(sockfd, iErr);
                        }

                        // close
                        close(sockfd);

                        continue;
                    }
                }

                if (EPOLLOUT & eventList[i].events)
                {
                    // cout << ftag << "EPOLLOUT" << endl;
                    // 如果有数据发送
                    sockfd = eventList[i].data.fd;

                    // Invoke the callback for the client
                    std::shared_ptr<Epoll_Socket_Connection> pClientConn =
                        pThread->m_connectionManager.GetConnection(sockfd);
                    if (nullptr == pClientConn)
                    {
                        if (nullptr != pThread->m_pEventHandler)
                        {
                            string strTran;
                            string sDebug = "connection manage couldn't find id=";
                            sDebug += sof_string::itostr(sockfd, strTran);

                            pThread->m_pEventHandler->OnErrorStr(sDebug);
                        }

                        // wild fd, need to be close.
                        shutdown(sockfd, SHUT_RDWR);
                        close(sockfd);

                        if (nullptr != pThread->m_pEventHandler)
                        {
                            pThread->m_pEventHandler->OnDisconnect(sockfd, EPOLLERR);
                        }
                        continue;
                    }

                    uiByteTransferred = 0;
                    iErr = 0;
                    iCid = 0;
                    errorSockfd = 0;
                    iRes = pClientConn->LoopSend(uiByteTransferred, iErr, iCid, errorSockfd);
                    if (0 > iRes)
                    {
                        // 从 connection 删除
                        pThread->m_connectionManager.RemoveConnection(sockfd);

                        if (nullptr != pThread->m_pEventHandler)
                        {
                            pThread->m_pEventHandler->OnDisconnect(sockfd, iErr);
                        }
                    }
                    else
                    {
                        pThread->m_pEventHandler->OnSentData(sockfd, uiByteTransferred);
                    }
                }
            }
            else
            {
                sockfd = eventList[i].data.fd;

                iErr = errno;
                if (eventList[i].events & EPOLLERR)
                {
                    if (nullptr != pThread->m_pEventHandler)
                    {
                        string strTran;
                        string sDebug = "event=EPOLLERR fd=";
                        sDebug += sof_string::itostr(sockfd, strTran);

                        pThread->m_pEventHandler->OnErrorCode(sDebug, iErr);
                    }
                }
                else if (eventList[i].events & EPOLLHUP)
                {
                    if (nullptr != pThread->m_pEventHandler)
                    {
                        string strTran;
                        string sDebug = "event=EPOLLHUP fd=";
                        sDebug += sof_string::itostr(sockfd, strTran);

                        pThread->m_pEventHandler->OnErrorCode(sDebug, iErr);
                    }
                }
                else
                {
                    uint32_t uiEvents = eventList[i].events;

                    if (nullptr != pThread->m_pEventHandler)
                    {
                        string strTran;
                        string sDebug = "event=";
                        sDebug += sof_string::itostr(uiEvents, strTran);
                        sDebug += " fd=";
                        sDebug += sof_string::itostr(sockfd, strTran);

                        pThread->m_pEventHandler->OnErrorCode(sDebug, iErr);
                    }
                }

                // close
                shutdown(sockfd, SHUT_RDWR);
                close(sockfd);

                // 从 connection 删除
                pThread->m_connectionManager.RemoveConnection(sockfd);

                if (nullptr != pThread->m_pEventHandler)
                {
                    pThread->m_pEventHandler->OnClientDisconnect(sockfd, iErr);
                }

                return 0;
            }
        }
    }

    close(epollfd);
    pThread->m_epollfd = -1;

    /*
    string strTran;
    string sDebug("thread finished id=");
    sDebug += sof_string::itostr((uint64_t)pThread->m_dThreadId, strTran);
    cout << ftag << "trace " << sDebug <<endl;
    */

    pThread->m_emThreadState = ThreadPool_Conf::STOPPED;
    pThread->m_dThreadId = 0;

    return 0;
}

int EpollClient_Core::AddNewClient(void)
{
    static const std::string ftag("EpollClient_Core::AddNewClient() ");

    if (0 < m_clientfd)
    {
        return 0;
    }

    if (ThreadPool_Conf::Running != m_emThreadRunOp)
    {
        return 0;
    }

    if (CLIENT_STATUS::CLIENT_STATUS_CONNECTING == m_iClientStatus ||
        CLIENT_STATUS::CLIENT_STATUS_CONNECTED == m_iClientStatus)
    {
        return 0;
    }

    int iErr = 0;
    m_iClientStatus = CLIENT_STATUS::CLIENT_STATUS_CONNECTING;

    m_clientfd = LinuxNetUtil::CreateConnectSocket(m_strServerIp, m_uiServerPort, iErr);
    if (0 > m_clientfd)
    {
        if (nullptr != m_pEventHandler)
        {
            m_pEventHandler->OnErrorCode("socket client create error", iErr);
        }
        m_iClientStatus = CLIENT_STATUS::CLIENT_STATUS_CLOSED;
        return -1;
    }

    string sDebug;
    string sItoa;
    {
        sDebug = "connect socket in ip=";
        sDebug += m_strServerIp;
        sDebug += " port=";
        sDebug += sof_string::itostr(m_uiServerPort, sItoa);
        sDebug += " success";
        m_pEventHandler->OnEvent(sDebug);
    }

    if (0 > m_epollfd || ThreadPool_Conf::ThreadRunningOption::Running != m_emThreadState)
    {
        if (nullptr != m_pEventHandler)
        {
            string strTran;
            string sDebug = "client thread is stop, not accept incoming, fd=";
            sDebug += sof_string::itostr(m_clientfd, strTran);

            m_pEventHandler->OnErrorStr(sDebug);
        }

        close(m_clientfd);
        m_clientfd = -1;
        m_iClientStatus = CLIENT_STATUS::CLIENT_STATUS_CLOSED;
        return -1;
    }

    // add Event
    int iRes = LinuxNetUtil::Add_EpollWatch_NewClient(m_epollfd, m_clientfd, iErr);
    if (0 > iRes)
    {
        if (nullptr != m_pEventHandler)
        {
            string strTran;
            string sDebug = "epoll add fail fd=";
            sDebug += sof_string::itostr(m_clientfd, strTran);

            m_pEventHandler->OnErrorCode(sDebug, iErr);
        }

        close(m_clientfd);
        m_clientfd = -1;
        m_iClientStatus = CLIENT_STATUS::CLIENT_STATUS_CLOSED;
        return -1;
    }

    m_iClientStatus = CLIENT_STATUS::CLIENT_STATUS_CONNECTED;

    // add connection to manager
    struct ConnectionInformation cinfo;
    cinfo.strRemoteIpAddress = m_strServerIp;
    cinfo.remotePortNumber = m_uiServerPort;

    shared_ptr<Epoll_Socket_Connection> c(
        new Epoll_Socket_Connection(m_clientfd, m_clientfd, cinfo, m_epollfd));

    m_connectionManager.AddConnection(c);

    if (nullptr != m_pEventHandler)
    {
        m_pEventHandler->OnNewConnection(c->m_id, c->m_cinfo, c);
    }

    return 0;
}

/*
向server发送数据

Param :
std::shared_ptr<std::vector<uint8_t>>& pdata : 待发送数据

Return :
void
*/
void EpollClient_Core::Send(std::shared_ptr<std::vector<uint8_t>> &pdata)
{
    static const string ftag("EpollClient_Core::Send() ");

    int iErr = 0;

    if (0 >= m_clientfd)
    {
        if (nullptr != m_pEventHandler)
        {
            string strTran;
            string sDebug = "trying to send to illegal id=";
            sDebug += sof_string::itostr(m_clientfd, strTran);

            m_pEventHandler->OnErrorCode(sDebug, iErr);
        }

        return;
    }

    if (nullptr == pdata)
    {
        if (nullptr != m_pEventHandler)
        {
            string strTran;
            string sDebug = "trying to send empty message id=";
            sDebug += sof_string::itostr(m_clientfd, strTran);

            m_pEventHandler->OnErrorCode(sDebug, iErr);
        }

        return;
    }

    /*
    cout << ftag
    << "ptrVectNetData [" << ptrVectNetData << "]";

    for (int i = 0; i < ptrVectNetData->size(); ++i)
    {
    cout << ptrVectNetData->at(i) << ",";
    }

    cout << endl;
    */

    int iConnId = 0;
    int iClientSockfd = 0;
    int iRes = m_connectionManager.Send(m_clientfd, pdata, iErr, iConnId, iClientSockfd);
    if (0 != iRes)
    {
        m_connectionManager.RemoveConnection(m_clientfd);

        if (nullptr != m_pEventHandler)
        {
            m_pEventHandler->OnDisconnect(m_clientfd, iErr);
        }
    }
}
#endif
