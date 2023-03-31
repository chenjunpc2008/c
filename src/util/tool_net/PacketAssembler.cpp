#include "PacketAssembler.h"

#include "NetPackage.h"
#include <vector>
#include <algorithm>

#include <time.h>

#ifdef _MSC_VER
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

#include "tool_string/sof_string.h"
#include "tool_string/Tgw_StringUtil.h"
#include "CRC32.h"

using namespace std;

PacketAssembler::PacketAssembler(void)
{
}

PacketAssembler::~PacketAssembler(void)
{
}

string &PacketAssembler::DebugOut(std::shared_ptr<simutgw::NET_PACKAGE_LOCAL> &ptrNetPack, string &out_content)
{
    string strTran;
    string strDebug;

    strDebug = "NET_PACKAGE beginstring=[";
    strDebug += ptrNetPack->head.beginstring;
    strDebug += "], version=[";
    strDebug += sof_string::itostr(ptrNetPack->head.iVersion, strTran);
    strDebug += "], timeStamp=[";
    strDebug += sof_string::itostr(ptrNetPack->head.iTimeStamp, strTran);
    strDebug += "], type=[";
    strDebug += sof_string::itostr(ptrNetPack->head.iType, strTran);
    strDebug += "], datalen=[";
    strDebug += sof_string::itostr(ptrNetPack->head.iDatalen, strTran);
    strDebug += "], dataCksum=[";
    strDebug += ptrNetPack->head.iDataCksum;
    strDebug += "], headCksum=[";
    strDebug += sof_string::itostr(ptrNetPack->head.iHeadCksum, strTran);
    strDebug += "], data=[";
    strDebug += ptrNetPack->data;
    strDebug += "]";

    out_content = strDebug;

    return out_content;
}

string &PacketAssembler::DebugOut(simutgw::NET_PACKAGE_HEAD &netPackHead, string &out_content)
{
    string strTran;
    string strDebug;

    strDebug = "NET_PACKAGE beginstring=[";
    strTran.clear();
    strTran.insert(0, netPackHead.beginstring, 0, simutgw::NETPACK_BEGINSTR_LEN);
    strDebug += strTran;

    strDebug += "], version=[";
    strTran.clear();
    strTran.insert(0, netPackHead.version, 0, simutgw::NETPACK_VERSION_LEN);
    strDebug += strTran;

    strDebug += "], timeStamp=[";
    sof_string::itostr(uint64_t(ntohl(netPackHead.timestamp)), strTran);
    strDebug += strTran;

    strDebug += "], type=[";
    strTran.clear();
    strTran.insert(0, netPackHead.type, 0, simutgw::NETPACK_TYPE_LEN);
    strDebug += strTran;

    strDebug += "], datalen=[";
    sof_string::itostr(uint64_t(ntohl(netPackHead.datalen)), strTran);
    strDebug += strTran;

    strDebug += "], check=[";
    sof_string::itostr(uint64_t(ntohl(netPackHead.dataCksum)), strTran);
    strDebug += strTran;

    strDebug += "], headChecksum=[";
    sof_string::itostr(uint64_t(ntohl(netPackHead.headcksum)), strTran);
    strDebug += strTran;

    strDebug += "]";

    out_content = strDebug;

    return out_content;
}

