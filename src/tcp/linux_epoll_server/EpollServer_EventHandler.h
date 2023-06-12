#ifndef __EPOLL_SERVER_EVENT_HANDLER_H__
#define __EPOLL_SERVER_EVENT_HANDLER_H__

#ifdef _MSC_VER
// linux only
#else

#include <stdint.h>
#include <vector>
#include <string>

#include "tcp/socket_conn_manage_base/ConnectionInformation.h"
#include "tcp/socket_conn_manage_base/EventBaseHandler.h"
#include "tcp/socket_conn_manage_base/SocketServiceBase.h"
#include "tcp/socket_conn_manage_base/Epoll_ConnectionManager.h"
#include "tcp/socket_conn_manage_base/Epoll_Socket_Connection.h"

class EpollServer_Core;

/*
消息回调句柄类.
*/
class EpollServer_EventHandler : public EventBaseHandler
{
    //
    // Members
    //
public:
protected:
    EpollServer_Core *m_pServiceOwner;

    //
    // Functions
    //
public:
    EpollServer_EventHandler(void);
    virtual ~EpollServer_EventHandler(void);

    void SetServiceOwner(EpollServer_Core *hServer)
    {
        m_pServiceOwner = hServer;
    }

    EpollServer_Core *GetServiceOwner(void)
    {
        return m_pServiceOwner;
    }

    //***************************************************************************
    // @details
    // This callback is invoked asynchronously when a new connection is accepted
    // by the tcp server.
    //
    // @param[in] cid
    // A unique Id that represents the connection.
    // This is the unique key that may be used for bookkeeping, and will remain
    // valid until OnDisconnect is called.
    //
    // @param[in] c
    // Information regarding the endpoints of the connection.
    //
    // @remark
    // This callback is invoked through the context of an tcp thread, which
    // may or may not be your main thread's context.
    //
    //***************************************************************************
    virtual void OnNewConnection(uint64_t cid, struct ConnectionInformation const &ci, std::shared_ptr<Epoll_Socket_Connection> &cConn);

protected:
private:
};

#endif
#endif
