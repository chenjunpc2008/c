#ifndef __CONF_NET_MSG_H__
#define __CONF_NET_MSG_H__

#include <vector>
#include <string>
#include <mutex>
#include <memory>

#include "tool_net/NetPackage.h"

namespace simutgw
{
    using namespace std;

    enum NET_MSG_TYPE
    {
        // Heartbeat 心跳消息
        Heartbeat = 0,
        // Logon 登录消息
        Logon = 10,
        // TestRequest 测试请求消息
        TestRequest = 1,
        // Resend 重发请求消息
        Resend = 2,
        // Reject 会话拒绝消息
        Reject = 3,
        // SeqReset 序号重设消息
        SeqReset = 4,
        // Logout 注销消息
        Logout = 5,
        // Execution 执行
        Order = 7,
        // Execution Report 执行报告
        Report = 8
    };

    /*
    socket登录信息
    */
    struct SocketUser
    {
        //
        string strMsgtype;

        string strVersion;

        // 账号
        string strAccountid;

        // 密码
        string strPwd;

        //
        string strText;
    };

    /*
    socket登录用户的信息
    */
    struct SockerUserInfo
    {
        // 账号
        string strAccountid;

        // 登录标志
        bool bLogon;

        // 下一条发送消息序号
        uint64_t ui64NextSendSeq;

        SockerUserInfo()
        {
            bLogon = false;
            ui64NextSendSeq = 0;
        }
    };

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
        std::vector<std::shared_ptr<simutgw::NET_PACKAGE_LOCAL>> vecRevDatas;
    };

    /*
        收到的请求消息
        */
    struct SocketMsgInfo
    {
        uint64_t ui64MsgType;
        uint64_t ui64ApplID;
        uint64_t ui64SeqNum;
        uint64_t ui64Add_Balance;
        uint64_t ui64Add_HK_Balance;
        uint64_t ui64Add_US_Balance;
        uint64_t ui64Add_Quantity;
        std::string strMsgType;
        std::string strAccount;
        std::string strTradeGroup;
        std::string strSeat;
        std::string strTrader;
        std::string strAdd_Balance;
        std::string strAdd_HK_Balance;
        std::string strAdd_US_Balance;
        std::string strFileType;
        std::string strFilePath;

        std::string strSecurityID;
        std::string strQuantity;

        std::string strCmdValue;
    };
}

#endif