#ifndef _MYSQLDATACURSOR_H_
#define _MYSQLDATACURSOR_H_
#include <Core/IDisposable.h>
#include <Provider/IProvider/IDataCursor.h>
#include <Provider/IProvider/stdafx.h>
#include <Provider/MySQL/MySQLConnection.h>
#include <string>
NS_TASK
class MySQLPreparedStatement;
class GVI_API_TASK_PROVIDER MySQLDataCursor : public IDataCursor
{
	friend class MySQLConnection;
protected:
	MySQLDataCursor();
	virtual ~MySQLDataCursor();

	virtual void Dispose()
	{
		delete this;
	}
protected:
	static MySQLDataCursor* Create(MySQLConnection* pConn
									, const std::wstring& strNameSpace
									, const std::vector<std::wstring>& keys);
public:
	virtual bool Next();

	virtual void GetKeyValue(std::wstring* strKey
							, const void** pDataBuffer
							, unsigned int* pnDataBufferLen);
private:

	Ptr<MySQLConnection> m_conn;
	MySQLPreparedStatement* m_stmt;
};
NS_END
#endif
