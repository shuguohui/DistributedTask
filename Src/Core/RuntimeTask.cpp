

#include "RuntimeTask.h"

RuntimeTask::RuntimeTask()
{

}

RuntimeTask::~RuntimeTask()
{

}

RuntimeTask* RuntimeTask::Create()
{
	return new RuntimeTask;
}

int RuntimeTask::GetUpdateTime() const
{
	return m_updateTime;
}

void RuntimeTask::SetUpdateTime(int nUpdateTime)
{
	m_updateTime = nUpdateTime;
}

int RuntimeTask::GetTaskID() const
{
	return m_taskID;
}

void RuntimeTask::SetTaskID(int nID)
{
	m_taskID = nID;
}

int RuntimeTask::GetIDX() const
{
	return m_idx;
}

void RuntimeTask::SetIDX(int nIDX)
{
	m_idx;
}

const std::wstring& RuntimeTask::GetHost() const
{
	return m_host;
}

void RuntimeTask::SetHost(const std::wstring& strHost)
{
	m_host = strHost;
}

const std::wstring& RuntimeTask::GetIP() const
{
	return m_ipAddress;
}

void RuntimeTask::SetIP(const std::wstring& strIPAddress)
{
	m_ipAddress = strIPAddress;
}
