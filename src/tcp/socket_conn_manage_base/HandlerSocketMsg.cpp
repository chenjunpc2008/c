#include "HandlerSocketMsg.h"
#include "tool_net/PacketAssembler.h"

#include "util/EzLog.h"

HandlerSocketMsg::HandlerSocketMsg()
{
}


HandlerSocketMsg::~HandlerSocketMsg()
{
}

// 把未处理的buffer加入到主buffer,再分包
void HandlerSocketMsg::AppendBuffer(uint64_t cid, const std::vector<uint8_t>& vecApdBuf,
	std::vector<std::shared_ptr<simutgw::NET_PACKAGE_LOCAL> >& vecApdPack)
{
	std::lock_guard<std::mutex> lock(m_cidMsgMutex);

	PacketAssem(cid, vecApdBuf, vecApdPack);
}

// 把未处理的buffer加入到主buffer，把主buffer中的包再分一次
void HandlerSocketMsg::PacketAssem(uint64_t cid, const std::vector<uint8_t>& vecApdBuf,
	std::vector<std::shared_ptr<simutgw::NET_PACKAGE_LOCAL> >& vecApdPack)
{
	static const string ftag("HandlerSocketMsg::PacketAssem() ");

	std::lock_guard<std::mutex> lock(m_mapCidMsg[cid].msgMutex);

	for (size_t st = 0; st < vecApdBuf.size(); ++st)
	{
		m_mapCidMsg[cid].vecRevBuffer.push_back(vecApdBuf[st]);
	}

	// param 3 = true;
	int iRes = PacketAssembler::RecvPackage(m_mapCidMsg[cid].vecRevBuffer, vecApdPack, true, false);
	if (-1 == iRes)
	{

	}

	// 尝试清空buffer，当超过固定大小时，清空
	const size_t cstMaxSize = 1024 * 1024 * 10;
	if (m_mapCidMsg[cid].vecRevBuffer.size() > cstMaxSize)
	{
		m_mapCidMsg[cid].vecRevBuffer.clear();

		BOOST_LOG_SEV(EzLog::GetLogger(), trivial::warning) << ftag << "clean large receive buff";
	}

}

// 移除id所用的buffer
bool HandlerSocketMsg::RemoveId(uint64_t cid)
{
	std::lock_guard<std::mutex> lock(m_cidMsgMutex);

	if (0 < m_mapCidMsg.erase(cid))
	{
		return true;
	}
	else
	{
		return false;
	}
}

// 移除所有id所用的buffer
void HandlerSocketMsg::RemoveAllId(void)
{
	std::lock_guard<std::mutex> lock(m_cidMsgMutex);

	m_mapCidMsg.clear();
}