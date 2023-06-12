#ifndef __EPOLL_SERVER_CORE_H__
#define __EPOLL_SERVER_CORE_H__

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
#include "tcp/socket_conn_manage_base/EventBaseHandler.h"

#include "EpollServer_Client_ThreadPool.h"
#include "tcp/socket_conn_manage_base/Epoll_ConnectionManager.h"

/*
TCP Server 用来 listen accept以及管理用户连接的线程
*/
class EpollServer_Core : public ThreadBase
{
    //
    // Members
    //
  public:
    // client connection manager
    Epoll_ConnectionManager m_connectionManager;

  protected:
    int m_listenfd;

    std::shared_ptr<EpollServer_EventHandler> m_pEventHandler;

    EpollServer_Client_ThreadPool m_clientProcs;

    // listen socket port
    unsigned int m_uiListenPort;
    // listen back log
    int m_iListenBacklog;

    // std::mutex m_cidMutex;
    // uint64_t m_currentId;

    //
    // Functions
    //
  public:
    EpollServer_Core(const int ciClientThreadNums,
                     std::shared_ptr<EpollServer_EventHandler> pEventHandler);
    virtual ~EpollServer_Core();

    int StartServer(const unsigned int in_port, const int in_backlog);

    void StopServer(void);

    void Send(uint64_t cid, std::shared_ptr<std::vector<uint8_t>> &pdata);

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
    void Shutdown(uint64_t cid, int how);

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
    void Disconnect(uint64_t cid);

  protected:
    /*
    启动线程

    Return :
    0 -- 启动成功
    -1 -- 启动失败
    */
    virtual int StartThread(void);

    static void *Run(void *pParam);

  private:
    // 禁用默认构造函数
    EpollServer_Core();
};

#endif
#endif // __EPOLL_SERVER_LISTEN_THREAD_H__
