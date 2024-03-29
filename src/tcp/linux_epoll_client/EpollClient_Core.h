﻿#ifndef __EPOLL_CLIENT_CORE_H__
#define __EPOLL_CLIENT_CORE_H__

#ifdef _MSC_VER
// linux only
#else

#include <errno.h>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <sys/epoll.h>
#include <unistd.h>

#include "threadpool/thread_pool_base/ThreadBase.h"

#include "tcp/linux_socket_base/LinuxNetUtil.h"

#include "tcp/socket_conn_manage_base/ConnectionInformation.h"
#include "tcp/socket_conn_manage_base/EventBaseHandler.h"

#include "EpollClient_EventHandler.h"
#include "tcp/socket_conn_manage_base/Epoll_ConnectionManager.h"

/*
TCP client 用来 和服务端通信以及管理用户连接的线程
*/
class EpollClient_Core : public ThreadBase
{
    //
    // Members
    //
  public:
    // max epoll size
    static const int MAX_EPOLL_EVENTS;

    // max epoll client events
    static const int MAX_CLIENT_EVENTS;

    enum CLIENT_STATUS
    {
        CLIENT_STATUS_CLOSED = 0,
        CLIENT_STATUS_CONNECTING,
        CLIENT_STATUS_CONNECTED
    };
    // client connection manager
    Epoll_ConnectionManager m_connectionManager;

  protected:
    std::mutex m_cliMutex;

    enum CLIENT_STATUS m_iClientStatus;

    int m_epollfd;

    int m_clientfd;

    std::shared_ptr<EpollClient_EventHandler> m_pEventHandler;

    // server socket ip
    std::string m_strServerIp;
    // server socket port
    unsigned int m_uiServerPort;

    //
    // Functions
    //
  public:
    explicit EpollClient_Core(std::shared_ptr<EpollClient_EventHandler> pEventHandler);
    virtual ~EpollClient_Core();

    int Connect(const std::string &in_serverip, const unsigned int in_port);

    void Disconnect(void);

    int GetClientfd(void) const { return m_clientfd; }

    /*
    向server发送数据

    Param :
    std::shared_ptr<std::vector<uint8_t>>& pdata : 待发送数据

    Return int : 0 -- 成功
    -1 -- client error
    -2 -- other error
    */
    int Send(std::shared_ptr<std::vector<uint8_t>> &pdata);

  protected:
    int Init(void);

    /*
    启动线程

    Return :
    0 -- 启动成功
    -1 -- 启动失败
    */
    virtual int StartThread(void);

    static void *Run(void *pParam);

    int AddNewClient(void);

    void disconnect_nolock(const int in_errNo);

  private:
    // 禁用默认构造函数
    EpollClient_Core();
};

#endif
#endif // __EPOLL_SERVER_LISTEN_THREAD_H__
