#include "MySQLConnection.h"
#include <Core/RuntimeTask.h>
#include <Core/utf8_.h>
#include <my_global.h>
#include <mysql.h>
#include <errmsg.h>
MySQLConnection::MySQLConnection()
	:m_mysql(NULL)
	, m_bOpen(false)
{

}

MySQLConnection::~MySQLConnection()
{
	Close();
}

MySQLConnection* MySQLConnection::Create()
{
	return new MySQLConnection;
}

bool MySQLConnection::CreateRepository(const ConnectionInfo* pConn)
{
	if(NULL == pConn || pConn->GetDatabase().empty() || pConn->GetType() != ConnectionInfo::MYSQL)
		return false;

	MYSQL mysql;
	mysql_init(&mysql);
	my_bool reconnect = 1;
	mysql_options(&mysql,MYSQL_OPT_RECONNECT,&reconnect);

	my_bool report_data_truncation = 0;
	mysql_options(&mysql,MYSQL_REPORT_DATA_TRUNCATION,&report_data_truncation);

	mysql_options(&mysql,MYSQL_SET_CHARSET_NAME,"utf8");

	int nTimeout = pConn->GetTimeout();
	if(nTimeout <= 0)
		nTimeout = 30;
	mysql_options(&mysql,MYSQL_OPT_CONNECT_TIMEOUT,&nTimeout);


	std::string strServer = Unicode2Utf8(pConn->GetServer());
	std::string strUserName = Unicode2Utf8(pConn->GetUser());
	std::string strPwd = Unicode2Utf8(pConn->GetPassword());
	std::string strDatabase = Unicode2Utf8(pConn->GetDatabase());
	int nPort = pConn->GetPortNumber();
	if(!mysql_real_connect(&mysql
							,strServer.c_str()
							, strUserName.c_str()
							, strPwd.c_str()
							, NULL
							, nPort
							, NULL
							, CLIENT_MULTI_STATEMENTS))
	{
		return false;
	}

	int errorid;
	//ÉèÖÃÄ¬ÈÏ×Ö·û±àÂëutf8

	errorid = mysql_query(&mysql,"SET NAMES utf8");
	if(errorid)
	{
		mysql_close(&mysql);
		return false;
	}

	errorid = mysql_query(&mysql,"use information_schema");
	if(errorid)
	{
		mysql_close(&mysql);
		return false;
	}

	char sql[1024];
	sprintf(sql,"select SCHEMA_NAME from information_schema.SCHEMATA where schema_name='%s'",strDatabase.c_str());
	errorid = mysql_real_query(&mysql,sql,strlen(sql));
	if(errorid)
	{
		mysql_close(&mysql);
		return false;
	}
	bool bHasSchema = false;
	MYSQL_RES* r = mysql_store_result(&mysql);
	if(0 == mysql_num_rows(r))
	{
		bHasSchema =false;
	}
	else
	{
		bHasSchema = true;
	}
	mysql_free_result(r);
	if(bHasSchema)
	{
		mysql_close(&mysql);
		return false;
	}

	sprintf(sql,"Create database `%s` default character set utf8",strDatabase.c_str());
	errorid = mysql_real_query(&mysql,sql,strlen(sql));
	if(errorid)
	{
		mysql_close(&mysql);
		return false;
	}

	sprintf(sql,"USE `%s`",strDatabase.c_str());
	errorid = mysql_real_query(&mysql,sql,strlen(sql));
	if(errorid)
	{
		mysql_close(&mysql);
		return false;
	}

	const char* pszTableSchema = 
		"CREATE TABLE TASK(id INT NOT NULL AUTO_INCREMENT,\n\
		idx INT NOT NULL,\n\
		level INT NOT NULL DEFAULT 1,\n\
		func varchar(255) NOT NULL,\n\
		ns varchar(255) NOT NULL,\n\
		updatetime INT NOT NULL,\n\
		timeout INT NOT NULL,\n\
		host varchar(255),\n\
		ip varchar(255),\n\
		status INT NOT NULL DEFAULT 0,\n\
		data MEDIUMBLOB ,\n\
		PRIMARY KEY(id),\n\
		KEY TASK_LEVEL(level) ,\n\
		KEY TASK_FUNC(func) ,\n\
		KEY TASK_NS(ns) ,\n\
		KEY TASK_STATUS(status) \n\
		) ENGINE=INNODB DEFAULT CHARSET=UTF8;";
	errorid = mysql_real_query(&mysql,pszTableSchema,strlen(pszTableSchema));
	if(errorid)
	{
		mysql_close(&mysql);
		return false;
	}

	mysql_close(&mysql);
	return true;
}

