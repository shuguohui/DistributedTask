#ifndef _TASK_H_
#define _TASK_H_
#include "IDisposable.h"
#include <string>
class Task : public IDisposable
{
protected:
	Task();

	virtual ~Task();

	virtual void Dispose()
	{
		delete this;
	}

public:
	static Task* Create();

	const std::wstring& GetFunctionName() const;

	void SetFunctionName(const std::wstring& strFuncName);

	const std::wstring& GetNamespace() const;

	void SetNamespace(const std::wstring& strNamespaceName);

	unsigned int GetTimeOut() const;

	void SetTimeOut(unsigned int nTimeOut);

	unsigned int GetTaskLevel() const;

	void SetTaskLevel(unsigned int nLevel);

	void SetData(const unsigned char* pBuffer,unsigned int nBufferLen);
private:
	std::wstring m_functionName;
	std::wstring m_namespace;
	unsigned int m_level;
	unsigned m_dataLen;
	unsigned char* m_data;
	unsigned m_timeout;
};

#endif