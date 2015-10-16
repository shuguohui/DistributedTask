#include "MySQLConnection.h"
#include "MySQLPreparedStatement.h"
#include "TransactionGurad.h"
#include <Core/RuntimeTask.h>
#include <Core/utf8_.h>
#include <vector>
#include <sstream>
#include <process.h>
#include <my_global.h>
#include <mysql.h>
#include <errmsg.h>
NS_TASK
MySQLConnection::MySQLConnection()
	:m_mysql(NULL)
	, m_bOpen(false)
	, m_IsTranStarted(false)
	, m_isolation(ReadCommitted)
	, m_tempBuffer(NULL)
	, m_tempBufferLen(0)
{

}

MySQLConnection::~MySQLConnection()
{
	Close();
}
struct tm MySQLConnection::GetLocalTime()
{
	struct tm nowTime;
	time_t timep;  
	struct tm *p;  

	time(&timep);
	p = gmtime(&timep);
	nowTime = *p;
	return nowTime;
}

struct tm MySQLConnection::GetDatabaseTime()
{
	
	//如果没连接就取本机时间
	if(!m_bOpen || !Ping())
	{
		return GetLocalTime();
	}
	else
	{
		//取服务器时间
		const char* pszSQL = "select now()";
		std::vector<MySQLPreparedStatement::Field> fields;
		Ptr<MySQLPreparedStatement>  ptrStatement = 
			MySQLPreparedStatement::Create(this,pszSQL,fields);
		if(NULL == ptrStatement)
			return GetLocalTime();

		if(!ptrStatement->Excute())
			return GetLocalTime();
		if(!ptrStatement->Fetch())
			return GetLocalTime();

		MYSQL_TIME my_time = ptrStatement->GetDateTime(0);
		tm tm_;  
		
		tm_.tm_year  = my_time.year-1900;  
		tm_.tm_mon   = my_time.month-1;  
		tm_.tm_mday  = my_time.day;  
		tm_.tm_hour  = my_time.hour;  
		tm_.tm_min   = my_time.minute;  
		tm_.tm_sec   = my_time.second;  
		tm_.tm_isdst = 0;
		return tm_;
	}
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
		pid INT,\n\
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