bool MySQLConnection::DropRepository(const ConnectionInfo* pConn)
{
	if(NULL == pConn || pConn->GetDatabase().empty() || pConn->GetType() != ConnectionInfo::MYSQL)
		return false;

	MYSQL mysql;
	mysql_init(&mysql);
	my_bool reconnect = 1;
	mysql_options(&mysql,MYSQL_OPT_RECONNECT,&reconnect);

	my_bool report_data_truncation = 0;
	mysql_options(&mysql,MYSQL_REPORT_DATA_TRUNCATION,&report_data_truncation);

	mysql_options(&mysql,MYSQL_SET_CHARSET_NAME,"utf8");

	int nTimeout = pConn->GetTimeout();
	if(nTimeout <= 0)
		nTimeout = 30;
	mysql_options(&mysql,MYSQL_OPT_CONNECT_TIMEOUT,&nTimeout);


	std::string strServer = Unicode2Utf8(pConn->GetServer());
	std::string strUserName = Unicode2Utf8(pConn->GetUser());
	std::string strPwd = Unicode2Utf8(pConn->GetPassword());
	std::string strDatabase = Unicode2Utf8(pConn->GetDatabase());
	int nPort = pConn->GetPortNumber();
	if(!mysql_real_connect(&mysql
							,strServer.c_str()
							, strUserName.c_str()
							, strPwd.c_str()
							, NULL
							, nPort
							, NULL
							, CLIENT_MULTI_STATEMENTS))
	{
		return false;
	}

	int errorid;
	//ÉèÖÃÄ¬ÈÏ×Ö·û±àÂëutf8

	errorid = mysql_query(&mysql,"USE MYSQL");
	if(errorid)
	{
		mysql_close(&mysql);
		return false;
	}

	char sql[1024];
	sprintf(sql,"DROP DATABASE IF EXISTS `%s`",strDatabase.c_str());
	errorid = mysql_real_query(&mysql,sql,strlen(sql));
	if(errorid)
	{
		mysql_close(&mysql);
		return false;
	}

	mysql_close(&mysql);
	return true;
}

Task* MySQLConnection::GetTask(const std::wstring& strFunctionName)
{
	if(!m_bOpen)
		return NULL;
	return NULL;
}

void MySQLConnection::FinishTask(const Task* pTask)
{
	if(NULL == pTask)
		return;
	const RuntimeTask* pRuntimeTask = dynamic_cast<const RuntimeTask*>(pTask);
	if(NULL == pRuntimeTask)
		return;

	if(!m_bOpen)
		return;
}

void MySQLConnection::CreateTasks(const std::vector<Ptr<Task> >& tasks)
{
	if(!m_bOpen)
		return;
}

void MySQLConnection::GetTaskStatus(const std::wstring& strFunction
	, const std::wstring& strNameSpace
	, int& nFinished
	, int& nTotal)
{
	if(!m_bOpen)
		return;
}

void MySQLConnection::DeleteTasks(const std::wstring& strFunction
	, const std::wstring& strNameSpace)
{
	if(!m_bOpen)
		return;
}

void MySQLConnection::CreateDataNameSpace(const std::wstring& strNameSpace)
{

}

void MySQLConnection::DeleteDataNameSpace(const std::wstring& strNameSpace)
{

}

const void* MySQLConnection::ReadData(const std::wstring& strNameSpace
										, int& nBufferLen)
{
	return NULL;
}

void MySQLConnection::WriteData(const std::wstring& strNameSpace
								, const void* pBuffer
								, int nBufferLen)
{

}

bool MySQLConnection::Open(const ConnectionInfo* pConnInfo)
{
	if(m_bOpen)
		return true;

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
							, strDatabase.c_str()
							, nPort
							, NULL
							, CLIENT_MULTI_STATEMENTS))
	{
		return false;
	}

	m_bOpen = true;
	return true;
}

void MySQLConnection::Close()
{

}