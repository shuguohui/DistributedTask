#include "Task.h"
NS_TASK
Task::Task()
	:m_data(NULL)
	, m_dataLen(0)
	, m_timeout(60)
	, m_level(0)
	, m_SupportOMP(false)
{

}

Task::~Task()
{
	if(m_data)
		free(m_data);
}
int Task::GetTaskID() const
{
	return 0;
}
Task* Task::Create()
{
	return new Task;
}
const std::wstring& Task::GetFunctionName() const
{
	return m_functionName;
}

void Task::SetFunctionName(const std::wstring& strFuncName)
{
	m_functionName = strFuncName;
}

void Task::SetNamespace(const std::wstring& strNamespaceName)
{
	m_namespace = strNamespaceName;
}

const std::wstring& Task::GetNamespace() const
{
	return m_namespace;
}
unsigned int Task::GetTimeOut() const
{
	return m_timeout;
}

void Task::SetTimeOut(unsigned int nTimeOut)
{
	m_timeout = nTimeOut;
}

void Task::SetData(const unsigned char* pBuffer,unsigned int nBufferLen)
{
	if(m_data)
	{
		free(m_data);
		m_data = NULL;
	}
	if(NULL == pBuffer || nBufferLen == 0)
	{
		m_data = NULL;
		m_dataLen = 0;
	}
	else
	{
		m_data = (unsigned char*)malloc(nBufferLen);
		m_dataLen = nBufferLen;
		memcpy(m_data,pBuffer,nBufferLen);
	}
}

const unsigned char* Task::GetData(unsigned int& nBufferLen) const
{
	nBufferLen = m_dataLen;
	return m_data;
}

unsigned int Task::GetTaskLevel() const
{
	return m_level;
}

void Task::SetTaskLevel(unsigned int nLevel)
{
	m_level = nLevel;
}

bool Task::IsSupportOMP() const
{
	return m_SupportOMP;
}

void Task::SetSupportOMP(bool bSupport)
{
	m_SupportOMP = bSupport;
}
NS_END