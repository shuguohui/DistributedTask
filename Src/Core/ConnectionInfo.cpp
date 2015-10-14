#include "ConnectionInfo.h"
ConnectionInfo::ConnectionInfo()
	: m_timeout(0)
{

}

ConnectionInfo::~ConnectionInfo()
{

}


ConnectionInfo* ConnectionInfo::Create()
{
	return new	 ConnectionInfo;
}

ConnectionInfo::ConnectionType ConnectionInfo::GetType() const
{
	return m_Type;
}

void ConnectionInfo::SetType(ConnectionInfo::ConnectionType type)
{
	m_Type = type;
}

const std::wstring& ConnectionInfo::GetServer() const
{
	return m_server;
}

void ConnectionInfo::SetServer(const std::wstring& strServer)
{
	m_server = strServer;
}

const std::wstring& ConnectionInfo::GetDatabase() const
{
	return m_database;
}

void ConnectionInfo::SetDatabase(const std::wstring& strDatabase)
{
	m_database = strDatabase;
}

const std::wstring& ConnectionInfo::GetInstance() const
{
	return m_instance;
}

void ConnectionInfo::SetInstance(const std::wstring& strInstance)
{
	m_instance = strInstance;
}

const std::wstring& ConnectionInfo::GetUser() const
{
	return m_userName;
}

void ConnectionInfo::SetUser(const std::wstring& strUserName)
{
	m_userName = strUserName;
}

const std::wstring& ConnectionInfo::GetPassword() const
{
	return m_password;
}

void ConnectionInfo::SetPassword(const std::wstring& strPwd)
{
	m_password = strPwd;
}

int ConnectionInfo::GetPortNumber() const
{
	return m_port;
}

void ConnectionInfo::SetPortNumber(int nPort)
{
	m_port = nPort;
}

int ConnectionInfo::GetTimeout() const
{
	return m_timeout;
}

void ConnectionInfo::SetTimeout(int nTimeout)
{
	m_timeout = nTimeout;
}