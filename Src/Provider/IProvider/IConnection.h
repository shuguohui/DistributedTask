#ifndef _ICONNECTION_H_
#define _ICONNECTION_H_
#include <Core/IDisposable.h>
#include <Core/Ptr.h>
#include <Core/Task.h>
#include <Core/ConnectionInfo.h>
#include <string>
#include <vector>
NS_TASK
class IDataCursor;
class IConnection : public IDisposable
{
public:
	enum TransactionIsolation
	{
		ReadUnCommitted = 1,
		ReadCommitted = 2,
		RepeatableRead = 3,
		Serialiable = 4,
	};
public:
	virtual bool CreateRepository(const ConnectionInfo* pConn) = 0;

	virtual bool DropRepository(const ConnectionInfo* pConn) = 0;

	virtual bool Open(const ConnectionInfo* pConnInfo) = 0;

	virtual void SetIdentification(const std::wstring& strIdentification) = 0;

	//virtual std::vector<std::wstring> GetFunctionNames() = 0;

	virtual std::vector<std::wstring> GetNameSpaces() = 0;

	//virtual Task* GetTask(const std::wstring& strFunctionName) = 0;

	virtual Task* GetTask(const std::vector<std::wstring>& funcNames) = 0;

	virtual void FinishTask(const Task* pTask) = 0;

	virtual void Abort(const Task* pTask) = 0;

	virtual void CreateTasks(const std::vector<Ptr<Task> >& tasks
							, const std::wstring& strNameSapce) = 0;

	virtual void GetTaskStatus(const std::wstring& strFunction
								, const std::wstring& strNameSpace
								, int& nFinished
								, int& nTotal) = 0;

	//todo 只有全完成的Task才能删除？只要有一个还没完成就删除失败？
	virtual void DeleteTasks(const std::wstring& strFunction
							, const std::wstring& strNameSpace) = 0;

	virtual bool CreateNameSpace(const std::wstring& strNameSpace) = 0;

	virtual bool DeleteNameSpace(const std::wstring& strNameSpace) = 0;

	virtual bool  ReadData(const std::wstring& strNameSpace
								, const std::wstring& strKey
								, const void** pBuffer
								, int& nBufferLen) = 0;

	//如果keys为空就返回所有keys的data
	virtual IDataCursor* ReadDataCursor(const std::wstring& strNameSpace
										, const std::vector<std::wstring>& keys) = 0;

	virtual bool WriteData(const std::wstring& strNameSpace
							, const std::wstring& strKey
							, const void* pBuffer
							, int nBufferLen) = 0;

	//事务相关
	virtual void SetTransactionIsolation(TransactionIsolation isolation) = 0;

	virtual TransactionIsolation GetTransactionIsolation() const = 0;

	virtual bool BeginTrans() = 0;

	virtual bool CommitTrans() = 0;

	virtual bool RollbackTrans() = 0;

	virtual bool TranStarted() const  = 0;

	virtual const wchar_t* GetLastErrorMsg() const = 0;

	virtual int GetLastErrorCode() const = 0;
};
NS_END
#endif
