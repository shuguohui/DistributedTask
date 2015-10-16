#include "Worker.h"
#include <Provider/MySQL/MySQLConnection.h>
#include <iostream>
NS_TASK
Worker::Worker()
	:m_bRunning(false)
{

}

Worker::~Worker()
{

}

Worker* Worker::Create(const ConnectionInfo* pConn)
{
	Ptr<IConnection> ptrConn = MySQLConnection::Create();
	bool b = ptrConn->Open(pConn);
	if(!b)
	{
		std::cout << "Open Failed" << std::endl;
		return NULL;
	}


	Worker* pWorker = new Worker;
	pWorker->m_connInfo = pConn->Clone();
	pWorker->m_conn = ptrConn;
	return pWorker;
}

void Worker::RegisterFunction(Function* pFunc)
{
	if(NULL == pFunc ||pFunc->GetFunctionName().empty())
		return;
	if(m_regiterFuncs.find(pFunc->GetFunctionName()) != m_regiterFuncs.end())
		return;
	m_regiterFuncs[pFunc->GetFunctionName()] = pFunc;
}
void Worker::UnRegisterFunction(const std::wstring& strFuncName)
{
	std::map<std::wstring,Ptr<Function> >::iterator iter = m_regiterFuncs.find(strFuncName);
	if(iter != m_regiterFuncs.end())
		m_regiterFuncs.erase(iter);
}

void Worker::Run()
{
	m_bRunning = true;
	while(m_bRunning)
	{
		std::vector<std::wstring> funcNames;
		for (std::map<std::wstring,Ptr<Function> >::const_iterator citer = m_regiterFuncs.begin();
			citer != m_regiterFuncs.end();
			++citer)
		{
			funcNames.push_back(citer->second->GetFunctionName());
		}
		Ptr<Task> ptrTask = m_conn->GetTask(funcNames);
		if(NULL == ptrTask)
		{
			std::cout << "no task,sleep 3s " << std::endl;
			Sleep(3000);
			continue;
		}
		std::map<std::wstring,Ptr<Function> >::iterator iter = 
			m_regiterFuncs.find(ptrTask->GetFunctionName());
		if(iter == m_regiterFuncs.end())
		{
			m_conn->Abort(ptrTask);
		}
		else
		{
			if(iter->second->GetCallBack()(ptrTask,this,NULL))
			{
				m_conn->FinishTask(ptrTask);
			}
			else
			{
				m_conn->Abort(ptrTask);
			}
		}
	}
}

void Worker::Stop()
{
	m_bRunning = false;
}

bool  Worker::ReadData(const std::wstring& strNameSpace
						, const std::wstring& strKey
						, const void** pBuffer
						, int& nBufferLen)
{
	return m_conn->ReadData(strNameSpace,strKey,pBuffer,nBufferLen);
}

bool Worker::WriteData(const std::wstring& strNameSpace
						, const std::wstring& strKey
						, const void* pBuffer
						, int nBufferLen)
{
	return m_conn->WriteData(strNameSpace,strKey,pBuffer,nBufferLen);
}
NS_END