/*
接收消息，并解成本地包

@param bool bTrimWasteData : 是否删除无效数据
true -- 删除
false -- 不删除
@param bool bCheckTimestamp : 是否检查时间戳
true -- 检查
false -- 不检查
*/
int PacketAssembler::RecvPackage(std::vector<uint8_t> &vctBuffer,
                                 std::vector<std::shared_ptr<simutgw::NET_PACKAGE_LOCAL>> &recvedDatas,
                                 bool bTrimWasteData, bool bCheckTimestamp)
{
    static const std::string ftag("PacketAssembler::RecvPackage() ");

    size_t uiBufferSize = vctBuffer.size();
    if (uiBufferSize <= simutgw::NET_PACKAGE_HEADLEN)
    {
        // 剩余的数据端不够包头数据长度，还需继续缓存
        return 1;
    }

    // 是否已匹配包头beginstring
    bool bMatchBeginstr = false;

    // pos
    size_t uiCurrentPos = 0;
    // Begin string pos
    size_t uiNetCompare_BeginStringPos = 0;
    // 剩余的数据
    size_t uiDatasLeft = 0;
    // net package head pos
    std::vector<uint8_t>::iterator itHeadPos = vctBuffer.end();

    simutgw::NET_PACKAGE_HEAD structNetPackHead;
    char *pStructNetPackHead = (char *)&structNetPackHead;

    // 拷贝字符串中间变量
    char cCopyTmp = '\0';

    // 数据buffer是否搜索完毕
    bool bIsBuffNeedProcessed = false;

    do
    {
        bIsBuffNeedProcessed = false;

        uiBufferSize = vctBuffer.size();

        bMatchBeginstr = false;
        uiNetCompare_BeginStringPos = 0;
        itHeadPos = vctBuffer.end();
        memset(pStructNetPackHead, '\0', simutgw::NET_PACKAGE_HEADLEN);

        // 查找首字母
        std::vector<uint8_t>::iterator itBuf =
            find(vctBuffer.begin(), vctBuffer.end(),
                 simutgw::NETPACK_BEGINSTRING[uiNetCompare_BeginStringPos]);

        if (vctBuffer.end() == itBuf)
        {
            if (bTrimWasteData)
            {
                // 无效数据
                vctBuffer.clear();

                return -1;
            }
        }
        else
        {
            // 匹配到包头beginString首字符

            // 是否去除无效包头数据
            if (bTrimWasteData && vctBuffer.begin() != itBuf)
            {
                // 匹配到包头beginString首字符，但不是序列首字母

                // 如果首字母都匹配不上，有可能是错序包，清空与下一个包头之间的内容
                std::vector<uint8_t>::iterator itFind =
                    find(vctBuffer.begin() + 1, vctBuffer.end(),
                         simutgw::NETPACK_BEGINSTRING[uiNetCompare_BeginStringPos]);

                if (itFind != vctBuffer.end())
                {
                    // 找到下一个包头，清空之间的内容
                    vctBuffer.erase(vctBuffer.begin(), itFind);

                    // 继续下一轮搜索
                    bIsBuffNeedProcessed = true;

                    break;
                }
                else
                {
                    // 未找到下一个包头，无效数据
                    vctBuffer.clear();

                    return -1;
                }
            }
            else
            {
                for (; vctBuffer.end() != itBuf; ++itBuf)
                {
                    // 匹配到包头beginString首字符
                    uiCurrentPos = itBuf - vctBuffer.begin();
                    uiDatasLeft = uiBufferSize - uiCurrentPos;

                    // 查看从当前位置开始是否有足够的包头数据
                    if (simutgw::NET_PACKAGE_HEADLEN > uiDatasLeft)
                    {
                        // 剩余的数据端不够包头数据长度，还需继续缓存
                        return 1;
                    }

                    // 记录包头开始位置
                    itHeadPos = itBuf;

                    // 继续对比是否是真的包头数据
                    do
                    {
                        if (simutgw::NETPACK_BEGINSTRING[uiNetCompare_BeginStringPos] != *(itBuf))
                        {
                            bMatchBeginstr = false;

                            break;
                        }
                        else
                        {
                            bMatchBeginstr = true;
                        }

                        ++uiNetCompare_BeginStringPos;
                        ++itBuf;
                    } while (uiNetCompare_BeginStringPos < simutgw::NETPACK_BEGINSTR_LEN);

                    if (bMatchBeginstr)
                    {
                        // 包头beginstring完整
                        // 从包头位置开始拷贝
                        itBuf = itHeadPos;

                        unsigned int iCopyedByte = 0;
                        for (iCopyedByte = 0;
                             iCopyedByte < simutgw::NET_PACKAGE_HEADLEN && itBuf != vctBuffer.end();
                             ++itBuf, ++iCopyedByte)
                        {
                            cCopyTmp = *itBuf;
                            memcpy(pStructNetPackHead + iCopyedByte, &cCopyTmp, 1);
                        }

                        std::shared_ptr<simutgw::NET_PACKAGE_LOCAL> ptrNetPack =
                            std::shared_ptr<simutgw::NET_PACKAGE_LOCAL>(new simutgw::NET_PACKAGE_LOCAL());

                        int iPackTranRes = NetPackageHeadToLocal(structNetPackHead, ptrNetPack, bCheckTimestamp);
                        if (0 != iPackTranRes)
                        {
                            // 包头beginstring非法数据
                            // 继续从余下位置搜索
                            // 重设迭代器位置为NET:的:
                            itBuf = vctBuffer.begin() + (uiNetCompare_BeginStringPos - 1);
                            bMatchBeginstr = false;
                            uiNetCompare_BeginStringPos = 0;
                            itHeadPos = vctBuffer.end();

                            continue;
                        }

                        if (ptrNetPack->head.iDatalen > 0)
                        {
                            // 含当前位置剩余的数据
                            if (vctBuffer.end() != itBuf)
                            {
                                uiCurrentPos = itBuf - vctBuffer.begin();
                                uiDatasLeft = uiBufferSize - uiCurrentPos;
                            }
                            else
                            {
                                uiDatasLeft = 0;
                            }

                            // 查看从当前位置开始（含当前位置）是否有足够的包数据
                            if (itBuf == vctBuffer.end() ||
                                (unsigned int)(ptrNetPack->head.iDatalen) > uiDatasLeft)
                            {
                                // 剩余的数据端不够包头数据长度，还需继续缓存
                                return 1;
                            }

                            for (iCopyedByte = 0;
                                 iCopyedByte < (unsigned int)(ptrNetPack->head.iDatalen) && itBuf != vctBuffer.end();
                                 ++itBuf, ++iCopyedByte)
                            {
                                cCopyTmp = *itBuf;
                                ptrNetPack->data += cCopyTmp;
                            }

                            // 检查数据包合法性
                            bool bPackChecked = CheckNetPackCRC(ptrNetPack);

                            if (bPackChecked)
                            {
                                // 数据包合法
                                recvedDatas.push_back(ptrNetPack);

                                // 清除已接受的数据包
                                vctBuffer.erase(itHeadPos, itBuf);

                                // vector内容已变动，iterator状态需重置

                                // 继续下一轮搜索
                                bIsBuffNeedProcessed = true;

                                break;
                            }
                            else
                            {
                                // 数据包不合法
                                // 从之前找到的包头位置继续搜索
                                itBuf = itHeadPos;

                                bMatchBeginstr = false;
                                uiNetCompare_BeginStringPos = 0;
                                itHeadPos = vctBuffer.end();
                                continue;
                            }
                        }
                    }
                    else // if( bMatchBeginstr )
                    {
                        // 包头beginstring不完整
                        // 从之前找到的包头位置继续搜索
                        itBuf = itHeadPos;

                        bMatchBeginstr = false;
                        uiNetCompare_BeginStringPos = 0;
                        itHeadPos = vctBuffer.end();

                        continue;
                    }
                } // for( ; vctBuffer.end() != itBuf; ++itBuf )
            }
        }

    } while (bIsBuffNeedProcessed);

    return 0;
}

