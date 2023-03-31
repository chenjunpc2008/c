#ifndef __CONNECTION_INFORMATION_H__
#define __CONNECTION_INFORMATION_H__

#include <string>
#include <stdint.h>

#include "tool_string/sof_string.h"

/*
Connection Information.
*/
struct ConnectionInformation
{
	//
	// Members
	//
	std::string strRemoteIpAddress;
	std::string strRemoteHostName;
	uint16_t remotePortNumber;

	//
	// Functions
	//
	ConnectionInformation(void)
		: remotePortNumber(0)
	{

	}

	ConnectionInformation(const ConnectionInformation& src)
	{
		strRemoteIpAddress = src.strRemoteIpAddress;
		strRemoteHostName = src.strRemoteHostName;
		remotePortNumber = src.remotePortNumber;
	}

	ConnectionInformation& operator =(const ConnectionInformation& src)
	{
		strRemoteIpAddress = src.strRemoteIpAddress;
		strRemoteHostName = src.strRemoteHostName;
		remotePortNumber = src.remotePortNumber;

		return *this;
	}

	std::string ToString()
	{
		std::string sItoa;
		std::string sOut("ip=");
		sOut += strRemoteIpAddress;
		sOut += " port=";
		sOut += sof_string::itostr(remotePortNumber, sItoa);
		return sOut;
	}
};

#endif