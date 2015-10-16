
#include <SDKDDKVer.h>

#include <stdio.h>
#include <tchar.h>
#include <Core/IDisposable.h>
#include <Core/Ptr.h>
#include <Core/Task.h>
#include <Core/ConnectionInfo.h>

#include <Provider/MySQL/MySQLConnection.h>
#include <iostream>
#include <vector>
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
	bool b = ptrConn->Open(ptrConnInfo);
	if(!b)
	{
		std::cout << "open failed!" << std::endl;
		return 0;
	}

	std::vector<std::wstring> ns = ptrConn->GetNameSpaces();
	bool bHasNS = false;
	std::wstring strNS = L"�찲��";
	std::wstring strFunc = L"test";
	for (std::vector<std::wstring>::const_iterator citer = ns.begin();
		citer != ns.end();
		++citer)
	{
		if(_wcsicmp(citer->c_str(),strNS.c_str()) == 0)
		{
			bHasNS = true;
			break;
		}
	}
	if(!bHasNS)
	{
		b = ptrConn->CreateNameSpace(strNS);
		if(!b)
		{
			std::cout << "CreateNameSpace failed!" << std::endl;
			return 0;
		}
	}
	
	//��������
	std::vector<Ptr<Task> > tasks;
	wchar_t sz[1024];
	for (int i = 0;i < 10000;i++)
	{
		swprintf(sz,L"TASK_%d",i);
		Ptr<Task> task = Task::Create();
		task->SetFunctionName(strFunc);
		task->SetData((unsigned char*)sz,wcslen(sz) * sizeof(wchar_t));
		if((i % 100) == 0)
			task->SetTaskLevel(2);
		else if((i % 76) == 0)
			task->SetTaskLevel(1);
		tasks.push_back(task);
	}

	ptrConn->CreateTasks(tasks,strNS);


	//��ʼ�������������
	while(true)
	{
		int nFinished;
		int nTotal;
		ptrConn->GetTaskStatus(strFunc,strNS,nFinished,nTotal);
		std::cout << "Task Status: " << nFinished << " / " << nTotal << std::endl;
		Sleep(5000);
	}
	return 0;
}

