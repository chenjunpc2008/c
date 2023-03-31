#include "EventBaseHandler.h"

#include <string.h>
#include <iostream>

#include "tool_string/sof_string.h"

#include "socket_conn_manage_base/ConnectionInformation.h"

using namespace std;

EventBaseHandler::EventBaseHandler(void)
{
}

EventBaseHandler::~EventBaseHandler(void)
{
}

void EventBaseHandler::OnReceiveData(uint64_t cid, std::vector<uint8_t> const &data)
{
    static const string ftag("EventBaseHandler::OnReceiveData() ");

    string strTran;
    string strDebug = "id=";
    strDebug += sof_string::itostr(cid, strTran);
    strDebug += " data=[";
    std::vector<uint8_t>::const_iterator it;
    for (it = data.begin(); it != data.end(); ++it)
    {
        strDebug += *it;
        strDebug += ",";
    }
    strDebug += "]";

    cout << ftag << strDebug << endl;
}

void EventBaseHandler::OnSentData(uint64_t cid, uint64_t byteTransferred)
{
    static const string ftag("EventBaseHandler::OnSentData() ");

    string strTran;
    string strDebug = "id=";
    strDebug += sof_string::itostr(cid, strTran);
    strDebug += ", byteTransferred=";
    strDebug += sof_string::itostr(byteTransferred, strTran);

    cout << ftag << strDebug << endl;
}

void EventBaseHandler::OnClientDisconnect(uint64_t cid, int errorcode)
{
    static const string ftag("EventBaseHandler::OnClientDisconnect() ");

    string strTran;
    string strDebug = "id=";
    strDebug += sof_string::itostr(cid, strTran);
    strDebug += ", error=";
    strDebug += sof_string::itostr(errorcode, strTran);
    strDebug += ", serror=";
    strDebug += strerror(errorcode);

    cout << ftag << strDebug << endl;
}

void EventBaseHandler::OnDisconnect(uint64_t cid, int errorcode)
{
    static const string ftag("EventBaseHandler::OnDisconnect() ");

    string strTran;
    string strDebug = "id=";
    strDebug += sof_string::itostr(cid, strTran);
    strDebug += ", error=";
    strDebug += sof_string::itostr(errorcode, strTran);
    strDebug += ", serror=";
    strDebug += strerror(errorcode);

    cout << ftag << strDebug << endl;
}

void EventBaseHandler::OnServerClose(int errorCode)
{
    // OnServerClose时一般是程序退出时，因为是多线程程序，可能会引用到已经释放的资源
    /*
    static const string ftag("EventBaseHandler::OnServerClose() ");

    string strTran;
    string strDebug = "error=";
    strDebug += sof_string::itostr(errorCode, strTran);
    strDebug += ", serror=";
    strDebug += strerror(errorCode);

    cout << ftag << strDebug<< endl;
    */
}

void EventBaseHandler::OnErrorCode(const std::string &errorMsg, int errorCode)
{
    static const string ftag("EventBaseHandler::OnErrorCode() ");

    string strTran;
    string strDebug = errorMsg;
    strDebug += ", error=";
    strDebug += sof_string::itostr(errorCode, strTran);
    strDebug += ", serror=";
    strDebug += strerror(errorCode);

    cout << ftag << strDebug << endl;
}

void EventBaseHandler::OnErrorStr(const std::string &errorMsg)
{
    static const string ftag("EventBaseHandler::OnErrorStr() ");

    cout << ftag << errorMsg << endl;
}
