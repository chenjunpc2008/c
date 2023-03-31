#ifndef __SOCKET_IOCP_SERVER_H__
#define __SOCKET_IOCP_SERVER_H__

/*
this IOCPServer is refer to github project:
askldjd/iocp-server
https://github.com/askldjd/iocp-server
*/

#ifdef _MSC_VER

#else
#error "this file supports windows only!!!"
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <stdio.h>

#include "socket_conn_manage_base/SocketServiceBase.h"
#include "win_iocpbase/IocpEventHandler.h"
#include "win_iocpbase/WinIOCPDefine.h"
#include "win_iocpbase/IocpData_Server.h"
#include "win_iocpbase/IOCP_Thread_Server.h"
#include "win_iocpbase/IOCP_ThreadPool.h"

class SocketIOCPServer : public SocketServiceBase
{
    //
    // Members
    //
protected:
    // 处理线程数
    DWORD m_dNumOfThreads;
    std::shared_ptr<IOCP_ThreadPool<IocpData_Server, IOCP_Thread_Server>> m_pIocpThreadPool;

    // listen ip
    std::string m_strIp;
    // listen端口
    u_short m_usPort;

    std::shared_ptr<IocpData_Server> m_pIocpdata;

    std::shared_ptr<IocpEventHandler> m_pIocpHandler;

    //
    // Functions
    //
public:
    SocketIOCPServer(const string &strIp, const u_short usPort,
                     std::shared_ptr<IocpEventHandler> pIocpHandler);
    virtual ~SocketIOCPServer(void);

    std::shared_ptr<IocpData_Server> &GetIocpData(void)
    {
        return m_pIocpdata;
    }

    int Init(DWORD dNumOfThreads);

    //!***************************************************************************
    //! @details
    //! Destructor
    //! Closes down the IO completion port, as well as ending all worker thread.
    //! All active connections will be shutdown abortively, and all outstanding
    //! sends are discarded. OnDisconnect callback will not be invoked if there
    //! are remaining connection in the IOCP server.
    //!
    //! For graceful shutdown, it is the user's responsibility to ensure that
    //! all connections are closed gracefully prior to destroying the IOCP server.
    //!
    //! OnServerClosed callback will be invoked when the server exits.
    //!
    //!***************************************************************************
    int CleanUp(void);

    // 获取线程数
    DWORD GetNumIocpThreads();

    /*
    Send data to client id by cid

    @param uint64_t cid : cid of the client
    @param const int iMsgType : message type
    @param const std::string &in_data : message

    @return
    */
    void Send(uint64_t cid, const int iMsgType, const std::string &in_data);

    //!***************************************************************************
    //! @details
    //! Send data to a connected client. IOCP server will automatically queue
    //! up packets and send them in order when ready. A corresponding
    //! OnSentData will be called when data are sent.
    //!
    //! @param[in,out] cid
    //! The connection id to send the data to.
    //!
    //! @param[in,out] data
    //! Data to send. IOCP server will use the memory as is, and will not
    //! allocate another copy.
    //!
    //! If the function operate successfully, swap() will be called on the
    //! vector, and the buffer will be emptied after this function returned.
    //!
    //! If the function fails (exception thrown), the data will be left
    //! as is.
    //!
    //! @throw
    //! CIocpException if connection no longer exists.
    //!
    //! CWin32Exception if the IOCP server failed to post this data to the
    //! IO Completion port.
    //!
    //! @remark
    //! Send should not be called on the same connection simultaneously from
    //! different threads, because it can result in an unpredictable send order.
    //! If Send() must be invoked from multiple threads , user should
    //! implement custom critical section on top of Send() for each connection.
    //!
    //!***************************************************************************
    void Send(uint64_t cid, std::shared_ptr<std::vector<uint8_t>> &pdata);

    //!***************************************************************************
    //! @details
    //! Shutdown certain operation on the socket.
    //!
    //! @param[in,out] cid
    //! The id of the connection to shut down.
    //!
    //! @param[in,out] how
    //! A flag that describes what types of operation will no longer be
    //! allowed. Possible values for this flag are listed in the Winsock2.h
    //! header file.
    //!
    //! SD_RECEIVE - Shutdown receive operations.
    //! SD_SEND - Shutdown send operations.
    //! SD_BOTH - Shutdown both send and receive operations.
    //!
    //! @throw
    //! CIocpException if connection no longer exists.
    //!
    //! CWin32Exception if the IOCP server failed to post this data to the
    //! IO Completion port.
    //!
    //! @post
    //! If the function completes successfully, the specified operation
    //! on the socket will no longer be allowed.
    //!
    //! @remark
    //! Shutdown should not be called on the same connection simultaneously from
    //! different threads. Otherwise, the post condition is undefined.
    //!
    //!***************************************************************************
    void Shutdown(uint64_t cid, int how);

    //!***************************************************************************
    //! @details
    //! Fully disconnect from a connected client. Once all outstanding sends
    //! are completed, a corresponding OnDisconnect callback will be invoked.
    //!
    //! @throw
    //! CIocpException if connection no longer exists.
    //!
    //! @param[in,out] cid
    //! The connection to disconnect from.
    //!
    //! @post
    //! If this function completes successfully, the socket will no longer
    //! be capable of sending or receiving new data (queued data prior to
    //! disconnect will be sent).
    //!
    //! @remark
    //! If the server is initiating the disconnect, it is recommended to call
    //! Shutdown(cid, SD_BOTH) and wait for OnClientDisconnected() callback prior
    //! to calling Disconnect(). Otherwise, it is possible OnReceiveData() callback
    //! to be called simultaneously with OnDisconnect() callback. After all, the
    //! client may simultaneously send data to this server during the
    //! Disconnect() call. In such scenario, you need to provide mutexes and
    //! additional logic to ignore the last sets of packets.
    //!
    //! Shutdown should not be called on the same connection simultaneously from
    //! different threads. Otherwise, the post condition is undefined.
    //!
    //!***************************************************************************
    void Disconnect(uint64_t cid);

protected:
    int InitializeThreadPool(DWORD numThread);

    int CreateAcceptEvent(void);

private:
    SocketIOCPServer(void);
};

#endif
