#ifndef _TASK_FUNCTION_H_
#define _TASK_FUNCTION_H_
#include <Task/stdafx.h>
#include <Core/IDisposable.h>
#include <Core/Ptr.h>
#include <Core/Task.h>
#include <string>

NS_TASK
class Worker;

typedef bool (task_function_fn)(Task* pTask,Worker* pConn,void* worker_context);

class GVI_API_TASK Function : public IDisposable
{
protected:
	Function();

	virtual ~Function();

	virtual void Dispose()
	{
		delete this;
	}

public:
	static Function* Create(const std::wstring& strFunction,task_function_fn callback);

	const std::wstring& GetFunctionName() const; 

	task_function_fn* GetCallBack() const;
private:
	std::wstring m_funcName;
	task_function_fn* m_callback;
	
};
NS_END
#endif