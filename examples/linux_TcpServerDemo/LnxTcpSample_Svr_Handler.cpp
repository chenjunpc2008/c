#include "LnxTcpSample_Svr_Handler.h"

#include "util/tool_log/normal_log_value.h"

#include "tcp/linux_epoll_server/EpollServer_Core.h"

#include "tcp/simple_net_pack/PacketAssembler.h"

using namespace std;

string LnxTcpSample_Svr_Handler::m_strPing("Ping");
string LnxTcpSample_Svr_Handler::m_strPong("Pong");

LnxTcpSample_Svr_Handler::LnxTcpSample_Svr_Handler(void) {}

LnxTcpSample_Svr_Handler::~LnxTcpSample_Svr_Handler(void) {}

void LnxTcpSample_Svr_Handler::OnNewConnection(uint64_t cid, struct ConnectionInformation const &ci,
                                               std::shared_ptr<Epoll_Socket_Connection> &cConn)
{
    static const string ftag("LnxTcpSample_Svr_Handler::OnNewConnection() ");

    cout << n_log::INFO << ftag << "new id=" << cid << ", ip=" << ci.strRemoteIpAddress
         << ", port=" << ci.remotePortNumber << endl;
}

void LnxTcpSample_Svr_Handler::OnReceiveData(uint64_t cid, std::vector<uint8_t> const &data)
{
    static const string ftag("LnxTcpSample_Svr_Handler::OnReceiveData() ");

    std::shared_ptr<Epoll_Socket_Connection> pClientConn =
        m_pServiceOwner->m_connectionManager.GetConnection(cid);
    if (nullptr == pClientConn)
    {
        cout << ftag << "connection manage couldn't find id=" << cid << endl;
    }

    // 已分包部分
    std::vector<std::shared_ptr<simplePack::NET_PACKAGE_LOCAL>> vecRevDatas;

    // 添加到主buffer,并再次分包
    m_handlerMsg.AppendBuffer(cid, data, vecRevDatas);

    uint64_t ui64ReportIndex = 0;
    shared_ptr<vector<uint8_t>> pbsData = nullptr;
    int iRes = 0;
    for (size_t i = 0; i < vecRevDatas.size(); ++i)
    {
        /* 先取ReportIndex */

        if (nullptr != pClientConn)
        {
            pClientConn->GetSeq(cid, ui64ReportIndex);
        }
        cout << n_log::INFO << ftag << "cid=" << cid << " rcv_data=" << vecRevDatas[i]->data
             << endl;

        pbsData = make_shared<vector<uint8_t>>();
        iRes = PacketAssembler::LocalPackageToNetBuffer(1, "hello", nullptr, *pbsData);
        if (0 != iRes)
        {
            cout << "LocalPackageToNetBuffer failed" << endl;
            return;
        }

        m_pServiceOwner->Send(cid, pbsData);
    }
}

void LnxTcpSample_Svr_Handler::OnSentData(uint64_t cid, uint64_t byteTransferred)
{
    static const string ftag("LnxTcpSample_Svr_Handler::OnSentData() ");

    cout << n_log::INFO << ftag << "id=" << cid << " byteTransferred=" << byteTransferred << endl;
}

void LnxTcpSample_Svr_Handler::OnClientDisconnect(uint64_t cid, int errorcode)
{
    static const string ftag("LnxTcpSample_Svr_Handler::OnClientDisconnect() ");

    // 从 message buffer 删除
    m_handlerMsg.RemoveId(cid);

    cout << n_log::INFO << ftag << " id=" << cid << " errorcode=" << errorcode << endl;
}

void LnxTcpSample_Svr_Handler::OnDisconnect(uint64_t cid, int errorcode)
{
    static const string ftag("LnxTcpSample_Svr_Handler::OnDisconnect() ");

    // 从 message buffer 删除
    m_handlerMsg.RemoveId(cid);

    cout << n_log::INFO << ftag << " id=" << cid << " errorcode=" << errorcode << endl;
}

/*
@details : Fully disconnect from a connected client. Once all outstanding sends
are completed, a corresponding OnDisconnect callback will be invoked.

@param[in] cid : The connection to disconnect from.
*/
void LnxTcpSample_Svr_Handler::Disconnect(uint64_t cid)
{
    static const string ftag("LnxTcpSample_Svr_Handler::Disconnect() ");

    // 从 message buffer 删除
    m_handlerMsg.RemoveId(cid);

    cout << n_log::INFO << ftag << " id=" << cid << endl;
}

void LnxTcpSample_Svr_Handler::OnServerClose(int errorCode)
{
    static const string ftag("LnxTcpSample_Svr_Handler::OnServerClose() ");

    // 从 message buffer 删除
    m_handlerMsg.RemoveAllId();
}

void LnxTcpSample_Svr_Handler::OnServerError(int errorCode)
{
    cout << n_log::ERROR << "OnServerError() "
         << " error=" << errorCode << " serror=" << strerror(errorCode) << endl;
}
