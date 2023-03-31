#ifndef __EPOLL_CLIENT_EVENT_HANDLER_H__
#define __EPOLL_CLIENT_EVENT_HANDLER_H__

#ifdef _MSC_VER
// linux only
#else

#include <stdint.h>
#include <vector>
#include <string>

#include "socket_conn_manage_base/EventBaseHandler.h"
#include "socket_conn_manage_base/SocketServiceBase.h"
#include "linux_epoll_server/Epoll_Socket_Connection.h"

class EpollClient_Core;

/*
消息回调句柄类.
*/
class EpollClient_EventHandler : public EventBaseHandler
{
    //
    // Members
    //
public:
protected:
    EpollClient_Core *m_pServiceOwner;

    //
    // Functions
    //
public:
    EpollClient_EventHandler(void);
    virtual ~EpollClient_EventHandler(void);

    void SetServiceOwner(EpollClient_Core *hServer)
    {
        m_pServiceOwner = hServer;
    }

    EpollClient_Core *GetServiceOwner(void)
    {
        return m_pServiceOwner;
    }

    //!***************************************************************************
    //! @details
    //! This callback is invoked asynchronously when a new connection is accepted
    //! by the IOCP server.
    //!
    //! @param[in] cid
    //! A unique Id that represents the connection.
    //! This is the unique key that may be used for bookkeeping, and will remain
    //! valid until OnDisconnect is called.
    //!
    //! @param[in] c
    //! Information regarding the endpoints of the connection.
    //!
    //! @remark
    //! This callback is invoked through the context of an IOCP thread, which
    //! may or may not be your main thread's context.
    //!
    //!***************************************************************************
    virtual void OnNewConnection(uint64_t cid, struct ConnectionInformation const &ci, std::shared_ptr<Epoll_Socket_Connection> &cConn);

protected:
private:
};

#endif
#endif
