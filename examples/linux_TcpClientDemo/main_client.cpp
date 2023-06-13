#include <iostream>
#include <memory.h>
#include <stdlib.h>

#include "LnxTcpSample_Client_Handler.h"
#include "tcp/linux_epoll_client/EpollClient_Core.h"

#include "util/tool_string/str_to_vector.h"

#include "tcp/simple_net_pack/PacketAssembler.h"

using namespace std;

int main()
{
    cout << "Hello world!" << endl;

    string remoteIp = "127.0.0.1";
    unsigned int remotePort = 10005;

    LnxTcpSample_Client_Handler *ptrH = new LnxTcpSample_Client_Handler();
    shared_ptr<EpollClient_EventHandler> pEventHandler((EpollClient_EventHandler *)ptrH);

    EpollClient_Core esvr(pEventHandler);

    int iRes = esvr.Connect(remoteIp, remotePort);
    if (0 != iRes)
    {
        cout << "client Connect failed ip=" << remoteIp << " port=" << remotePort << endl;
        return -1;
    }

    shared_ptr<vector<uint8_t>> pbsData = nullptr;

    while (1)
    {
        sleep(1);

        pbsData = make_shared<vector<uint8_t>>();

        iRes = PacketAssembler::LocalPackageToNetBuffer(1, "hello", nullptr, *pbsData);
        if (0 != iRes)
        {
            cout << "LocalPackageToNetBuffer failed" << endl;
            return -1;
        }

        iRes = esvr.Send(pbsData);
        if (0 == iRes || -2 == iRes)
        {
            continue;
        }
        else if (-1 == iRes)
        {
            sleep(10);

            cout << "trying to reconnect to ip=" << remoteIp << " port=" << remotePort << endl;
            // reconnect
            iRes = esvr.Connect(remoteIp, remotePort);
            if (0 != iRes)
            {
                cout << "client Connect failed ip=" << remoteIp << " port=" << remotePort << endl;
            }
        }
    }

    esvr.Disconnect();

    return 0;
}
