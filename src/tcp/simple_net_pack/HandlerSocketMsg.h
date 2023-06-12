#ifndef __HANDLER_SOCKET_MSG_H__
#define __HANDLER_SOCKET_MSG_H__

#include <mutex>
#include <map>

#include "conf_net_msg.h"

/*
socket message的消息缓存、解包类
*/
class HandlerSocketMsg
{
    /*
    member
    */
private:
    std::mutex m_cidMsgMutex;

    // 消息缓存
    std::map<uint64_t, struct simplePack::SocketUserMessage> m_mapCidMsg;

public:
    HandlerSocketMsg();
    virtual ~HandlerSocketMsg();

    // 把未处理的buffer加入到主buffer,再分包
    void AppendBuffer(uint64_t cid, const std::vector<uint8_t> &vecApdBuf, std::vector<std::shared_ptr<simplePack::NET_PACKAGE_LOCAL>> &vecApdPack);

    // 把移除id所用的buffer
    bool RemoveId(uint64_t cid);

    // 移除所有id所用的buffer
    void RemoveAllId(void);

private:
    // 把未处理的buffer加入到主buffer，把主buffer中的包再分一次
    void PacketAssem(uint64_t cid, const std::vector<uint8_t> &vecApdBuf,
                     std::vector<std::shared_ptr<simplePack::NET_PACKAGE_LOCAL>> &vecApdPack);
};

#endif
