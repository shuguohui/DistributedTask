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
	//设置默认字符编码utf8

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
		idx INT NOT NULL DEFAULT 0,\n\
		level INT NOT NULL DEFAULT 1,\n\
		func varchar(255) NOT NULL,\n\
		ns varchar(255) NOT NULL,\n\
		updatetime INT NOT NULL DEFAULT 0,\n\
		timeout INT NOT NULL,\n\
		host varchar(255),\n\
		ip varchar(255),\n\
		createDatetime timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,\n\
		finishDatetime timestamp,\n\
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
	//设置默认字符编码utf8

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

	if(!Ping())
		return NULL;

	//锁定Task表
	//获取任务列表，按照level排序，status = 0的 （for update进行独占锁）
	//获取一个可用的TASK并更新它的状态等信息
	//sql
	const char* pszSQL = "";

	return NULL;
}

void MySQLConnection::FinishTask(const Task* pTask)
{
	if(!m_bOpen)
		return;

	if(!Ping())
		return;

	if(NULL == pTask)
		return;

	const RuntimeTask* pRuntimeTask = dynamic_cast<const RuntimeTask*>(pTask);
	if(NULL == pRuntimeTask)
		return;

	//锁定该条记录
	//检查idx是否一致不一致说明超时等原因被其他人拿走了。
	//更新Task的状态

}

void MySQLConnection::CreateTasks(const std::vector<Ptr<Task> >& tasks)
{
	if(!m_bOpen)
		return;

	if(!Ping())
		return;

	//先获取当前服务器时间
	const char* pszSQL = 
		"insert into TASK(level,func,ns,timeout,host,ip,data) values(?,?,?,?,?,?,?)";
}

void MySQLConnection::GetTaskStatus(const std::wstring& strFunction
	, const std::wstring& strNameSpace
	, int& nFinished
	, int& nTotal)
{
	if(!m_bOpen)
		return;

	if(!Ping())
		return;

	//nFinished
	const char* pszSQL = 
		"SELECT count(*) from TASK where func = ? and ns = ? and status = 1";

	//nTotal
	pszSQL = 
		"SELECT count(*) from TASK where func = ? and ns = ?";

}

void MySQLConnection::DeleteTasks(const std::wstring& strFunction
	, const std::wstring& strNameSpace)
{
	if(!m_bOpen)
		return;

	if(!Ping())
		return;

	//
	std::string strTable = Unicode2Utf8(strNameSpace);
	const char* pszSQL = 
		"SELECT count(*) from TASK where func = ? and ns = ? and status = 0";

	//查询如果有记录就不能删除

	pszSQL = "DELETE FROM TASK WHERE func = ? and ns = ?";
}

bool MySQLConnection::CreateDataNameSpace(const std::wstring& strNameSpace)
{
	if(!m_bOpen)
		return false;

	if(!Ping())
		return false;

	std::string strDatabase = Unicode2Utf8(m_connInfo->GetDatabase());
	std::string strTable = Unicode2Utf8(strNameSpace);
	MYSQL* mysql = (MYSQL*)m_mysql;

	int errorid = mysql_query(mysql,"use information_schema");
	if(errorid)
	{
		return false;
	}

	char sql[1024];
	sprintf(sql
			,"select TABLE_NAME from INFORMATION_SCHEMA.TABLES where TABLE_SCHEMA='%s' and TABLE_NAME='%s'"
			, strDatabase.c_str()
			, strTable.c_str());
	errorid = mysql_real_query(mysql,sql,strlen(sql));
	if(errorid)
	{
		return false;
	}
	bool bHasSchema = false;
	MYSQL_RES* r = mysql_store_result(mysql);
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
		return false;
	}

	sprintf(sql,"USE `%s`",strDatabase.c_str());
	errorid = mysql_real_query(mysql,sql,strlen(sql));
	if(errorid)
	{
		return false;
	}

	sprintf(sql
		,"CREATE TABLE `%s` \n\
		(`key` varchar(255) NOT NULL,\n\
		`value` MEDIUMBLOB ,\n\
		PRIMARY KEY(`key`)\n\
		) ENGINE=INNODB DEFAULT CHARSET=UTF8;"
		, strTable.c_str());
	errorid = mysql_real_query(mysql,sql,strlen(sql));
	if(errorid)
	{
		return false;
	}
	return true;
}

bool MySQLConnection::DeleteDataNameSpace(const std::wstring& strNameSpace)
{
	if(!m_bOpen)
		return false;
	if(!Ping())
		return false;

	std::string strTable = Unicode2Utf8(strNameSpace);
	MYSQL* mysql = (MYSQL*)m_mysql;

	char sql[1024];
	sprintf(sql,"DROP TABLE IF EXISTS `%s`",strTable.c_str());
	int errorid = mysql_real_query(mysql,sql,strlen(sql));
	if(errorid)
	{
		return false;
	}
	return true;
}

const void* MySQLConnection::ReadData(const std::wstring& strNameSpace
										, const std::wstring& strKey
										, int& nBufferLen)
{
	if(!m_bOpen)
		return NULL;

	if(!Ping())
		return NULL;

	//sql
	std::string strTable = Unicode2Utf8(strNameSpace);
	char sql[1024];
	sprintf(sql,"SELECT `value` from `%s` where `key` = ?",strTable.c_str());
}

void MySQLConnection::WriteData(const std::wstring& strNameSpace
								, const std::wstring& strKey
								, const void* pBuffer
								, int nBufferLen)
{
	if(!m_bOpen)
		return;

	if(!Ping())
		return;
	//sql
	std::string strTable = Unicode2Utf8(strNameSpace);
	char sql[1024];
	sprintf(sql
			, "INSERT `%s`(`key`,`value`) values (?,?) ON DUPLICATE KEY UPDATE `value` = ?"
			, strTable.c_str());
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
	m_connInfo = pConnInfo->Clone();
	return true;
}

void MySQLConnection::Close()
{
	if(!m_bOpen)
		return;
	Ping();

	MYSQL* mysql = (MYSQL*)m_mysql;
	mysql_close(mysql);

	m_bOpen = false;
	free(m_mysql);
	m_mysql = NULL;

}

bool MySQLConnection::Ping()
{
	if(!m_bOpen)
		return false;
	int n = mysql_ping((MYSQL*)m_mysql);
	return n == 0;
}