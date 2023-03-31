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
    string strDebug = "err=";
    strDebug += sof_string::itostr(errorCode, strTran);

    cout << ftag << strDebug << endl;
}

void IocpEventHandler::OnNewConnection(uint64_t cid, struct ConnectionInformation const &ci)
{
    static const string ftag("IocpEventHandler::OnNewConnection() ");

    string strTran;
    string strDebug = "new id=";
    strDebug += sof_string::itostr(cid, strTran);

    cout << ftag << strDebug << endl;
}

void IocpEventHandler::OnReceiveData(uint64_t cid, std::vector<uint8_t> const &data)
{
    static const string ftag("IocpEventHandler::OnReceiveData() ");

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

void IocpEventHandler::OnSentData(uint64_t cid, uint64_t byteTransferred)
{
    static const string ftag("IocpEventHandler::OnSentData() ");

    string strTran;
    string strDebug = "id=";
    strDebug += sof_string::itostr(cid, strTran);

    cout << ftag << strDebug << endl;
}

void IocpEventHandler::OnClientDisconnect(uint64_t cid, int errorcode)
{
    static const string ftag("IocpEventHandler::OnClientDisconnect() ");

    string strTran;
    string strDebug = "id=";
    strDebug += sof_string::itostr(cid, strTran);

    cout << ftag << strDebug << endl;

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
    string strDebug = "id=";
    strDebug += sof_string::itostr(cid, strTran);
    strDebug += " errorcode=";
    strDebug += sof_string::itostr(errorcode, strTran);

    cout << ftag << strDebug << endl;
}

void IocpEventHandler::OnServerClose(int errorCode)
{
    static const string ftag("IocpEventHandler::OnServerClose() ");

    string strTran;
    string strDebug = " errorcode=";
    strDebug += sof_string::itostr(errorCode, strTran);

    cout << ftag << strDebug << endl;
}