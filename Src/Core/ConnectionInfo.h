#ifndef _CONNECTIONINFO_H_
#define _CONNECTIONINFO_H_
#include "IDisposable.h"
#include <string>
class GVI_API_TASK ConnectionInfo : public IDisposable
{
public:
	enum ConnectionType
	{
		UNKNOWN = 0,
		MYSQL = 1,
		ORACLE = 2,
	};
protected:
	ConnectionInfo();

	virtual ~ConnectionInfo();

	virtual void Dispose()
	{
		delete this;
	}

public:
	static ConnectionInfo* Create();

	ConnectionInfo* Clone() const;

	ConnectionInfo::ConnectionType GetType() const;

	void SetType(ConnectionInfo::ConnectionType type);

	const std::wstring& GetServer() const;

	void SetServer(const std::wstring& strServer);

	const std::wstring& GetDatabase() const;

	void SetDatabase(const std::wstring& strDatabase);

	const std::wstring& GetInstance() const;

	void SetInstance(const std::wstring& strInstance);

	const std::wstring& GetUser() const;

	void SetUser(const std::wstring& strUserName);

	const std::wstring& GetPassword() const;

	void SetPassword(const std::wstring& strPwd);

	int GetPortNumber() const;

	void SetPortNumber(int nPort);

	int GetTimeout() const;

	void SetTimeout(int nTimeout);
private:
	std::wstring m_server;
	std::wstring m_database;
	std::wstring m_instance;
	std::wstring m_userName;
	std::wstring m_password;
	unsigned int m_port;
	unsigned int m_timeout;
	ConnectionType m_Type;
};

#endif