	pszTableSchema = "CREATE TABLE `LOCK`(id INT NOT NULL,\n\
					 PRIMARY KEY(id)) ENGINE=INNODB DEFAULT CHARSET=UTF8;";
	errorid = mysql_real_query(&mysql,pszTableSchema,strlen(pszTableSchema));
	if(errorid)
	{
		mysql_close(&mysql);
		return false;
	}
	errorid = mysql_query(&mysql,"insert into `LOCK` (id) values(1)");
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

bool MySQLConnection::Lock()
{
	const char* pszSQL = "select * from `LOCK` where id=1 for update";
	std::vector<MySQLPreparedStatement::Field> fields;
	Ptr<MySQLPreparedStatement>  ptrStatement = 
		MySQLPreparedStatement::Create(this,pszSQL,fields);
	if(NULL == ptrStatement)
		return false;

	if(!ptrStatement->Excute())
		return false;
	ptrStatement = NULL;
	return true;
}

Task* MySQLConnection::GetTask(const std::vector<std::wstring>& funcNames)
{
	if(funcNames.empty())
		return NULL;
	if(!m_bOpen)
		return NULL;

	if(!Ping())
		return NULL;


	//获取当前时间
	struct tm nowTime = GetDatabaseTime();
	int nowTimeSecond = mktime(&nowTime);

	TransactionGurad transHelper(this);
	if(!Lock())
		return NULL;

	//获取任务列表，按照level排序，status = 0的 （for update进行独占锁）
	//获取一个可用的TASK并更新它的状态等信息
	//sql
	int num_func = funcNames.size();
	std::stringstream ss;
	ss << "?";
	for (int i = 1; i < num_func;i++ )
	{
		ss << ",?";
	}
	char szSQL[1024];
	sprintf(szSQL
			, "select `id`,`idx`,`level`,`func`,`ns`,`updatetime`,`timeout`,`data` FROM `TASK` where `func` in (%s) and `status` = 0 order by `level` desc"
			, ss.str().c_str());
	std::vector<MySQLPreparedStatement::Field> fields;
	for (int i = 0;i < num_func;i++)
	{
		fields.push_back(MySQLPreparedStatement::Field(MYSQL_TYPE_VARCHAR,255));
	}
	
	Ptr<MySQLPreparedStatement> ptrStatement = 
		MySQLPreparedStatement::Create(this,szSQL,fields);
	if(NULL == ptrStatement)
		return NULL;
	for (int i = 0;i < num_func;i++)
	{
		ptrStatement->SetString(i,Unicode2Utf8(funcNames[i]).c_str());
	}
	

	if(!ptrStatement->Excute())
		return NULL;
	RuntimeTask* pTask = NULL;
	while(ptrStatement->Fetch())
	{
		int id = ptrStatement->GetInt(0);
		int idx = ptrStatement->GetInt(1);
		int level = ptrStatement->GetInt(2);
		const char* pszFunc = ptrStatement->GetString(3);
		const char* pszNS = ptrStatement->GetString(4);
		int updatetime = ptrStatement->GetInt(5);
		int timeout = ptrStatement->GetInt(6);
		const void* pBuffer = NULL;
		unsigned int nBufferLen = 0;
		ptrStatement->GetValue(7,&pBuffer,&nBufferLen);
		if(updatetime + timeout > nowTimeSecond)
			continue;
		pTask = RuntimeTask::Create();
		pTask->SetTaskID(id);
		pTask->SetIDX(++idx);
		pTask->SetFunctionName(Utf82Unicode(pszFunc));
		pTask->SetNamespace(Utf82Unicode(pszNS));
		pTask->SetTaskLevel(level);
		pTask->SetTimeOut(timeout);
		pTask->SetUpdateTime(nowTimeSecond);
		pTask->SetData((const unsigned char*)pBuffer,nBufferLen);
		break;
	}
	ptrStatement = NULL;
	if(pTask)
	{
		char pszNameBuffer[128] = {0};  
		gethostname(pszNameBuffer,128);
		//获得本地IP地址  
		struct hostent* pHost;  
		pHost=gethostbyname(pszNameBuffer);//pHost返回的是指向主机的列表  
		std::string strIP;
		for (int i=0;pHost!=NULL&&pHost->h_addr_list[i]!=NULL;i++)  
		{  
			LPCSTR psz = inet_ntoa(*(struct in_addr *)pHost->h_addr_list[i]);//得到指向ip的psz变量  
			strIP += psz;  
			break;
		}  

		const char*  pszSQL = "UPDATE `TASK` SET `idx` = ?,`updatetime`=?,`host`=?,`ip`=?,`pid`=? where id = ?";
		fields.clear();
		fields.push_back(MySQLPreparedStatement::Field(MYSQL_TYPE_LONG,sizeof(int)));
		fields.push_back(MySQLPreparedStatement::Field(MYSQL_TYPE_LONG,sizeof(int)));
		fields.push_back(MySQLPreparedStatement::Field(MYSQL_TYPE_VARCHAR,255));
		fields.push_back(MySQLPreparedStatement::Field(MYSQL_TYPE_VARCHAR,255));
		fields.push_back(MySQLPreparedStatement::Field(MYSQL_TYPE_LONG,sizeof(int)));
		fields.push_back(MySQLPreparedStatement::Field(MYSQL_TYPE_LONG,sizeof(int)));
		ptrStatement = 
			MySQLPreparedStatement::Create(this,pszSQL,fields);
		if(NULL == ptrStatement)
		{
			pTask->Release();
			return NULL;
		}
		ptrStatement->SetInt(0,pTask->GetIDX());
		ptrStatement->SetInt(1,nowTimeSecond);
		ptrStatement->SetString(2,pszNameBuffer);
		ptrStatement->SetString(3,strIP.c_str());
		ptrStatement->SetInt(4,getpid());
		ptrStatement->SetInt(5,pTask->GetTaskID());
		if(!ptrStatement->Excute())
		{
			pTask->Release();
			return NULL;
		}
	}
	transHelper.Commit();
	return pTask;
}

Task* MySQLConnection::GetTask(const std::wstring& strFunctionName)
{
	if(!m_bOpen)
		return NULL;

	if(!Ping())
		return NULL;

	//获取当前时间
	struct tm nowTime = GetDatabaseTime();
	int nowTimeSecond = mktime(&nowTime);

	TransactionGurad transHelper(this);
	if(!Lock())
		return NULL;
	
	//获取任务列表，按照level排序，status = 0的 （for update进行独占锁）
	//获取一个可用的TASK并更新它的状态等信息
	//sql
	const char* pszSQL = 
		"select `id`,`idx`,`level`,`func`,`ns`,`updatetime`,`timeout`,`data` FROM `TASK` where `func` = ? and `status` = 0 order by `level` desc";
	std::vector<MySQLPreparedStatement::Field> fields;
	fields.push_back(MySQLPreparedStatement::Field(MYSQL_TYPE_VARCHAR,255));
	Ptr<MySQLPreparedStatement> ptrStatement = 
		MySQLPreparedStatement::Create(this,pszSQL,fields);
	if(NULL == ptrStatement)
		return NULL;
	ptrStatement->SetString(0,Unicode2Utf8(strFunctionName).c_str());

	if(!ptrStatement->Excute())
		return NULL;
	RuntimeTask* pTask = NULL;
	while(ptrStatement->Fetch())
	{
		int id = ptrStatement->GetInt(0);
		int idx = ptrStatement->GetInt(1);
		int level = ptrStatement->GetInt(2);
		const char* pszFunc = ptrStatement->GetString(3);
		const char* pszNS = ptrStatement->GetString(4);
		int updatetime = ptrStatement->GetInt(5);
		int timeout = ptrStatement->GetInt(6);
		const void* pBuffer = NULL;
		unsigned int nBufferLen = 0;
		ptrStatement->GetValue(7,&pBuffer,&nBufferLen);
		if(updatetime + timeout > nowTimeSecond)
			continue;
		pTask = RuntimeTask::Create();
		pTask->SetTaskID(id);
		pTask->SetIDX(++idx);
		pTask->SetFunctionName(Utf82Unicode(pszFunc));
		pTask->SetNamespace(Utf82Unicode(pszNS));
		pTask->SetTaskLevel(level);
		pTask->SetTimeOut(timeout);
		pTask->SetUpdateTime(nowTimeSecond);
		pTask->SetData((const unsigned char*)pBuffer,nBufferLen);
		break;
	}
	ptrStatement = NULL;
	if(pTask)
	{
		char pszNameBuffer[128] = {0};  
		gethostname(pszNameBuffer,128);
		//获得本地IP地址  
		struct hostent* pHost;  
		pHost=gethostbyname(pszNameBuffer);//pHost返回的是指向主机的列表  
		std::string strIP;
		for (int i=0;pHost!=NULL&&pHost->h_addr_list[i]!=NULL;i++)  
		{  
			LPCSTR psz = inet_ntoa(*(struct in_addr *)pHost->h_addr_list[i]);//得到指向ip的psz变量  
			strIP += psz;  
			break;
		}  

		pszSQL = "UPDATE `TASK` SET `idx` = ?,`updatetime`=?,`host`=?,`ip`=?,`pid`=? where id = ?";
		fields.clear();
		fields.push_back(MySQLPreparedStatement::Field(MYSQL_TYPE_LONG,sizeof(int)));
		fields.push_back(MySQLPreparedStatement::Field(MYSQL_TYPE_LONG,sizeof(int)));
		fields.push_back(MySQLPreparedStatement::Field(MYSQL_TYPE_VARCHAR,255));
		fields.push_back(MySQLPreparedStatement::Field(MYSQL_TYPE_VARCHAR,255));
		fields.push_back(MySQLPreparedStatement::Field(MYSQL_TYPE_LONG,sizeof(int)));
		fields.push_back(MySQLPreparedStatement::Field(MYSQL_TYPE_LONG,sizeof(int)));
		ptrStatement = 
			MySQLPreparedStatement::Create(this,pszSQL,fields);
		if(NULL == ptrStatement)
		{
			pTask->Release();
			return NULL;
		}
		ptrStatement->SetInt(0,pTask->GetIDX());
		ptrStatement->SetInt(1,nowTimeSecond);
		ptrStatement->SetString(2,pszNameBuffer);
		ptrStatement->SetString(3,strIP.c_str());
		ptrStatement->SetInt(4,getpid());
		ptrStatement->SetInt(5,pTask->GetTaskID());
		if(!ptrStatement->Excute())
		{
			pTask->Release();
			return NULL;
		}
	}
	transHelper.Commit();
	return pTask;
}
void MySQLConnection::Abort(const Task* pTask)
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

