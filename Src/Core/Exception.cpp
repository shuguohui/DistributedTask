#include "Exception.h"
NS_TASK
Exception::Exception()
{

}

Exception::~Exception()
{

}

Exception::Exception(const std::wstring& strMessage,int nCode)
{
	m_message = strMessage;
	m_code = nCode;
}

Exception::Exception(const std::wstring& strMessage)
{
	m_message = strMessage;
	m_code = 0;
}
NS_END