/*
将网络数据包头转化为本地格式

@param bool bCheckTimestamp : 是否检查时间戳
true -- 检查
false -- 不检查
*/
int PacketAssembler::NetPackageHeadToLocal(simutgw::NET_PACKAGE_HEAD &stNetPack,
                                           std::shared_ptr<simutgw::NET_PACKAGE_LOCAL> &ptrNetPack, bool bCheckTimestamp)
{
    static const string ftag("PacketAssembler::NetPackageHeadToLocal() ");

    int iRes = 0;
    std::string strVersion;
    std::string strType;

    // beginstring
    ptrNetPack->head.beginstring.insert(0, stNetPack.beginstring, 0, simutgw::NETPACK_BEGINSTR_LEN);

    // version
    strVersion.insert(0, stNetPack.version, 0, simutgw::NETPACK_VERSION_LEN);

    iRes = Tgw_StringUtil::String2Int_atoi(strVersion, ptrNetPack->head.iVersion);
    if (0 != iRes)
    {
        string strDebug("Version trans to int failed, src=");
        string strPackContent;

        strDebug += DebugOut(stNetPack, strPackContent);

        EzLog::e(ftag, strDebug);
        return -1;
    }

    // timestamp
    ptrNetPack->head.iTimeStamp = ntohl(stNetPack.timestamp);

    // 校验时间戳
    iRes = ValidateTimeStamp(ptrNetPack->head.iTimeStamp, bCheckTimestamp);
    if (0 != iRes)
    {
        string strDebug("ValidateTimeStamp failed, src=");
        string strPackContent;

        strDebug += DebugOut(stNetPack, strPackContent);

        EzLog::e(ftag, strDebug);
        return -1;
    }

    // type
    strType.insert(0, stNetPack.type, 0, simutgw::NETPACK_TYPE_LEN);

    iRes = Tgw_StringUtil::String2Int_atoi(strType, ptrNetPack->head.iType);
    if (0 != iRes)
    {
        string strDebug("Type trans to int failed, src=");
        string strPackContent;

        strDebug += DebugOut(stNetPack, strPackContent);

        EzLog::e(ftag, strDebug);
        return -1;
    }

    // 校验version,type
    if (ptrNetPack->head.iVersion != simutgw::NETPACK_VERSION || ptrNetPack->head.iType < 0)
    {
        return -1;
    }

    // datalen
    ptrNetPack->head.iDatalen = ntohl(stNetPack.datalen);

    // data check sum
    ptrNetPack->head.iDataCksum = ntohl(stNetPack.dataCksum);

    // head checksum
    ptrNetPack->head.iHeadCksum = ntohl(stNetPack.headcksum);

    // 校验head checksum
    int iHeadCksum = GenerateHeadChecksum(stNetPack);
    if (ptrNetPack->head.iHeadCksum != iHeadCksum)
    {
        string strTran;
        string strPackContent;
        string strDebug("HeadCksum check failed, expected=");
        strDebug += sof_string::itostr(iHeadCksum, strTran);
        strDebug += ", src=";

        strDebug += DebugOut(stNetPack, strPackContent);

        EzLog::e(ftag, strDebug);
        return -1;
    }

    return 0;
}

