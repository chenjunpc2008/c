#include "IocpEventHandler.h"

#include <iostream>

#include "tool_string/sof_string.h"

#include "socket_conn_manage_base/ConnectionInformation.h"

using namespace std;

IocpEventHandler::IocpEventHandler(void)
    : m_pServiceOwner(nullptr)
{
}

IocpEventHandler::~IocpEventHandler(void)
{
}

void IocpEventHandler::OnServerError(int errorCode)
{
    static const string ftag("IocpEventHandler::OnServerError() ");

    string strTran;
    string sDebug = "err=";
    sDebug += sof_string::itostr(errorCode, strTran);

    cout << ftag << sDebug << endl;
}

void IocpEventHandler::OnNewConnection(uint64_t cid, struct ConnectionInformation const &ci)
{
    static const string ftag("IocpEventHandler::OnNewConnection() ");

    string strTran;
    string sDebug = "new id=";
    sDebug += sof_string::itostr(cid, strTran);

    cout << ftag << sDebug << endl;
}

void IocpEventHandler::OnReceiveData(uint64_t cid, std::vector<uint8_t> const &data)
{
    static const string ftag("IocpEventHandler::OnReceiveData() ");

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

    cout << ftag << sDebug << endl;
}

void IocpEventHandler::OnSentData(uint64_t cid, uint64_t byteTransferred)
{
    static const string ftag("IocpEventHandler::OnSentData() ");

    string strTran;
    string sDebug = "id=";
    sDebug += sof_string::itostr(cid, strTran);

    cout << ftag << sDebug << endl;
}

void IocpEventHandler::OnClientDisconnect(uint64_t cid, int errorcode)
{
    static const string ftag("IocpEventHandler::OnClientDisconnect() ");

    string strTran;
    string sDebug = "id=";
    sDebug += sof_string::itostr(cid, strTran);

    cout << ftag << sDebug << endl;

    try
    {
        GetServiceOwner().Shutdown(cid, SD_SEND);

        GetServiceOwner().Disconnect(cid);
    }
    catch (exception const &e)
    {
        cout << ftag << "exception " << e.what() << endl;

        return;
    }
}

void IocpEventHandler::OnDisconnect(uint64_t cid, int errorcode)
{
    static const string ftag("IocpEventHandler::OnDisconnect() ");

    string strTran;
    string sDebug = "id=";
    sDebug += sof_string::itostr(cid, strTran);
    sDebug += " errorcode=";
    sDebug += sof_string::itostr(errorcode, strTran);

    cout << ftag << sDebug << endl;
}

void IocpEventHandler::OnServerClose(int errorCode)
{
    static const string ftag("IocpEventHandler::OnServerClose() ");

    string strTran;
    string sDebug = " errorcode=";
    sDebug += sof_string::itostr(errorCode, strTran);

    cout << ftag << sDebug << endl;
}