#ifndef _ICONNECTION_H_
#define _ICONNECTION_H_
#include "IDisposable.h"
#include "Ptr.h"
#include "Task.h"
#include <string>
#include <vector>
class IConnection : public IDisposable
{
public:
	virtual bool CreateRepository();

	virtual bool DropRepository();

	virtual Task* GetTask(const std::wstring& strFunctionName);

	virtual void FinishTask(const Task* pTask);

	virtual void CreateTasks(const std::vector<Ptr<Task> >& tasks);

	void GetTaskStatus(const std::wstring& strFunction
						, const std::wstring& strNameSpace
						, int& nFinished
						, int& nTotal);

	//todo 只有全完成的Task才能删除？只要有一个还没完成就删除失败？
	void DeleteTasks(const std::wstring& strFunction
					, const std::wstring& strNameSpace);
}
