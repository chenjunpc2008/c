#ifndef __NET_PACKAGE_H__
#define __NET_PACKAGE_H__

#include <string>
#include <stdint.h>

/*
网络数据包
*/
namespace simplePack
{

    // 将字节对齐方式设为1
#pragma pack(push, 1)

    // 网络数据包头
    struct NET_PACKAGE_HEAD
    {
        // 包头起始字段 BeginString  4字节 0-3
        char beginstring[4];
        // 封包版本号 4字节 4-7
        char version[4];
        // 包时间戳 4字节 8-11
        uint32_t timestamp;
        // 包数据类型 4字节 12-15
        char type[4];
        // 包数据的内容长度，不包含此包结构和包头尾 4字节 16-19
        uint32_t datalen;
        // 包数据校验值 md5二进制数据 4字节 20-23
        uint32_t dataCksum;
        // 包头checksum，4字节 24-27
        uint32_t headcksum;
        // ReserveField 填充数据块，4字节 28-31
        char reserveField[4];
    };

    // 将当前字节对齐值设为默认值(通常是4)
#pragma pack(pop)

    // 本地化的网络数据包头
    struct NET_PACKAGE_HEAD_LOCAL
    {
        // 包头起始字段 BeginString  4字节 0-3
        std::string beginstring;
        // 封包版本号 4字节 4-7
        int iVersion;
        // 包时间戳 4字节 8-11
        uint32_t iTimeStamp;
        // 包数据类型 4字节 12-15
        int iType;
        // 包数据的内容长度，不包含此包结构和包头尾 4字节 16-19
        uint32_t iDatalen;
        // 包数据校验值 md5二进制数据 4字节 20-23
        uint32_t iDataCksum;
        // 包头checksum，4字节 24-27
        uint32_t iHeadCksum;
    };

    // 包头长度，为固定大小32字节
    static unsigned int NET_PACKAGE_HEADLEN = sizeof(NET_PACKAGE_HEAD);

    static const char NETPACK_BEGINSTRING[4] = {'N', 'E', 'T', ':'};

    static const unsigned int NETPACK_BEGINSTR_LEN = 4;

    static const int NETPACK_VERSION = 2;

    static const char NETPACK_VERSION_STR[4] = {'0', '0', '0', '2'};

    static const unsigned int NETPACK_VERSION_LEN = 4;

    static const unsigned int NETPACK_TIMESTAMP_LEN = 4;

    static const unsigned int NETPACK_TYPE_LEN = 4;

    static const unsigned int NETPACK_DATALEN_LEN = 4;

    static const unsigned int NETPACK_DATACKSUM_LEN = 4;

    static const unsigned int NETPACK_HEADCKSUM_LEN = 4;

    static const unsigned int NETPACK_RESERVE_FIELD_LEN = 4;

    struct NET_PACKAGE_LOCAL
    {
        struct NET_PACKAGE_HEAD_LOCAL head;
        std::string data;
    };

};

#endif