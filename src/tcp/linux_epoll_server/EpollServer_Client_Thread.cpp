#include "EpollServer_Client_Thread.h"

#ifdef _MSC_VER
// linux only
#else

#include "linux_socket_base/LinuxNetUtil.h"

#include "EpollServer_Core.h"

using namespace std;

// max epoll size
const int EpollServer_Client_Thread::MAX_EPOLL_EVENTS = 500;

// max epoll client events
const int EpollServer_Client_Thread::MAX_CLIENT_EVENTS = 100;

EpollServer_Client_Thread::EpollServer_Client_Thread(const uint64_t uiId, std::shared_ptr<EpollServer_EventHandler> pEventHandler)
    : ThreadBase(uiId), m_epollfd(-1), m_aullClientNums(0), m_pEventHandler(pEventHandler)
{
}

EpollServer_Client_Thread::~EpollServer_Client_Thread()
{
    // dtor
}

/*
启动线程

Return :
0 -- 启动成功
-1 -- 启动失败
*/
int EpollServer_Client_Thread::StartThread()
{
    // static const std::string ftag("EpollServer_Client_Thread::StartThread() ");

    int iRes = ThreadBase::StartThread(&EpollServer_Client_Thread::Run);

    return iRes;
}

void *EpollServer_Client_Thread::Run(void *pParam)
{
    static const std::string ftag("EpollServer_Client_Thread::Run() ");

    EpollServer_Client_Thread *pThread = static_cast<EpollServer_Client_Thread *>(pParam);

    int iRes = 0;

    // 事件数组
    struct epoll_event eventList[MAX_CLIENT_EVENTS];

    // epoll 初始化
    int epollfd = epoll_create(MAX_EPOLL_EVENTS);
    if (-1 == epollfd)
    {
        int iErr = errno;

        if (nullptr != pThread->m_pEventHandler)
        {
            string strDebug = ftag;
            strDebug += "EPOLL_CTL_MOD error";

            pThread->m_pEventHandler->OnErrorCode(strDebug, iErr);
        }
    }

    pThread->m_epollfd = epollfd;

    // epoll
    while ((ThreadPool_Conf::Running == pThread->m_emThreadRunOp) ||
           (ThreadPool_Conf::TillTaskFinish == pThread->m_emThreadRunOp))
    {
        // epoll_wait
        int ret = epoll_wait(epollfd, eventList, EpollServer_Client_Thread::MAX_CLIENT_EVENTS, ThreadPool_Conf::g_dwWaitMS_Event);
        if (0 > ret)
        {
            int iErr = errno;
            if (EINTR == iErr)
            {
                continue;
            }

            if (nullptr != pThread->m_pEventHandler)
            {
                string strDebug = ftag;
                strDebug += "epoll_wait error";

                pThread->m_pEventHandler->OnErrorCode(strDebug, iErr);
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
            // 读写事件有可能同时触发
            if ((EPOLLIN & eventList[i].events) || (EPOLLOUT & eventList[i].events))
            {
                // 收到数据
                if (EPOLLIN & eventList[i].events)
                {
                    // cout << ftag << "EPOLLIN" << endl;
                    int sockfd = eventList[i].data.fd;
                    if (0 > sockfd)
                    {
                        continue;
                    }

                    std::vector<uint8_t> rcvData;
                    int iErr = 0;
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

                        if (nullptr != pThread->m_pEventHandler)
                        {
                            EpollServer_Core *pServerCore = pThread->m_pEventHandler->GetServiceOwner();
                            if (nullptr != pServerCore)
                            {
                                // 从 connection 删除
                                pServerCore->m_connectionManager.RemoveConnection(sockfd);
                            }

                            // Invoke the callback for the client
                            pThread->m_pEventHandler->OnClientDisconnect(sockfd, iErr);
                        }

                        // 减少client连接计数
                        pThread->m_aullClientNums -= 1;
                    }
                }

                if (EPOLLOUT & eventList[i].events)
                {
                    // cout << ftag << "EPOLLOUT" << endl;
                    //  如果有数据发送
                    int sockfd = eventList[i].data.fd;

                    if (nullptr != pThread->m_pEventHandler)
                    {
                        // Invoke the callback for the client
                        std::shared_ptr<Epoll_Socket_Connection> pClientConn = pThread->m_pEventHandler->GetServiceOwner()->m_connectionManager.GetConnection(sockfd);
                        if (nullptr == pClientConn)
                        {
                            if (nullptr != pThread->m_pEventHandler)
                            {
                                string strDebug = ftag;
                                strDebug += "connection manage couldn't find id=";
                                strDebug += sof_string::itostr(sockfd, strTran);

                                pThread->m_pEventHandler->OnErrorStr(strDebug);
                            }

                            continue;
                        }

                        uint64_t uiByteTransferred = 0;
                        int iErr = 0;
                        int iCid = 0;
                        int errorSockfd = 0;
                        int iRes = pClientConn->LoopSend(uiByteTransferred, iErr, iCid, errorSockfd);
                        if (0 > iRes)
                        {
                            EpollServer_Core *pServerCore = pThread->m_pEventHandler->GetServiceOwner();
                            if (nullptr != pServerCore)
                            {
                                // 从 connection 删除
                                pServerCore->m_connectionManager.RemoveConnection(iCid);
                            }

                            pThread->m_pEventHandler->OnDisconnect(sockfd, iErr);
                            // 减少client连接计数
                            pThread->m_aullClientNums -= 1;
                        }
                        else
                        {
                            pThread->m_pEventHandler->OnSentData(sockfd, uiByteTransferred);
                        }
                    }
                    else
                    {
                        if (nullptr != pThread->m_pEventHandler)
                        {
                            string strDebug = ftag;
                            strDebug += "no handler for in EPOLLOUT id=";
                            strDebug += sof_string::itostr(sockfd, strTran);

                            pThread->m_pEventHandler->OnErrorStr(strDebug);
                        }
                    }
                }
            }
            else
            {
                int sockfd = eventList[i].data.fd;

                int iErr = errno;
                if (eventList[i].events & EPOLLERR)
                {
                    if (nullptr != pThread->m_pEventHandler)
                    {
                        string strDebug = ftag;
                        strDebug += "event=EPOLLERR fd=";
                        strDebug += sof_string::itostr(sockfd, strTran);

                        pThread->m_pEventHandler->OnErrorCode(strDebug, iErr);
                    }
                }
                else if (eventList[i].events & EPOLLHUP)
                {
                    if (nullptr != pThread->m_pEventHandler)
                    {
                        string strDebug = ftag;
                        strDebug += "event=EPOLLHUP fd=";
                        strDebug += sof_string::itostr(sockfd, strTran);

                        pThread->m_pEventHandler->OnErrorCode(strDebug, iErr);
                    }
                }
                else
                {
                    uint32_t uiEvents = eventList[i].events;

                    if (nullptr != pThread->m_pEventHandler)
                    {
                        string strDebug = ftag;
                        strDebug += "event=";
                        strDebug += sof_string::itostr(uiEvents, strTran);
                        strDebug += " fd=";
                        strDebug += sof_string::itostr(sockfd, strTran);

                        pThread->m_pEventHandler->OnErrorCode(strDebug, iErr);
                    }
                }

                if (nullptr != pThread->m_pEventHandler)
                {
                    EpollServer_Core *pServerCore = pThread->m_pEventHandler->GetServiceOwner();
                    if (nullptr != pServerCore)
                    {
                        // 从 connection 删除
                        pServerCore->m_connectionManager.RemoveConnection(sockfd);
                    }

                    pThread->m_pEventHandler->OnServerError(iErr);
                    pThread->m_pEventHandler->OnClientDisconnect(sockfd, iErr);
                }

                // 减少client连接计数
                pThread->m_aullClientNums -= 1;

                return 0;
            }
        }
    }

    close(epollfd);
    pThread->m_epollfd = -1;

    /*
    if (!EzLog::LogLvlFilter(trace))
    {
    string strTran;
    string strDebug("thread finished id=");
    strDebug += sof_string::itostr((uint64_t)pThread->m_dThreadId, strTran);
    EzLog::Out(ftag, trace, strDebug );
    }
    */

    pThread->m_emThreadState = ThreadPool_Conf::STOPPED;
    pThread->m_dThreadId = 0;

    return 0;
}