	TransactionGurad transHelper(this);
	if(!Lock())
		return;

	const char* pszSQL = "select idx from `TASK` where id = ?";
	std::vector<MySQLPreparedStatement::Field> fields;
	fields.push_back(MySQLPreparedStatement::Field(MYSQL_TYPE_LONG,sizeof(int)));
	Ptr<MySQLPreparedStatement>  ptrStatement = 
		MySQLPreparedStatement::Create(this,pszSQL,fields);
	if(NULL == ptrStatement)
		return ;
	ptrStatement->SetInt(0,pRuntimeTask->GetTaskID());

	if(!ptrStatement->Excute())
		return;
	if(!ptrStatement->Fetch())
		return;
	//检查idx是否一致不一致说明超时等原因被其他人拿走了。
	int nIdx = ptrStatement->GetInt(0);
	if(nIdx != pRuntimeTask->GetIDX())
		return;
	ptrStatement = NULL;
	//更新Task的状态
	pszSQL = "UPDATE `TASK` SET updatetime = 0,`STATUS` = 0 where id=?";
	ptrStatement = 
		MySQLPreparedStatement::Create(this,pszSQL,fields);
	if(NULL == ptrStatement)
		return ;
	ptrStatement->SetInt(0,pRuntimeTask->GetTaskID());

