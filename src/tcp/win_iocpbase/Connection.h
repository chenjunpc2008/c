#ifndef __CONNETION_H__
#define __CONNETION_H__

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <stdio.h>
#include <vector>
#include <memory>
#include <mutex>

#include "IocpContext.h"
#include "SendQueue.h"

/*
Connection.
*/
class Connection
{
    //
    // Members
    //
public:
    bool HasOutstandingContext();

    SOCKET m_socket;
    uint64_t m_id;

    long m_disconnectPending;
    long m_sendClosePending;
    long m_rcvClosed;

    IocpContext m_rcvContext;

    SendQueue m_sendQueue;

    IocpContext m_disconnectContext;

    std::mutex m_connectionMutex;

    // 消息缓存
    struct simutgw::SocketUserMessage m_SocketMsg;

    //
    // Functions
    //
public:
    Connection(SOCKET socket, uint64_t cid, int rcvBufferSize);
    virtual ~Connection(void);

    std::shared_ptr<IocpContext> CreateSendContext();

    bool CloseRcvContext(void);
};

#endif