#ifndef _MYSQLPREPAREDSTATEMENT_H_
#define _MYSQLPREPAREDSTATEMENT_H_
#include "MySQLConnection.h"
#include <my_global.h>
#include <mysql.h>
#include <errmsg.h>
#include <string>

NS_TASK
class GVI_API_TASK_PROVIDER MySQLPreparedStatement: public IDisposable
{
public:
	struct Field
	{
		unsigned int fieldLen;
		enum_field_types fieldType;
		Field(enum_field_types type,unsigned int len)
		{
			fieldType = type;
			fieldLen = len;
		}
	};
protected:
	MySQLPreparedStatement();
	virtual ~MySQLPreparedStatement();

	virtual void Dispose()
	{
		delete this;
	}
public:
	static MySQLPreparedStatement* Create(MySQLConnection* pConn
										 , const char* pszSQL
										 , const std::vector<Field>& params);

	bool Fetch();

	bool Excute();

	bool IsNULL(int nPos);

	void SetNULL(int nPos);

	void SetDateTime(int nPos,const MYSQL_TIME& tm);

	MYSQL_TIME GetDateTime(int nPos);

	void SetInt(int nPos,int nValue);

	void SetString(int nPos,const char* pszValue);

	int GetInt(int nPos);

	const char* GetString(int nPos);

	void SetValue(int nPos,const void* pValue,unsigned int nLen);

	void GetValue(int nPos, const void** pValue,unsigned int* pLen);

	
private:
	bool Init(const char* pszSQL
			  , const std::vector<Field>& params);

	int FindCharacterCount(const char* pszSQL,char c);

	
private:
	Ptr<MySQLConnection>		m_conn;
	int							m_paramCount;
	int							m_resultCount;
	MYSQL_STMT*					m_stmt;
	MYSQL_RES*					m_result;
	MYSQL_FIELD*				m_fields;

	MYSQL_BIND*					m_param_bind;
	char**						m_param_buffer;
	my_bool*					m_param_isNULLs;
	unsigned long*				m_param_lens;
	unsigned long*				m_param_lens_static;

	MYSQL_BIND*					m_result_bind;
	char**						m_result_buffer;
	my_bool*					m_result_isNULLs;
	unsigned long*				m_result_lens;		
};
NS_END
#endif
