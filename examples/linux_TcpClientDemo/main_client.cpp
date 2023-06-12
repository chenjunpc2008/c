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

    LnxTcpSample_Client_Handler *ptrH = new LnxTcpSample_Client_Handler();
    shared_ptr<EpollClient_EventHandler> pEventHandler((EpollClient_EventHandler *)ptrH);

    EpollClient_Core esvr(pEventHandler);
    int iRes = esvr.Init();
    if (0 != iRes)
    {
        cout << "client init failed" << endl;
        return -1;
    }
    esvr.Connect("127.0.0.1", 10005);

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

        esvr.Send(pbsData);
    }

    esvr.Disconnect();

    return 0;
}
