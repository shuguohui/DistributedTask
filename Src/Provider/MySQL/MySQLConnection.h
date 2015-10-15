#ifndef _MYSQLCONNECTION_H_
#define _MYSQLCONNECTION_H_
#include <Core/ConnectionInfo.h>
#include <Core/IDisposable.h>
#include <Provider/IProvider/IConnection.h>
#include <Provider/IProvider/stdafx.h>
#include <string>

#define  MAX_BLOB_LEN 16 * 1024 * 1024
class GVI_API_TASK_PROVIDER MySQLConnection : public IConnection
{
protected:
	MySQLConnection();
	virtual ~MySQLConnection();

	virtual void Dispose()
	{
		delete this;
	}
public:
	static MySQLConnection* Create();

	virtual bool Open(const ConnectionInfo* pConnInfo);

	virtual  bool CreateRepository(const ConnectionInfo* pConn);

	virtual  bool DropRepository(const ConnectionInfo* pConn);

	virtual Task* GetTask(const std::wstring& strFunctionName);

	virtual void FinishTask(const Task* pTask);

	virtual void CreateTasks(const std::vector<Ptr<Task> >& tasks);

	virtual void GetTaskStatus(const std::wstring& strFunction
								, const std::wstring& strNameSpace
								, int& nFinished
								, int& nTotal);

	virtual void DeleteTasks(const std::wstring& strFunction
							, const std::wstring& strNameSpace);

	virtual bool CreateDataNameSpace(const std::wstring& strNameSpace);

	virtual bool DeleteDataNameSpace(const std::wstring& strNameSpace);

	virtual bool ReadData(const std::wstring& strNameSpace
								, const std::wstring& strKey
								, const void** pBuffer
								, int& nBufferLen);

	virtual bool WriteData(const std::wstring& strNameSpace
							, const std::wstring& strKey
							, const void* pBuffer
							, int nBufferLen);

	virtual void SetTransactionIsolation(TransactionIsolation isolation);

	virtual TransactionIsolation GetTransactionIsolation() const;

	virtual bool BeginTrans();

	virtual bool CommitTrans();

	virtual bool RollbackTrans();

	virtual bool TranStarted() const;

public:
	void* GetMySQLConn()
	{
		return m_mysql;
	}
	bool IsLostConnection();
	bool Ping();

	struct tm GetDatabaseTime();

	struct tm GetLocalTime();
private:
	void Close();
	const char* GetTransactionIsolationString(TransactionIsolation isolate);
private:
	void* m_mysql;
	bool m_bOpen;
	TransactionIsolation m_isolation;
	bool m_IsTranStarted;
	Ptr<ConnectionInfo> m_connInfo;
	void* m_tempBuffer;

};
#endif
