#include "EventBaseHandler.h"

#include <iostream>
#include <string.h>

#include "util/tool_log/normal_log_value.h"
#include "util/tool_string/sof_string.h"

#include "tcp/socket_conn_manage_base/ConnectionInformation.h"

using namespace std;

EventBaseHandler::EventBaseHandler(void) {}

EventBaseHandler::~EventBaseHandler(void) {}

void EventBaseHandler::OnReceiveData(uint64_t cid,
                                     std::vector<uint8_t> const &data)
{
    static const string ftag("EventBaseHandler::OnReceiveData() ");

    string strTran;
    string sDebug = "id=";
    sDebug += sof_string::itostr(cid, strTran);
    sDebug += " data=[";
    std::vector<uint8_t>::const_iterator it;
    for (it = data.begin(); it != data.end(); ++it)
    {
        sDebug += *it;
        sDebug += ",";
    }
    sDebug += "]";

    cout << n_log::INFO << ftag << sDebug << endl;
}

void EventBaseHandler::OnSentData(uint64_t cid, uint64_t byteTransferred)
{
    static const string ftag("EventBaseHandler::OnSentData() ");

    string strTran;
    string sDebug = "id=";
    sDebug += sof_string::itostr(cid, strTran);
    sDebug += ", byteTransferred=";
    sDebug += sof_string::itostr(byteTransferred, strTran);

    cout << n_log::INFO << ftag << sDebug << endl;
}

void EventBaseHandler::OnClientDisconnect(uint64_t cid, int errorcode)
{
    static const string ftag("EventBaseHandler::OnClientDisconnect() ");

    string strTran;
    string sDebug = "id=";
    sDebug += sof_string::itostr(cid, strTran);
    sDebug += ", error=";
    sDebug += sof_string::itostr(errorcode, strTran);
    sDebug += ", serror=";
    sDebug += strerror(errorcode);

    cout << n_log::INFO << ftag << sDebug << endl;
}

void EventBaseHandler::OnDisconnect(uint64_t cid, int errorcode)
{
    static const string ftag("EventBaseHandler::OnDisconnect() ");

    string strTran;
    string sDebug = "id=";
    sDebug += sof_string::itostr(cid, strTran);
    sDebug += ", error=";
    sDebug += sof_string::itostr(errorcode, strTran);
    sDebug += ", serror=";
    sDebug += strerror(errorcode);

    cout << n_log::INFO << ftag << sDebug << endl;
}

void EventBaseHandler::OnServerClose(int errorCode)
{
    // OnServerClose时一般是程序退出时，因为是多线程程序，可能会引用到已经释放的资源
    /*
    static const string ftag("EventBaseHandler::OnServerClose() ");

    string strTran;
    string sDebug = "error=";
    sDebug += sof_string::itostr(errorCode, strTran);
    sDebug += ", serror=";
    sDebug += strerror(errorCode);

    cout << ftag << sDebug<< endl;
    */
}

void EventBaseHandler::OnEvent(const std::string &msg)
{
    static const string ftag("EventBaseHandler::OnEvent() ");

    cout << n_log::INFO << ftag << msg << endl;
}

void EventBaseHandler::OnErrorCode(const std::string &errorMsg, int errorCode)
{
    static const string ftag("EventBaseHandler::OnErrorCode() ");

    string strTran;
    string sDebug = errorMsg;
    sDebug += ", error=";
    sDebug += sof_string::itostr(errorCode, strTran);
    sDebug += ", serror=";
    sDebug += strerror(errorCode);

    cout << n_log::ERROR << ftag << sDebug << endl;
}

void EventBaseHandler::OnErrorStr(const std::string &errorMsg)
{
    static const string ftag("EventBaseHandler::OnErrorStr() ");

    cout << n_log::ERROR << ftag << errorMsg << endl;
}
