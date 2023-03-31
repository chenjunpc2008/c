#ifndef __CLIENT_SOCKET_H__
#define __CLIENT_SOCKET_H__

#include <stdint.h>

#include "win_iocpbase/WinSock_Client.h"
#include "win_iocpbase/IOCPUtil.h"
#include "win_iocpbase/IocpEventHandler.h"
#include "win_iocpbase/IocpData_Client.h"
#include "win_iocpbase/IOCP_Thread_Client.h"
#include "win_iocpbase/IOCP_ThreadPool.h"

class ClientSocket
{
    //
    // Members
    //
protected:
    std::string m_strServerIp;
    u_short m_usServer_port;

    std::shared_ptr<IocpEventHandler> m_pIocpHandler;
    std::shared_ptr<IocpData_Client> m_pIocpdata;

    DWORD m_dNumOfThreads;

    std::shared_ptr<IOCP_ThreadPool<IocpData_Client, IOCP_Thread_Client>> m_pIocpThreadPool;

    uint64_t m_ui64MyCid;

    //
    // Functions
    //
public:
    explicit ClientSocket(std::shared_ptr<IocpEventHandler> pIocpHandler);
    virtual ~ClientSocket();

    /*
    初始化环境及IOCP线程池

    Param :
    void

    Return :
    0 -- 处理成功
    -1 -- 处理失败
    */
    int Init(void);

    /*
    与server建立socket tcp连接

    Param :
    const string& in_ip : Server Ip address.
    const u_short in_port : Server Listen port.

    Return :
    0 -- 连接成功
    -1 -- 连接失败
    */
    int Connect(const string &in_ip, const u_short in_port);

    //!***************************************************************************
    //! @details
    //! Fully disconnect from a connected client. Once all outstanding sends
    //! are completed, a corresponding OnDisconnect callback will be invoked.
    //!
    //! check class SocketServiceBase::Disconnect()
    //!***************************************************************************
    void Disconnect(void);

    /*
    向server发送数据

    Param :
    std::shared_ptr<std::vector<uint8_t>>& pdata : 已打包好待发送unsigned char型数据

    Return :
    void
    */
    void Send(std::shared_ptr<std::vector<uint8_t>> &pdata);

protected:
    /*
    初始化线程池

    Param :
    DWORD numThread : 线程数量

    Return :
    0 -- 成功
    -1 -- 失败
    */
    int InitializeThreadPool(DWORD numThread);

    /*
    关闭清理资源

    Param :
    void

    Return :
    0 -- 成功
    -1 -- 失败
    */
    int CleanUp(void);

    //!***************************************************************************
    //! @details
    //! Shutdown certain operation on the socket.
    //!
    //! check class SocketServiceBase::Shutdown()
    //!***************************************************************************
    void Shutdown(uint64_t cid, int how);

    /*
    设置keepalive
    @Return
    0 -- 成功
    -1 -- 失败
    */
    int SetKeepAlive();

    /*
    与server重新建立socket tcp连接

    Return :
    0 -- 连接成功
    -1 -- 连接失败
    */
    int ReConnect();
};

#endif