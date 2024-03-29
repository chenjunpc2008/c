﻿#ifndef __WINSOCK_CLIENT_H__
#define __WINSOCK_CLIENT_H__

#include <string>

#include "tcp/win_iocpbase/WinIOCPDefine.h"
#include "tcp/win_iocpbase/IocpData_Client.h"
#include "tcp/socket_conn_manage_base/ConnectionInformation.h"

#include "tcp/win_iocpbase/WinSockUtil.h"

/*
windows socket client use.
*/
namespace WinSock_Client
{
    int CreateClientSocket(const std::string &in_ip, const u_short in_port,
                           SOCKET &out_clientSocket, uint64_t &out_iErr);

};

#endif