int EpollServer_Client_Thread::AddNewClient(const int in_sockfd, int &out_epollfd)
{
    static const std::string ftag("EpollServer_Client_Thread::AddNewClient() ");

    if (0 > m_epollfd ||
        ThreadPool_Conf::ThreadRunningOption::Running != m_emThreadState)
    {
        if (nullptr != m_pEventHandler)
        {
            string strDebug = ftag;
            strDebug += "client thread is stop, not accept incoming, fd=";
            strDebug += sof_string::itostr(in_sockfd, strTran);

            m_pEventHandler->OnErrorStr(strDebug);
        }

        close(in_sockfd);
        return -1;
    }

    // add Event
    int iErr = 0;
    int iRes = LinuxNetUtil::Add_EpollWatch_NewClient(m_epollfd, in_sockfd, iErr);
    if (0 > iRes)
    {
        if (nullptr != m_pEventHandler)
        {
            string strDebug = ftag;
            strDebug += "epoll add fail fd=";
            strDebug += sof_string::itostr(in_sockfd, strTran);

            m_pEventHandler->OnErrorCode(strDebug, iErr);
        }

        close(in_sockfd);
        return -1;
    }

    out_epollfd = m_epollfd;

    m_aullClientNums += 1;

    return 0;
}

uint64_t EpollServer_Client_Thread::GetCurrentClientNums(void) const
{
    uint64_t nums = m_aullClientNums;
    return nums;
}

#endif
