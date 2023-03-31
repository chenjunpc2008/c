#ifndef __WINSOCK_UTIL_H__
#define __WINSOCK_UTIL_H__

#ifdef _MSC_VER

#else
#error "this file supports windows only!!!"
#endif

#include <string>
#include <memory>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <stdio.h>

#include "WinIOCPDefine.h"
#include "IocpDataBase.h"
#include "IocpData_Server.h"
#include "tcp/socket_conn_manage_base/ConnectionInformation.h"

/*
Windows socket utility.
*/
namespace WinSockUtil
{
    int InitializeWinsock(uint64_t &out_iErr);

    int CreateOverlappedSocket(SOCKET &out_socket, uint64_t &out_iErr);

    int CreateListenSocket(const std::string &in_ip, const u_short in_port,
                           std::shared_ptr<IocpData_Server> &out_pIocpdata, uint64_t &out_iErr);

    LPFN_ACCEPTEX LoadAcceptEx(SOCKET &s, uint64_t &out_iErr);

    //****************************************************************************
    //! @details
    //! This method extracts the Connection Information object in real time.
    //! Therefore, the accept socket must be valid.
    //!
    //! @param[in] socket
    //! The socket to extract connection information from
    //!
    //! @return ndc::tcpip::ConnectionInformation
    //! Returns the Connection Information based on the accept socket
    //!
    //! @remark
    //! If the returned Connection Information object holds no information,
    //! it implies that the function has failed.
    //!
    //****************************************************************************
    struct ConnectionInformation GetConnectionInformation(SOCKET socket);

    void PostAccept(std::shared_ptr<IocpData_Server> &iocpData, uint64_t &out_iErr);

    int PostRecv(IocpContext &iocpContext);

    int PostSend(IocpContext &iocpContext, );

    int PostDisconnect(std::shared_ptr<IocpDataBase> &iocpData, Connection &c);

    /*
        取本地以太网IP地址
    */
    int GetLocalEthernetIp(std::string &out_strIp);
};

#endif