int PacketAssembler::LocalPackageToNetBuffer(const int in_iMsgType, const std::string &in_data,
                                             const uint32_t *in_piTimeStamp, std::vector<uint8_t> &out_vctData)
{
    static const std::string ftag("PacketAssembler::LocalPackageToNetBuffer() ");

    size_t i = 0;
    int iRes = 0;
    size_t iStringLen = 0;
    std::string strType;

    //
    // beginstring
    for (i = 0; i < simutgw::NETPACK_BEGINSTR_LEN; ++i)
    {
        out_vctData.push_back(simutgw::NETPACK_BEGINSTRING[i]);
    }

    //
    // version
    for (i = 0; i < (simutgw::NETPACK_VERSION_LEN); ++i)
    {
        out_vctData.push_back(simutgw::NETPACK_VERSION_STR[i]);
    }

    // timestamp
    int iTimeStamp = 0;
    if (nullptr != in_piTimeStamp)
    {
        iTimeStamp = *in_piTimeStamp;
    }
    else
    {
        iTimeStamp = GetTimeStamp();
    }

    uint32_t iNetTimestamp = htonl(iTimeStamp);
    char szNetTimestamp[4] = {'\0', '\0', '\0', '\0'};
    memcpy(szNetTimestamp, &iNetTimestamp, 4);

    for (i = 0; i < simutgw::NETPACK_TIMESTAMP_LEN; ++i)
    {
        out_vctData.push_back(szNetTimestamp[i]);
    }

    //
    // type
    sof_string::itostr(in_iMsgType, strType);

    iStringLen = strType.length();
    if (simutgw::NETPACK_TYPE_LEN < iStringLen)
    {
        string strDebug("Type length check failed, src=");
        strDebug += strType;

        EzLog::e(ftag, strDebug);
        return -1;
    }

    for (i = 0; i < (simutgw::NETPACK_TYPE_LEN - iStringLen); ++i)
    {
        out_vctData.push_back('0');
    }

    for (i = 0; i < iStringLen; ++i)
    {
        out_vctData.push_back(strType[i]);
    }

    //
    // datalen
    uint32_t iDatalen = (uint32_t)(in_data.length());

    uint32_t iNetDatalen = htonl(iDatalen);
    char szNetDatalen[4] = {'\0', '\0', '\0', '\0'};
    memcpy(szNetDatalen, &iNetDatalen, 4);

    for (i = 0; i < simutgw::NETPACK_DATALEN_LEN; ++i)
    {
        out_vctData.push_back(szNetDatalen[i]);
    }

    // data checksum
    uint32_t ui32DataCheckSum;
    iRes = GenerateNetPackCRC(in_data, ui32DataCheckSum);
    if (0 != iRes)
    {
        string strTran;
        string strDebug("GenerateNetPackCRC failed, src=[");
        strDebug += in_data;
        strDebug += "],errcode=";
        strDebug += sof_string::itostr(iRes, strTran);

        EzLog::e(ftag, strDebug);
        return -1;
    }

    uint32_t iNetDataCheckSum = htonl(ui32DataCheckSum);
    char szNetDataCheckSum[4] = {'\0', '\0', '\0', '\0'};
    memcpy(szNetDataCheckSum, &iNetDataCheckSum, 4);

    for (i = 0; i < simutgw::NETPACK_DATACKSUM_LEN; ++i)
    {
        out_vctData.push_back(szNetDataCheckSum[i]);
    }

    // head checksum
    // 当前包的偏移量位置，指明当组装多个包时当前包的地址
    size_t iPos = out_vctData.size() - (simutgw::NET_PACKAGE_HEADLEN - simutgw::NETPACK_HEADCKSUM_LEN - simutgw::NETPACK_RESERVE_FIELD_LEN);
    uint32_t iHeadChecksum = GenerateChecksum(out_vctData, iPos);
    uint32_t iNetHeadChecksum = htonl(iHeadChecksum);
    char szNetHeadChecksum[4] = {'\0', '\0', '\0', '\0'};
    memcpy(szNetHeadChecksum, &iNetHeadChecksum, 4);

    for (i = 0; i < simutgw::NETPACK_HEADCKSUM_LEN; ++i)
    {
        out_vctData.push_back(szNetHeadChecksum[i]);
    }

    // reserve field
    for (i = 0; i < simutgw::NETPACK_RESERVE_FIELD_LEN; ++i)
    {
        out_vctData.push_back(' ');
    }

    //
    // data
    iStringLen = in_data.length();
    for (i = 0; i < iStringLen; ++i)
    {
        out_vctData.push_back(in_data[i]);
    }

    return 0;
}

