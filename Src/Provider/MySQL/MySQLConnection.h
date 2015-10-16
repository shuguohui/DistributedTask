#ifndef _MYSQLCONNECTION_H_
#define _MYSQLCONNECTION_H_
#include <Core/ConnectionInfo.h>
#include <Core/IDisposable.h>
#include <Provider/IProvider/IConnection.h>
#include <Provider/IProvider/IDataCursor.h>
#include <Provider/IProvider/stdafx.h>
#include <string>

#define  MAX_BLOB_LEN 16 * 1024 * 1024
NS_TASK
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

	virtual Task* GetTask(const std::vector<std::wstring>& funcNames);

	virtual void FinishTask(const Task* pTask);

	virtual void Abort(const Task* pTask);

	virtual void CreateTasks(const std::vector<Ptr<Task> >& tasks
							, const std::wstring& strNameSapce);

	virtual void GetTaskStatus(const std::wstring& strFunction
								, const std::wstring& strNameSpace
								, int& nFinished
								, int& nTotal);

	virtual void DeleteTasks(const std::wstring& strFunction
							, const std::wstring& strNameSpace);

	virtual std::vector<std::wstring> GetNameSpaces();

	virtual bool CreateNameSpace(const std::wstring& strNameSpace);

	virtual bool DeleteNameSpace(const std::wstring& strNameSpace);

	virtual bool ReadData(const std::wstring& strNameSpace
								, const std::wstring& strKey
								, const void** pBuffer
								, int& nBufferLen);

	virtual IDataCursor* ReadDataCursor(const std::wstring& strNameSpace
										, const std::vector<std::wstring>& keys);

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

	bool Lock();

	inline void ResizeMemory( void** ppv, size_t size )
	{
		void** ppb = (void**)ppv;
		void* pbResize;
		pbResize = realloc(*ppb, size);
		if( pbResize != NULL )
			*ppb = pbResize;
	}
private:
	void* m_mysql;
	bool m_bOpen;
	TransactionIsolation m_isolation;
	bool m_IsTranStarted;
	Ptr<ConnectionInfo> m_connInfo;
	void* m_tempBuffer;
	int   m_tempBufferLen;

};
NS_END
#endif
