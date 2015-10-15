
#include <SDKDDKVer.h>

#include <stdio.h>
#include <tchar.h>
#include <Core/IDisposable.h>
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
	b = ptrConn->CreateNameSpace(L"test");
	b = ptrConn->CreateNameSpace(L"test1");
	std::string str = "shuguohui";

	std::vector<Ptr<Task> > tasks;
	Ptr<Task> task = Task::Create();
	task->SetFunctionName(_T("test"));
	task->SetNamespace(_T("test"));
	task->SetData((unsigned char*)str.c_str(),str.length());
	tasks.push_back(task);

	str = "shuguohui1";
	task = Task::Create();
	task->SetFunctionName(_T("test"));
	task->SetNamespace(_T("test1"));
	task->SetData((unsigned char*)str.c_str(),str.length());
	tasks.push_back(task);

	ptrConn->CreateTasks(tasks,_T("test"));

	Ptr<Task> ptrTask = ptrConn->GetTask(L"test");
	ptrConn->FinishTask(ptrTask);

	int nFinish;
	int nTotal;
	ptrConn->GetTaskStatus(L"test",L"test",nFinish,nTotal);

	ptrConn->DeleteTasks(_T("test"),_T("test"));
	
	b = ptrConn->WriteData(L"蜀国会",L"shuguohui",str.c_str(),str.length());

	const char* pBuffer = NULL;
	int nLen = 0;
	b = ptrConn->ReadData(L"蜀国会",L"shuguohui",(const void**)&pBuffer,nLen);
	b = nLen == str.length();
	std::string str1(pBuffer,nLen);

	str = "shuguohui2";
	b = ptrConn->WriteData(L"蜀国会",L"shuguohui",str.c_str(),str.length());

	pBuffer = NULL;
	nLen = 0;
	b = ptrConn->ReadData(L"蜀国会",L"shuguohui",(const void**)&pBuffer,nLen);
	b = nLen == str.length();
	std::string str2(pBuffer,nLen);

	b = ptrConn->DeleteNameSpace(L"蜀国会");
	//b = ptrConn->DropRepository(ptrConnInfo);
	return 0;
}

