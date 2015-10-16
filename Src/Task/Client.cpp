#include "Client.h"
#include <Provider/MySQL/MySQLConnection.h>
#include <iostream>
NS_TASK
Client::Client()
{

}

Client::~Client()
{

}

Client* Client::Create(const ConnectionInfo* pConn)
{
	Ptr<IConnection> ptrConn = MySQLConnection::Create();
	bool b = ptrConn->Open(pConn);
	if(!b)
	{
		std::cout << "Open Failed" << std::endl;
		return NULL;
	}

	
	Client* pClient = new Client;
	pClient->m_connInfo = pConn->Clone();
	pClient->m_conn = ptrConn;
	return pClient;
}

void Client::CreateTasks(const std::vector<Ptr<Task> >& tasks
						, const std::wstring& strNameSapce)
{
	m_conn->CreateTasks(tasks,strNameSapce);
}

void Client::GetTaskStatus(const std::wstring& strFunction
							, const std::wstring& strNameSpace
							, int& nFinished
							, int& nTotal)
{
	m_conn->GetTaskStatus(strFunction,strNameSpace,nFinished,nTotal);
}


void Client::DeleteTasks(const std::wstring& strFunction
						, const std::wstring& strNameSpace)
{
	m_conn->DeleteTasks(strFunction,strNameSpace);
}

bool Client::CreateNameSpace(const std::wstring& strNameSpace)
{
	return m_conn->CreateNameSpace(strNameSpace);
}

bool Client::DeleteNameSpace(const std::wstring& strNameSpace)
{
	return m_conn->DeleteNameSpace(strNameSpace);
}

std::vector<std::wstring> Client::GetNameSpaces()
{
	return m_conn->GetNameSpaces();
}
NS_END