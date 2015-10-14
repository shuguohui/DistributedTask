
#include <SDKDDKVer.h>

#include <stdio.h>
#include <tchar.h>
#include <Core/Ptr.h>
#include <Core/Task.h>
#include <Core/ConnectionInfo.h>
#include <Provider/MySQL/MySQLConnection.h>
int main()
{
	Ptr<ConnectionInfo> ptrConnInfo = ConnectionInfo::Create();
	ptrConnInfo->SetType(ConnectionInfo::MYSQL);
	ptrConnInfo->SetServer(L"localhost");
	ptrConnInfo->SetPortNumber(3306);
	ptrConnInfo->SetUser(L"root");
	ptrConnInfo->SetPassword(L"666666");
	ptrConnInfo->SetDatabase(L"TASK");
	ptrConnInfo->SetTimeout(60);

	Ptr<MySQLConnection> ptrConn = MySQLConnection::Create();
	bool b = ptrConn->DropRepository(ptrConnInfo);
	b = ptrConn->CreateRepository(ptrConnInfo);

	b = ptrConn->Open(ptrConnInfo);
	b = ptrConn->CreateDataNameSpace(L"蜀国会");
	b = ptrConn->DeleteDataNameSpace(L"蜀国会");
	b = ptrConn->DropRepository(ptrConnInfo);
	return 0;
}