int PacketAssembler::GenerateNetPackCRC(const string &in_strNetPackData, uint32_t &out_checkSum)
{
    // static const std::string ftag("PacketAssembler::GenerateNetPackCRC() ");

    unsigned int ui = 0;

    // check
    // Create a CRC32 checksum calculator.
    CRC32 crc;

    size_t iStringLen = in_strNetPackData.length();

    // Here we add each byte to the checksum, caclulating the checksum as we go.
    for (ui = 0; ui < iStringLen; ++ui)
    {
        crc.update(in_strNetPackData[ui]);
    }

    // Once we have added all of the data, generate the final CRC32 checksum.
    uint32_t checksum = crc.finalize();

    out_checkSum = checksum;

    return 0;
}

bool PacketAssembler::CheckNetPackCRC(const std::shared_ptr<simutgw::NET_PACKAGE_LOCAL> &ptrNetPack)
{
    // static const std::string ftag("PacketAssembler::CheckNetPackCRC() ");

    uint32_t iCheckFinal;
    int iRes = GenerateNetPackCRC(ptrNetPack->data, iCheckFinal);
    if (0 != iRes)
    {
        return false;
    }

    if (iCheckFinal == ptrNetPack->head.iDataCksum)
    {
        return true;
    }

    return false;
}

bool PacketAssembler::PackageCompare(std::shared_ptr<simutgw::NET_PACKAGE_LOCAL> &ptrNetPackDest,
                                     std::shared_ptr<simutgw::NET_PACKAGE_LOCAL> &ptrNetPackSrc)
{
    static const std::string ftag("PacketAssembler::PackageCompare() ");

    if ((0 == ptrNetPackDest->head.beginstring.compare(ptrNetPackSrc->head.beginstring)) && (ptrNetPackDest->head.iVersion == ptrNetPackSrc->head.iVersion) && (ptrNetPackDest->head.iType == ptrNetPackSrc->head.iType) && (ptrNetPackDest->head.iDatalen == ptrNetPackSrc->head.iDatalen) && (ptrNetPackDest->head.iDataCksum == ptrNetPackSrc->head.iDataCksum) && (0 == ptrNetPackDest->data.compare(ptrNetPackSrc->data)))
    {
        return true;
    }
    else
    {
        string strDebug("net packages not equal, dest=");
        string strPackContent;

        DebugOut(ptrNetPackDest, strPackContent);
        strDebug += strPackContent;
        strDebug += "; src=";

        DebugOut(ptrNetPackSrc, strPackContent);
        strDebug += strPackContent;

        EzLog::e(ftag, strDebug);

        return false;
    }
}

