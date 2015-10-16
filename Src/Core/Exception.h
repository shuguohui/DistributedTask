#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_
#include "stdafx.h"
#include <string>
NS_TASK
class GVI_API_TASK_CORE Exception
{
private:
	Exception();
public:
	virtual ~Exception();

	Exception(const std::wstring& strMessage,int nCode);

	Exception(const std::wstring& strMessage);

	
private:
	std::wstring m_message;
	int m_code;
};
NS_END
#endif