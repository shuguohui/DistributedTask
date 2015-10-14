#include "MySQLConnection.h"
#include <Core/utf8_.h>
#include <my_global.h>
#include <mysql.h>
#include <errmsg.h>
MySQLConnection::MySQLConnection()
	:m_mysql(NULL)
{

}

MySQLConnection::~MySQLConnection()
{
	Close();
}

MySQLConnection* MySQLConnection::Create(const ConnectionInfo* pConnInfo)
{
	MySQLConnection* pConn =  new MySQLConnection;
	if(!pConn->Open(pConnInfo))
	{
		delete pConn;
		return NULL;
	}
	return pConn;
}

bool MySQLConnection::CreateRepository()
{
	return true;
}

bool MySQLConnection::DropRepository()
{
	return true;
}

Task* MySQLConnection::GetTask(const std::wstring& strFunctionName)
{
	return NULL;
}

void MySQLConnection::FinishTask(const Task* pTask)
{

}

void MySQLConnection::CreateTasks(const std::vector<Ptr<Task> >& tasks)
{

}

void MySQLConnection::GetTaskStatus(const std::wstring& strFunction
	, const std::wstring& strNameSpace
	, int& nFinished
	, int& nTotal)
{

}

void MySQLConnection::DeleteTasks(const std::wstring& strFunction
	, const std::wstring& strNameSpace)
{

}

bool MySQLConnection::Open(const ConnectionInfo* pConnInfo)
{
	if(NULL == pConnInfo || pConnInfo->GetType() != ConnectionInfo::MYSQL)
		return false;
	m_mysql = malloc(sizeof(MYSQL));
	MYSQL* mysql = (MYSQL*)m_mysql;
	mysql_init(mysql);
	my_bool reconnect = 1;
	mysql_options(mysql,MYSQL_OPT_RECONNECT,&reconnect);
	
	my_bool report_data_truncation = 0;
	mysql_options(mysql,MYSQL_REPORT_DATA_TRUNCATION,&report_data_truncation);

	mysql_options(mysql,MYSQL_SET_CHARSET_NAME,"utf8");

	int nTimeout = pConnInfo->GetTimeout();
	if(nTimeout <= 0)
		nTimeout = 30;
	mysql_options(mysql,MYSQL_OPT_CONNECT_TIMEOUT,&nTimeout);


	std::string strServer = Unicode2Utf8(pConnInfo->GetServer());
	std::string strUserName = Unicode2Utf8(pConnInfo->GetUser());
	std::string strPwd = Unicode2Utf8(pConnInfo->GetPassword());
	std::string strDatabase = Unicode2Utf8(pConnInfo->GetDatabase());
	int nPort = pConnInfo->GetPortNumber();
	if(!mysql_real_connect(mysql,strServer.c_str()
							, strUserName.c_str()
							, strPwd.c_str()
							, strDatabase.empty() ? NULL : strDatabase.c_str()
							, nPort
							, NULL
							, CLIENT_MULTI_STATEMENTS))
	{
		return false;
	}
	return true;
}

void MySQLConnection::Close()
{

}