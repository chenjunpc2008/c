#ifndef __SOCKET_SERVICE_BASE_H__
#define __SOCKET_SERVICE_BASE_H__

#include <stdint.h>

#ifdef _MSC_VER
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <stdio.h>
#else

#endif

/*
Base class for socket service, as father class of process callback.
*/
class SocketServiceBase
{
	//
	// Members
	//
protected:
	

	//
	// Functions
	//
public:

	//!***************************************************************************
	//! @details
	//! Shutdown certain operation on the socket.
	//!
	//! @param[in,out] cid
	//! The id of the connection to shut down.
	//!
	//! @param[in,out] how
	//! A flag that describes what types of operation will no longer be 
	//! allowed. Possible values for this flag are listed in the Winsock2.h 
	//! header file.
	//!
	//! SD_RECEIVE - Shutdown receive operations.
	//! SD_SEND - Shutdown send operations.
	//! SD_BOTH - Shutdown both send and receive operations.
	//!
	//! @throw
	//! CIocpException if connection no longer exists.
	//!
	//! CWin32Exception if the IOCP server failed to post this data to the
	//! IO Completion port.
	//!
	//! @post
	//! If the function completes successfully, the specified operation
	//! on the socket will no longer be allowed.
	//!
	//! @remark
	//! Shutdown should not be called on the same connection simultaneously from 
	//! different threads. Otherwise, the post condition is undefined.
	//!
	//!***************************************************************************
	virtual void Shutdown(uint64_t cid, int how) = 0;

	//!***************************************************************************
	//! @details
	//! Fully disconnect from a connected client. Once all outstanding sends
	//! are completed, a corresponding OnDisconnect callback will be invoked.
	//!
	//! @throw
	//! CIocpException if connection no longer exists.
	//!
	//! @param[in,out] cid
	//! The connection to disconnect from.
	//!
	//! @post
	//! If this function completes successfully, the socket will no longer
	//! be capable of sending or receiving new data (queued data prior to 
	//! disconnect will be sent).
	//!
	//! @remark
	//! If the server is initiating the disconnect, it is recommended to call 
	//! Shutdown(cid, SD_BOTH) and wait for OnClientDisconnected() callback prior 
	//! to calling Disconnect(). Otherwise, it is possible OnReceiveData() callback 
	//! to be called simultaneously with OnDisconnect() callback. After all, the 
	//! client may simultaneously send data to this server during the 
	//! Disconnect() call. In such scenario, you need to provide mutexes and
	//! additional logic to ignore the last sets of packets.
	//!
	//! Shutdown should not be called on the same connection simultaneously from 
	//! different threads. Otherwise, the post condition is undefined.
	//!
	//!***************************************************************************
	virtual void Disconnect( uint64_t cid ) = 0;

};

#endif