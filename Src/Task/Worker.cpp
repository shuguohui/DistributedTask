#include "Worker.h"
#include <Provider/MySQL/MySQLConnection.h>
#include <iostream>
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
		Ptr<Task> ptrTask = m_conn->GetTask(L"");
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
			if(iter->second->GetCallBack()(ptrTask,m_conn,NULL))
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