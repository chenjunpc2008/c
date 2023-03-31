#include "EpollServer_EventHandler.h"

#ifdef _MSC_VER
// linux only
#else

#include <iostream>

#include "tcp/socket_conn_manage_base/ConnectionInformation.h"

using namespace std;

EpollServer_EventHandler::EpollServer_EventHandler(void)
    : m_pServiceOwner(nullptr)
{
}

EpollServer_EventHandler::~EpollServer_EventHandler(void)
{
}

void EpollServer_EventHandler::OnNewConnection(uint64_t cid, struct ConnectionInformation const &ci, std::shared_ptr<Epoll_Socket_Connection> &cConn)
{
    static const string ftag("EpollServer_EventHandler::OnNewConnection() ");

    if (nullptr != m_pServiceOwner)
    {
        cout << ftag
             << "new id=" << cid << ", ip=" << ci.strRemoteIpAddress
             << ", port=" << ci.remotePortNumber << endl;
    }
}

#endif