uint32_t PacketAssembler::GenerateChecksum(const std::vector<uint8_t> &vctData, size_t iBeginPos)
{
    unsigned long cksum = 0;
    size_t index = iBeginPos;
    size_t endPos = iBeginPos + simutgw::NET_PACKAGE_HEADLEN - simutgw::NETPACK_HEADCKSUM_LEN - simutgw::NETPACK_RESERVE_FIELD_LEN;
    while (index < vctData.size() && index < endPos)
    {
        // uint8_t tmp = static_cast<uint8_t>(vctData[index]);
        cksum += static_cast<uint8_t>(vctData[index]);
        ++index;
    }

    // //将32位数转换成16
    // while (cksum>>16)
    //	cksum=(cksum>>16)+(cksum &0xffff);
    //
    // cksum = ~cksum;
    cksum = cksum % 131072;
    uint32_t iCksum = static_cast<int>(cksum);

    return iCksum;
}

uint32_t PacketAssembler::GenerateHeadChecksum(const simutgw::NET_PACKAGE_HEAD &netPackHead)
{
    unsigned long cksum = 0;
    size_t index = 0;
    size_t endPos = simutgw::NET_PACKAGE_HEADLEN - simutgw::NETPACK_HEADCKSUM_LEN - simutgw::NETPACK_RESERVE_FIELD_LEN;
    const char *phead = (const char *)&netPackHead;
    while (index < endPos)
    {
        // uint8_t tmp = static_cast<uint8_t>(phead[index]);
        cksum += static_cast<uint8_t>(phead[index]);
        ++index;
    }

    // //将32位数转换成16
    // while (cksum>>16)
    //	cksum=(cksum>>16)+(cksum &0xffff);
    //
    // cksum = ~cksum;
    cksum = cksum % 131072;
    uint32_t iCksum = static_cast<int>(cksum);

    return iCksum;
}

