#include "EpollClient_EventHandler.h"

#ifdef _MSC_VER
// linux only
#else

#include <iostream>

#include "tool_string/sof_string.h"

#include "socket_conn_manage_base/ConnectionInformation.h"

using namespace std;

EpollClient_EventHandler::EpollClient_EventHandler(void)
    : m_pServiceOwner(nullptr)
{
}

EpollClient_EventHandler::~EpollClient_EventHandler(void)
{
}

void EpollClient_EventHandler::OnNewConnection(uint64_t cid, struct ConnectionInformation const &ci, std::shared_ptr<Epoll_Socket_Connection> &cConn)
{
    static const string ftag("EpollClient_EventHandler::OnNewConnection() ");

    cout << ftag << "new id=" << cid << ", ip=" << ci.strRemoteIpAddress
         << ", port=" << ci.remotePortNumber << endl;
}

#endif
