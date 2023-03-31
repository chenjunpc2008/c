#ifndef __PACK_ASSEMBLER_H__
#define __PACK_ASSEMBLER_H__

#include "NetPackage.h"

#include <stdint.h>
#include <vector>
#include <time.h>
#include <memory>

/*
网络数据包组包、拆包工具
*/
class PacketAssembler
{
	//
	// Members
	//
protected:


	//
	// Functions
	//
public:
	PacketAssembler(void);
	~PacketAssembler(void);

	static std::string& DebugOut(std::shared_ptr<simutgw::NET_PACKAGE_LOCAL>& ptrNetPack,
		std::string& out_content);

	static std::string& DebugOut(simutgw::NET_PACKAGE_HEAD& netPackHead,
		std::string& out_content);

	/*
	接收消息，并解成本地包

	@param bool bTrimWasteData : 是否删除无效数据
	true -- 删除
	false -- 不删除
	@param bool bCheckTimestamp : 是否检查时间戳
	true -- 检查
	false -- 不检查
	*/
	static int RecvPackage(std::vector<uint8_t>& vctBuffer,
		std::vector<std::shared_ptr<simutgw::NET_PACKAGE_LOCAL>>& recvedDatas,
		bool bTrimWasteData, bool bCheckTimestamp);

	/*
	将网络数据包头转化为本地格式

	@param bool bCheckTimestamp : 是否检查时间戳
	true -- 检查
	false -- 不检查
	*/
	static int NetPackageHeadToLocal(simutgw::NET_PACKAGE_HEAD& stNetPack,
		std::shared_ptr<simutgw::NET_PACKAGE_LOCAL>& ptrNetPack, bool bCheckTimestamp);

	static int LocalPackageToNetBuffer(const int in_iMsgType, const std::string &in_data,
		const uint32_t* in_piTimeStamp, std::vector<uint8_t>& out_vctData);

	static int GenerateNetPackCRC(const std::string& in_strNetPackData, uint32_t& out_checkSum);

	static bool CheckNetPackCRC(const std::shared_ptr<simutgw::NET_PACKAGE_LOCAL>& ptrNetPack);

	static bool PackageCompare(std::shared_ptr<simutgw::NET_PACKAGE_LOCAL>& ptrNetPackDest,
		std::shared_ptr<simutgw::NET_PACKAGE_LOCAL>& ptrNetPackSrc);

	/*
	根据已有的包头生成checksum
	*/
	static uint32_t GenerateChecksum(const std::vector<uint8_t>& vctData, size_t iBeginPos);

	static uint32_t GenerateHeadChecksum(const simutgw::NET_PACKAGE_HEAD& netPackHead);

	static uint32_t GenerateHeadChecksum(const std::shared_ptr<simutgw::NET_PACKAGE_LOCAL>& prtNetPack);

	/*
	得到当前的时间戳
	*/
	static int GetTimeStamp()
	{
		time_t t = time(NULL);

		int j = static_cast<int>(t);
		return j;
	}

	/*
	查看当前时间戳是否是当日

	@param const int iStamp : timestamp
	@param bool bCheck : 是否检查时间戳
	true -- 检查
	false -- 不检查

	@return :
	-1 -- 否
	0 -- 是
	*/
	static int ValidateTimeStamp(const int iStamp, bool bCheck);
};

#endif