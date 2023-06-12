#include <iostream>
#include <memory.h>
#include <stdlib.h>

#include "LnxTcpSample_Svr_Handler.h"
#include "tcp/linux_epoll_server/EpollServer_Core.h"

using namespace std;

int main()
{
    cout << "Hello world!" << endl;
    LnxTcpSample_Svr_Handler *ptrH = new LnxTcpSample_Svr_Handler();
    shared_ptr<EpollServer_EventHandler> pEventHandler(
        (EpollServer_EventHandler *)ptrH);

    EpollServer_Core esvr(2, pEventHandler);
    esvr.StartServer(10005, 100);

    while (1)
    {
        sleep(1);
    }

    esvr.StopServer();

    return 0;
}