	if(!ptrStatement->Excute())
		return;
	ptrStatement = NULL;
	transHelper.Commit();
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

	TransactionGurad transHelper(this);
	if(!Lock())
		return;

	const char* pszSQL = "select idx from `TASK` where id = ?";
	std::vector<MySQLPreparedStatement::Field> fields;
	fields.push_back(MySQLPreparedStatement::Field(MYSQL_TYPE_LONG,sizeof(int)));
	Ptr<MySQLPreparedStatement>  ptrStatement = 
		MySQLPreparedStatement::Create(this,pszSQL,fields);
	if(NULL == ptrStatement)
		return ;
	ptrStatement->SetInt(0,pRuntimeTask->GetTaskID());
	
	if(!ptrStatement->Excute())
		return;
	if(!ptrStatement->Fetch())
		return;
	//检查idx是否一致不一致说明超时等原因被其他人拿走了。
	int nIdx = ptrStatement->GetInt(0);
	if(nIdx != pRuntimeTask->GetIDX())
		return;
	ptrStatement = NULL;
	//更新Task的状态
	pszSQL = "UPDATE `TASK` SET finishDatetime = now(),`STATUS` = 1 where id=?";
	ptrStatement = 
		MySQLPreparedStatement::Create(this,pszSQL,fields);
	if(NULL == ptrStatement)
		return ;
	ptrStatement->SetInt(0,pRuntimeTask->GetTaskID());

	if(!ptrStatement->Excute())
		return;
	ptrStatement = NULL;
	transHelper.Commit();
}

