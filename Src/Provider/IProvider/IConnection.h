#ifndef _ICONNECTION_H_
#define _ICONNECTION_H_
#include <Core/IDisposable.h>
#include <Core/Ptr.h>
#include <Core/Task.h>
#include <Core/ConnectionInfo.h>
#include <string>
#include <vector>
class IConnection : public IDisposable
{
public:
	virtual bool CreateRepository(const ConnectionInfo* pConn) = 0;

	virtual bool DropRepository(const ConnectionInfo* pConn) = 0;

	virtual bool Open(const ConnectionInfo* pConnInfo) = 0;

	virtual Task* GetTask(const std::wstring& strFunctionName) = 0;

	virtual void FinishTask(const Task* pTask) = 0;

	virtual void CreateTasks(const std::vector<Ptr<Task> >& tasks) = 0;

	virtual void GetTaskStatus(const std::wstring& strFunction
								, const std::wstring& strNameSpace
								, int& nFinished
								, int& nTotal) = 0;

	//todo 只有全完成的Task才能删除？只要有一个还没完成就删除失败？
	virtual void DeleteTasks(const std::wstring& strFunction
							, const std::wstring& strNameSpace) = 0;

	virtual bool CreateDataNameSpace(const std::wstring& strNameSpace) = 0;

	virtual bool DeleteDataNameSpace(const std::wstring& strNameSpace) = 0;

	virtual const void* ReadData(const std::wstring& strNameSpace
								, const std::wstring& strKey
								, int& nBufferLen) = 0;

	virtual void WriteData(const std::wstring& strNameSpace
							, const std::wstring& strKey
							, const void* pBuffer
							, int nBufferLen) = 0;
};
#endif
