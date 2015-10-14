#ifndef _ICONNECTION_H_
#define _ICONNECTION_H_
#include <Core/IDisposable.h>
#include <Core/Ptr.h>
#include <Core/Task.h>
#include <string>
#include <vector>
class IConnection : public IDisposable
{
public:
	virtual bool CreateRepository() = 0;

	virtual bool DropRepository() = 0;

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
};
#endif
