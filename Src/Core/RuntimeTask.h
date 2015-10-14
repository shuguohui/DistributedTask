#ifndef _RUNTIMETASK_H_
#define _RUNTIMETASK_H_
#include "Task.h"
#include <string>
class RuntimeTask : public Task
{
protected:
	RuntimeTask();

	virtual ~RuntimeTask();

	virtual void Dispose()
	{
		delete this;
	}

public:
	int GetUpdateTime() const;

	void SetUpdateTime(int nUpdateTime);

	int GetTaskID() const;

	void SetTaskID(int nID);

	int GetIDX() const;

	void SetIDX(int nIDX);

	const std::wstring& GetHost() const;

	void SetHost(const std::wstring& strHost);

	const std::wstring& GetIP() const;

	void SetIP(const std::wstring& strIPAddress);

private:
	int m_updateTime;
	int m_taskID;
	int m_idx;
	std::wstring m_host;
	std::wstring m_ipAddress;
};

#endif