void MySQLConnection::CreateTasks(const std::vector<Ptr<Task> >& tasks
									, const std::wstring& strNameSapce)
{
	if(tasks.empty())
		return;

	if(!m_bOpen)
		return;

	if(!Ping())
		return;

	//先检查是不是有这个namespace
	std::vector<std::wstring> ns = GetNameSpaces();
	bool bHasNS = false;
	for (std::vector<std::wstring>::const_iterator citer = ns.begin();
		citer != ns.end();
		++citer)
	{
		if(_wcsicmp(citer->c_str(),strNameSapce.c_str()) == 0)
		{
			bHasNS = true;
			break;
		}
	}
	if(!bHasNS)
		return;
	
	TransactionGurad transHelper(this);
	if(!Lock())
		return;

	const char* pszSQL = "insert into `TASK`(level,func,ns,timeout,data) values(?,?,?,?,?)";

	std::vector<MySQLPreparedStatement::Field> fields;
	fields.push_back(MySQLPreparedStatement::Field(MYSQL_TYPE_LONG,sizeof(int)));
	fields.push_back(MySQLPreparedStatement::Field(MYSQL_TYPE_VARCHAR,255));
	fields.push_back(MySQLPreparedStatement::Field(MYSQL_TYPE_VARCHAR,255));
	fields.push_back(MySQLPreparedStatement::Field(MYSQL_TYPE_LONG,sizeof(int)));
	fields.push_back(MySQLPreparedStatement::Field(MYSQL_TYPE_MEDIUM_BLOB,MAX_BLOB_LEN));
	Ptr<MySQLPreparedStatement>  ptrStatement = 
		MySQLPreparedStatement::Create(this,pszSQL,fields);
	if(NULL == ptrStatement)
		return ;

	
	std::string strUTF8NS = Unicode2Utf8(strNameSapce);
	unsigned int nBufferLen = 0;
	for (std::vector<Ptr<Task> >::const_iterator citer = tasks.begin();
		citer != tasks.end();
		++citer)
	{
		Task* pTask = citer->operator Task *();
		ptrStatement->SetInt(0,pTask->GetTaskLevel());
		ptrStatement->SetString(1,Unicode2Utf8(pTask->GetFunctionName()).c_str());
		ptrStatement->SetString(2,strUTF8NS.c_str());
		ptrStatement->SetInt(3,pTask->GetTimeOut());

		const unsigned char* pBuffer = pTask->GetData(nBufferLen);
		ptrStatement->SetValue(4,pBuffer,nBufferLen);
		
		if(!ptrStatement->Excute())
			return;
	}
	
	transHelper.Commit();
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

	std::string strUtf8Func = Unicode2Utf8(strFunction);
	std::string strUtf8NS = Unicode2Utf8(strNameSpace);
	//nFinished
	const char* pszSQL = 
		"SELECT count(*) from TASK where func = ? and ns = ? and status = 1";
	std::vector<MySQLPreparedStatement::Field> fields;
	fields.push_back(MySQLPreparedStatement::Field(MYSQL_TYPE_VARCHAR,255));
	fields.push_back(MySQLPreparedStatement::Field(MYSQL_TYPE_VARCHAR,255));
	Ptr<MySQLPreparedStatement> ptrStatement = 
		MySQLPreparedStatement::Create(this,pszSQL,fields);
	if(NULL == ptrStatement)
		return ;
	ptrStatement->SetString(0,strUtf8Func.c_str());
	ptrStatement->SetString(1,strUtf8NS.c_str());
	if(!ptrStatement->Excute())
		return;
	if(!ptrStatement->Fetch())
		return;

	nFinished = ptrStatement->GetInt(0);
	ptrStatement = NULL;

	//nTotal
	pszSQL = 
		"SELECT count(*) from TASK where func = ? and ns = ?";
	ptrStatement = 
		MySQLPreparedStatement::Create(this,pszSQL,fields);
	if(NULL == ptrStatement)
		return ;
	ptrStatement->SetString(0,strUtf8Func.c_str());
	ptrStatement->SetString(1,strUtf8NS.c_str());
	if(!ptrStatement->Excute())
		return;
	if(!ptrStatement->Fetch())
		return;

	nTotal = ptrStatement->GetInt(0);
	ptrStatement = NULL;
}

