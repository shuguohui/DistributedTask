
#include <SDKDDKVer.h>

#include <stdio.h>
#include <tchar.h>
#include <Core/IDisposable.h>
#include <Core/Ptr.h>
#include <Core/Task.h>
#include <Core/ConnectionInfo.h>
#include <Task/Worker.h>
#include <Task/Function.h>
#include <iostream>
#include <vector>
using namespace DT;
bool CallBack(Task* pTask,Worker* pConn,void* worker_context)
{
	std::wcout << L"Get Task,FuncName:" << pTask->GetFunctionName() 
				<< L" NameSpace:" << pTask->GetNamespace() 
				<< L" Level :"<< pTask->GetTaskLevel() << std::endl;

	unsigned int nBufferLen = 0;
	const unsigned char* pszValue = pTask->GetData(nBufferLen);
	std::wstring strValue((const wchar_t*)pszValue,nBufferLen / sizeof(wchar_t));
	std::wcout << L"Get Value:" << strValue << std::endl;
	bool b = pConn->WriteData(pTask->GetNamespace(),strValue,strValue.c_str(),strValue.length() * sizeof(wchar_t));
	if(!b)
	{
		std::cout << "WriteData failed!" << std::endl;
		return false;
	}

	const char* pBuffer = NULL;
	int nLen = 0;
	b = pConn->ReadData(pTask->GetNamespace(),strValue,(const void**)&pBuffer,nLen);
	if(!b || !(nLen == strValue.length() * sizeof(wchar_t)))
	{
		std::cout << "ReadData failed!" << std::endl;
		return false;
	}
	std::wstring str2((const wchar_t* )pBuffer,nLen / sizeof(wchar_t));
	if(_wcsicmp(str2.c_str(),strValue.c_str()))
	{
		std::cout << "ReadData failed!" << std::endl;
		return false;
	}


	//Sleep(100);

	return true;
}

int main()
{
	std::wcout.imbue(std::locale("chs"));

	Ptr<ConnectionInfo> ptrConnInfo = ConnectionInfo::Create();
	ptrConnInfo->SetType(ConnectionInfo::MYSQL);
	ptrConnInfo->SetServer(L"localhost");
	ptrConnInfo->SetPortNumber(3306);
	ptrConnInfo->SetUser(L"root");
	ptrConnInfo->SetPassword(L"666666");
	ptrConnInfo->SetDatabase(L"TASK");
	ptrConnInfo->SetTimeout(60);

	Ptr<Worker> ptrWorker = Worker::Create(ptrConnInfo);
	Ptr<Function> ptrFunction = Function::Create(L"test",&CallBack);
	ptrWorker->RegisterFunction(ptrFunction);
	ptrWorker->SetIdentification(L"my");
	ptrWorker->Run();
	/*bool b = ptrConn->Open(ptrConnInfo);
	if(!b)
	{
		std::cout << "open failed!" << std::endl;
		return 0;
	}
	std::wstring strFunc = L"test";
	while(true)
	{
		Ptr<Task> ptrTask = ptrConn->GetTask(strFunc);
		if(NULL == ptrTask)
		{
			std::cout << "no task,sleep 3s " << std::endl;
			Sleep(3000);
			continue;
		}

		std::wcout << L"Get Task,FuncName:" << strFunc 
					<< L" NameSpace:" << ptrTask->GetNamespace() 
					<< L" Level :"<< ptrTask->GetTaskLevel() << std::endl;

		unsigned int nBufferLen = 0;
		const unsigned char* pszValue = ptrTask->GetData(nBufferLen);
		std::wstring strValue((const wchar_t*)pszValue,nBufferLen / sizeof(wchar_t));
		std::wcout << L"Get Value:" << strValue << std::endl;
		b = ptrConn->WriteData(ptrTask->GetNamespace(),strValue,strValue.c_str(),strValue.length() * sizeof(wchar_t));
		if(!b)
		{
			std::cout << "WriteData failed!" << std::endl;
			continue;
		}

		const char* pBuffer = NULL;
		int nLen = 0;
		b = ptrConn->ReadData(ptrTask->GetNamespace(),strValue,(const void**)&pBuffer,nLen);
		if(!b || !(nLen == strValue.length() * sizeof(wchar_t)))
		{
			std::cout << "ReadData failed!" << std::endl;
			continue;
		}
		std::wstring str2((const wchar_t* )pBuffer,nLen / sizeof(wchar_t));
		if(_wcsicmp(str2.c_str(),strValue.c_str()) == 0)
		{
			std::cout << "ReadData failed!" << std::endl;
			continue;
		}

		ptrConn->FinishTask(ptrTask);
		Sleep(100);
	}*/
	return 0;
}

