#include "MySQLDataCursor.h"
#include "MySQLPreparedStatement.h"
#include <Core/utf8_.h>

NS_TASK
	MySQLDataCursor::MySQLDataCursor()
	:m_stmt(NULL)
{

}

MySQLDataCursor::~MySQLDataCursor()
{
	if(m_stmt)
	{
		m_stmt->Release();
		m_stmt = NULL;
	}
}
MySQLDataCursor* MySQLDataCursor::Create(MySQLConnection* pConn
										, const std::wstring& strNameSpace
										, const std::vector<std::wstring>& keys)
{
	int num_key = keys.size();
	std::string strWhereClause;
	if(num_key > 0)
	{
		std::stringstream ss;
		ss << " where `key` in (?";
		for (int i = 1; i < num_key;i++ )
		{
			ss << ",?";
		}
		ss << ")";
		strWhereClause =  ss.str();
	}
	std::string strUTF8NS = Unicode2Utf8(strNameSpace);
	char szSQL[8096];
	sprintf(szSQL
		, "select `key`,`value` from `%s` %s"
		, strUTF8NS.c_str()
		, strWhereClause.c_str());
	std::vector<MySQLPreparedStatement::Field> fields;
	for (int i = 0;i < num_key;i++)
	{
		fields.push_back(MySQLPreparedStatement::Field(MYSQL_TYPE_VARCHAR,255));
	}

	MySQLPreparedStatement* stmt = 
		MySQLPreparedStatement::Create(pConn,szSQL,fields);
	if(NULL == stmt)
		return NULL;
	for (int i = 0;i < num_key;i++)
	{
		stmt->SetString(i,Unicode2Utf8(keys[i]).c_str());
	}

	MySQLDataCursor* pCursor = new MySQLDataCursor;
	pCursor->m_conn = pConn;
	pCursor->m_stmt = stmt;
	if(!stmt->Excute())
	{
		pCursor->Release();
		return NULL;
	}

	return pCursor;
}

bool MySQLDataCursor::Next()
{
	return m_stmt->Fetch();
}

void MySQLDataCursor::GetKeyValue(std::wstring* strKey
								, const void** pDataBuffer
								, unsigned int* pnDataBufferLen)
{
	

	if(strKey)
	{
		const char* pszKey = m_stmt->GetString(0);
		*strKey = Utf82Unicode(pszKey);
	}
	if(pDataBuffer && pnDataBufferLen)
		m_stmt->GetValue(1,pDataBuffer,pnDataBufferLen);

}

NS_END