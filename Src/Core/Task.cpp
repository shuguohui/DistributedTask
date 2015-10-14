#include "Task.h"

Task::Task()
	:m_data(NULL)
	, m_dataLen(0)
	, m_timeout(0)
{

}

Task::~Task()
{
	if(m_data)
		free(m_data);
}

const std::wstring& Task::GetFunctionName() const
{
	return m_functionName;
}

void Task::SetFunctionName(const std::wstring& strFuncName)
{
	m_functionName = strFuncName;
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
		memcpy(m_data,pBuffer,nBufferLen);
	}
}

unsigned int Task::GetTaskLevel() const
{
	return m_level;
}

void Task::SetTaskLevel(unsigned int nLevel)
{
	m_level = nLevel;
}