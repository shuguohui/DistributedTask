#include "Manager.h"
#include <Provider/IProvider/IConnection.h>
#include <Provider/MySQL/MySQLConnection.h>
Manager::Manager()
{

}

Manager::~Manager()
{

}

Manager* Manager::Create(const ConnectionInfo* pConn)
{
	Manager* pManager = new Manager;
	pManager->m_conn = pConn->Clone();
	return pManager;
}

bool Manager::CreateRepository()
{
	Ptr<MySQLConnection> ptrConn = MySQLConnection::Create();
	return ptrConn->CreateRepository(m_conn);
}

bool Manager::DropRepository()
{
	Ptr<IConnection> ptrConn = MySQLConnection::Create();
	return ptrConn->DropRepository(m_conn);
}