void MySQLConnection::DeleteTasks(const std::wstring& strFunction
	, const std::wstring& strNameSpace)
{
	if(!m_bOpen)
		return;

	if(!Ping())
		return;

	
	std::string strUTF8Func = Unicode2Utf8(strFunction);
	std::string strUTF8NS = Unicode2Utf8(strNameSpace);
	const char* pszSQL = 
		"SELECT count(*) from `TASK` where func = ? and ns = ? and status = 0";
	std::vector<MySQLPreparedStatement::Field> fields;
	fields.push_back(MySQLPreparedStatement::Field(MYSQL_TYPE_VARCHAR,255));
	fields.push_back(MySQLPreparedStatement::Field(MYSQL_TYPE_VARCHAR,255));
	Ptr<MySQLPreparedStatement> ptrStatement = 
		MySQLPreparedStatement::Create(this,pszSQL,fields);
	if(NULL == ptrStatement)
		return ;
	ptrStatement->SetString(0,strUTF8Func.c_str());
	ptrStatement->SetString(1,strUTF8NS.c_str());
	if(!ptrStatement->Excute())
		return;
	if(!ptrStatement->Fetch())
		return;
	//查询如果有记录就不能删除
	int nCount = ptrStatement->GetInt(0);
	if(nCount != 0)
		return;
	ptrStatement = NULL;

	
	TransactionGurad transHelper(this);
	if(!Lock())
		return;

	pszSQL = "DELETE FROM TASK WHERE func = ? and ns = ?";
	ptrStatement = 
		MySQLPreparedStatement::Create(this,pszSQL,fields);
	if(NULL == ptrStatement)
		return ;
	ptrStatement->SetString(0,strUTF8Func.c_str());
	ptrStatement->SetString(1,strUTF8NS.c_str());
	if(!ptrStatement->Excute())
		return;

	ptrStatement = NULL;

	transHelper.Commit();
}
std::vector<std::wstring> MySQLConnection::GetNameSpaces()
{
	std::vector<std::wstring> ns;
	if(!m_bOpen)
		return ns;

	if(!Ping())
		return ns;
	std::string strDatabase = Unicode2Utf8(m_connInfo->GetDatabase());
	const char* pszSQL = 
		"select TABLE_NAME from INFORMATION_SCHEMA.TABLES where TABLE_SCHEMA= ?";
	std::vector<MySQLPreparedStatement::Field> fields;
	fields.push_back(MySQLPreparedStatement::Field(MYSQL_TYPE_VARCHAR,255));
	Ptr<MySQLPreparedStatement> ptrStatement = 
		MySQLPreparedStatement::Create(this,pszSQL,fields);
	if(NULL == ptrStatement)
		return ns;
	ptrStatement->SetString(0,strDatabase.c_str());
	if(!ptrStatement->Excute())
		return ns;
	while(ptrStatement->Fetch())
	{
		const char* pszValue = ptrStatement->GetString(0);
		if(stricmp(pszValue,"TASK") == 0 || stricmp(pszValue,"LOCK") == 0)
			continue;
		ns.push_back(Utf82Unicode(pszValue));
	}
	return ns;
}
bool MySQLConnection::CreateNameSpace(const std::wstring& strNameSpace)
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

bool MySQLConnection::DeleteNameSpace(const std::wstring& strNameSpace)
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

bool MySQLConnection::ReadData(const std::wstring& strNameSpace
										, const std::wstring& strKey
										, const void** pBuffer
										, int& nBufferLen)
{
	if(!m_bOpen)
		return false;

	if(!Ping())
		return false;

	//sql
	std::string strTable = Unicode2Utf8(strNameSpace);
	std::string strUTF8Key = Unicode2Utf8(strKey);
	if(strUTF8Key.length() >=255)
		return false;

	char sql[1024];
	sprintf(sql,"SELECT `value` from `%s` where `key` = ?",strTable.c_str());
	std::vector<MySQLPreparedStatement::Field> fields;
	fields.push_back(MySQLPreparedStatement::Field(MYSQL_TYPE_VARCHAR,255));
	Ptr<MySQLPreparedStatement> ptrStatement = 
		MySQLPreparedStatement::Create(this,sql,fields);
	if(NULL == ptrStatement)
		return false;
	ptrStatement->SetValue(0,strUTF8Key.c_str(),strUTF8Key.length());

	if(!ptrStatement->Excute())
		return false;
	if(!ptrStatement->Fetch())
		return false;

	
	const void* p = NULL;
	nBufferLen = 0;
	ptrStatement->GetValue(0,(const void**)&p,(unsigned int*)&nBufferLen);
	if(nBufferLen > m_tempBufferLen)
	{
		m_tempBufferLen = nBufferLen;
		ResizeMemory(&m_tempBuffer,m_tempBufferLen);
	}
	memcpy(m_tempBuffer,p,nBufferLen);
	*pBuffer = m_tempBuffer;

	return true;
}

