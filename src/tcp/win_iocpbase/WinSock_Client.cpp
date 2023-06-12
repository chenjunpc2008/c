#include "WinSock_Client.h"

#include "tcp/socket_conn_manage_base/ConnectionInformation.h"

#include <IPHlpApi.h>

#pragma comment(lib, "Iphlpapi.lib")

using namespace std;

namespace WinSock_Client
{
    int CreateClientSocket(const string &in_ip, const u_short in_port,
                           SOCKET &out_clientSocket, uint64_t &out_iErr)
    {
        static const string ftag("CreateClientSocket() ");

        string strTran;
        string sDebug;

        // sockaddr_in service;

        string strIp = in_ip;
        u_short port = in_port;

        //----------------------------------------
        // Create socket
        SOCKET hClientSocket = INVALID_SOCKET;
        int iResult = WinSockUtil::CreateOverlappedSocket(hClientSocket);
        if (0 != iResult)
        {
            return -1;
        }

        //----------------------------------------
        // connect to server.
        sockaddr_in serAddr;
        serAddr.sin_family = AF_INET;
        serAddr.sin_port = htons(port);
        serAddr.sin_addr.S_un.S_addr = inet_addr(strIp.c_str());
        iResult = connect(hClientSocket, (sockaddr *)&serAddr, sizeof(serAddr));
        if (SOCKET_ERROR == iResult)
        {
            // 连接失败
            out_iErr = WSAGetLastError();

            closesocket(hClientSocket);
            hClientSocket = INVALID_SOCKET;
            WSACleanup();
            return -1;
        }

        out_clientSocket = hClientSocket;

        {
            sDebug = "connect success on address: ";
            sDebug += strIp;
            sDebug += ":";
            sDebug += sof_string::itostr(port, strTran);

            cout << ftag << sDebug << endl;
        }

        return 0;
    }

};
