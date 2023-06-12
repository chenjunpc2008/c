#ifndef __CONF_NET_MSG_H__
#define __CONF_NET_MSG_H__

#include <vector>
#include <string>
#include <mutex>
#include <memory>

#include "NetPackage.h"

namespace simplePack
{
    /*
    socket用户的消息缓存
    包括：
    未分包部分和已分包部分
    */
    struct SocketUserMessage
    {
        std::mutex msgMutex;

        // 未分包部分
        std::vector<uint8_t> vecRevBuffer;

        // 已分包部分
        std::vector<std::shared_ptr<simplePack::NET_PACKAGE_LOCAL>> vecRevDatas;
    };
}

#endif