bool MySQLConnection::WriteData(const std::wstring& strNameSpace
								, const std::wstring& strKey
								, const void* pBuffer
								, int nBufferLen)
{
	if(!m_bOpen)
		return false;

	if(!Ping())
		return false;
	//sql
	std::string strTable = Unicode2Utf8(strNameSpace);
	std::string strUTF8Key = Unicode2Utf8(strKey);
	if(strUTF8Key.length() >=255)
		return false;


	char sql[1024];
	sprintf(sql
			, "INSERT `%s`(`key`,`value`) values (?,?) ON DUPLICATE KEY UPDATE `value` = ?"
			, strTable.c_str());

	std::vector<MySQLPreparedStatement::Field> fields;
	fields.push_back(MySQLPreparedStatement::Field(MYSQL_TYPE_VARCHAR,255));
	fields.push_back(MySQLPreparedStatement::Field(MYSQL_TYPE_MEDIUM_BLOB,nBufferLen));
	fields.push_back(MySQLPreparedStatement::Field(MYSQL_TYPE_MEDIUM_BLOB,nBufferLen));
	Ptr<MySQLPreparedStatement> ptrStatement = 
		MySQLPreparedStatement::Create(this,sql,fields);
	if(NULL == ptrStatement)
		return false;
	ptrStatement->SetValue(0,strUTF8Key.c_str(),strUTF8Key.length());
	ptrStatement->SetValue(1,pBuffer,nBufferLen);
	ptrStatement->SetValue(2,pBuffer,nBufferLen);
	if(!ptrStatement->Excute())
		return false;
	return true;
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
	
	m_tempBufferLen = 1024 * 1024;
	m_tempBuffer = malloc(m_tempBufferLen);

	return true;
}

void MySQLConnection::Close()
{
	if(m_tempBuffer)
	{
		free(m_tempBuffer);
		m_tempBuffer = NULL;
	}

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

	const char* ps = mysql_error((MYSQL*)m_mysql);
	return n == 0;
}

void MySQLConnection::SetTransactionIsolation(TransactionIsolation isolation)
{
	m_isolation = isolation;
}

IConnection::TransactionIsolation MySQLConnection::GetTransactionIsolation() const
{
	return m_isolation;
}

const char* MySQLConnection::GetTransactionIsolationString(TransactionIsolation isolate)
{
	switch(isolate)
	{
	case IConnection::ReadCommitted:
		return "READ COMMITTED";
	case IConnection::ReadUnCommitted:
		return "READ UNCOMMITTED";
	case  IConnection::RepeatableRead:
		return "REPEATABLE READ";
	default:
		return "SERIALIABLE";
	}
}

bool MySQLConnection::IsLostConnection()
{
	MYSQL* mysql = (MYSQL*)m_mysql;
	return CR_SERVER_GONE_ERROR == mysql_errno(mysql) || mysql->net.vio == NULL;

}

bool MySQLConnection::BeginTrans()
{
	if(!Ping())
		return false;
	if(m_IsTranStarted)
		return false;

	MYSQL* mysql = (MYSQL*)m_mysql;
	if(mysql_query(mysql,"ROLLBACK"))
		return false;

	char sql[1024];
	sprintf(sql,"SET TRANSACTION ISOLATION LEVEL %s",GetTransactionIsolationString(m_isolation));
	if(mysql_query(mysql,sql))
		return false;
	if(mysql_query(mysql,"START TRANSACTION"))
		return false;

	m_IsTranStarted = true;
	return true;
}

bool MySQLConnection::CommitTrans()
{
	if(IsLostConnection())
		return false;

	if(!m_IsTranStarted)
		return false;
	MYSQL* mysql = (MYSQL*)m_mysql;
	if(mysql_query(mysql,"COMMIT"))
		return false;
	return true;
}

bool MySQLConnection::RollbackTrans()
{
	if(IsLostConnection())
		return false;

	if(!m_IsTranStarted)
		return false;

	MYSQL* mysql = (MYSQL*)m_mysql;
	if(mysql_query(mysql,"ROLLBACK"))
		return false;
	return true;
}

bool MySQLConnection::TranStarted() const
{
	return m_IsTranStarted;
}

NS_END