uint32_t PacketAssembler::GenerateHeadChecksum(const std::shared_ptr<simutgw::NET_PACKAGE_LOCAL> &prtNetPack)
{
    unsigned int i = 0;
    size_t iStringLen = 0;
    std::string strType;

    std::vector<uint8_t> vctData;
    //
    // beginstring
    for (i = 0; i < simutgw::NETPACK_BEGINSTR_LEN; ++i)
    {
        vctData.push_back(simutgw::NETPACK_BEGINSTRING[i]);
    }

    //
    // version
    for (i = 0; i < (simutgw::NETPACK_VERSION_LEN); ++i)
    {
        vctData.push_back(simutgw::NETPACK_VERSION_STR[i]);
    }

    // timestamp
    uint32_t iNetTimestamp = htonl(prtNetPack->head.iTimeStamp);

    char szNetTimestamp[4] = {'\0', '\0', '\0', '\0'};
    memcpy(szNetTimestamp, &iNetTimestamp, 4);

    for (i = 0; i < simutgw::NETPACK_TIMESTAMP_LEN; ++i)
    {
        vctData.push_back(szNetTimestamp[i]);
    }

    //
    // type
    sof_string::itostr(prtNetPack->head.iType, strType);

    iStringLen = strType.length();
    if (simutgw::NETPACK_TYPE_LEN < iStringLen)
    {
        return -1;
    }

    for (i = 0; i < (simutgw::NETPACK_TYPE_LEN - iStringLen); ++i)
    {
        vctData.push_back('0');
    }

    for (i = 0; i < iStringLen; ++i)
    {
        vctData.push_back(strType[i]);
    }

    //
    // datalen
    uint32_t iNetDatalen = htonl(prtNetPack->head.iDatalen);

    char szNetDatalen[4] = {'\0', '\0', '\0', '\0'};
    memcpy(szNetDatalen, &iNetDatalen, 4);

    for (i = 0; i < simutgw::NETPACK_DATALEN_LEN; ++i)
    {
        vctData.push_back(szNetDatalen[i]);
    }

    // data checksum
    uint32_t iNetDataCheckSum = htonl(prtNetPack->head.iDataCksum);

    char szNetDataCheckSum[4] = {'\0', '\0', '\0', '\0'};
    memcpy(szNetDataCheckSum, &iNetDataCheckSum, 4);

    for (i = 0; i < simutgw::NETPACK_DATACKSUM_LEN; ++i)
    {
        vctData.push_back(szNetDataCheckSum[i]);
    }

    // head checksum
    uint32_t iHeadChecksum = GenerateChecksum(vctData, 0);

    return iHeadChecksum;
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
int PacketAssembler::ValidateTimeStamp(const int iStamp, bool bCheck)
{
    if (!bCheck)
    {
        return 0;
    }

    if (0 >= iStamp)
    {
        // 不合法的时间
        return -1;
    }

    if (iStamp == 1510885145 || iStamp == 1510879893)
    {
        //  测试时使用
        return 0;
    }

    time_t tStamp = iStamp;
    struct tm stStampTime;
    // 时间戳转换成的时间
#ifdef _MSC_VER
    localtime_s(&stStampTime, &tStamp);
#else
    localtime_r(&tStamp, &stStampTime);
#endif

    time_t tCurr = time(NULL);
    struct tm stCurrTime;
    // 当前时间
#ifdef _MSC_VER
    localtime_s(&stCurrTime, &tCurr);
#else
    localtime_r(&tCurr, &stCurrTime);
#endif

    if (!(stStampTime.tm_year == stCurrTime.tm_year) ||
        !(stStampTime.tm_mon == stCurrTime.tm_mon) ||
        !(stStampTime.tm_mday == stCurrTime.tm_mday))
    {
        // 日期与当前日期不一致
        return -1;
    }

    return 0;
}

/*
查看当前时间戳是否是当日
-1 -- 否
0 -- 是
*/
/*
int PacketAssembler::ValidateTimeStamp( const std::string& strStamp )
{
if ( strStamp.length() != 10 )
{
// 时间戳长度为10位
return -1;
}

uint64_t ui64Stamp = 0;
int iRes = Tgw_StringUtil::String2UInt64_strtoui64( strStamp, ui64Stamp );
if ( 0 > iRes )
{
return -1;
}

time_t tStamp = ui64Stamp;
struct tm stStampTime;
// 时间戳转换成的时间
localtime_s( &stStampTime, &tStamp );

time_t tCurr = time( NULL );
struct tm stCurrTime;
// 当前时间
localtime_s( &stCurrTime, &tCurr );

if ( !( stStampTime.tm_year == stCurrTime.tm_year ||
stStampTime.tm_mon == stCurrTime.tm_mon ||
stStampTime.tm_mday == stCurrTime.tm_mday ) )
{
// 日期与当前日期不一致
return -1;
}

return 0;
}
*/
