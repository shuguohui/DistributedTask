#include "MySQLPreparedStatement.h"
NS_TASK
//MYSQL 默认是1m，因此比默认小一点
#define DEFAUT_BLOBSIZE 1024 * 512
MySQLPreparedStatement::MySQLPreparedStatement()
	: m_paramCount(0)
	, m_resultCount(0)
	, m_stmt(NULL)
	, m_param_bind(NULL)
	, m_param_buffer(NULL)
	, m_param_isNULLs(NULL)
	, m_param_lens(NULL)
	, m_result_bind(NULL)
	, m_result_buffer(NULL)
	, m_result_isNULLs(NULL)
	, m_result_lens(NULL)
	, m_result(NULL)
	, m_fields(NULL)
	, m_param_lens_static(NULL)
{

}
MySQLPreparedStatement::~MySQLPreparedStatement()
{
	for (std::map<int,char*>::iterator iter = m_blob_buffers.begin();
		iter != m_blob_buffers.end();
		++iter)
	{
		if(iter->second)
			free(iter->second);
	}
	if(m_result)
	{
		mysql_free_result(m_result);
		m_result = NULL;
		m_fields = NULL;
	}
	if(m_stmt)
	{
		if(m_stmt->mysql->net.vio)
			mysql_stmt_close(m_stmt);
		m_stmt = NULL;
	}
	if(m_param_bind)
		delete[] m_param_bind;

	if(m_param_isNULLs)
		delete[] m_param_isNULLs;

	if(m_param_lens)
		delete[] m_param_lens;
	if(m_param_lens_static)
		delete[] m_param_lens_static;

	if(m_param_buffer)
	{
		for (int i = 0;i <m_paramCount;i++)
		{
			if(m_param_buffer[i])
				free(m_param_buffer[i]);
		}
		free(m_param_buffer);
	}

	if(m_result_bind)
		delete[] m_result_bind;

	if(m_result_isNULLs)
		delete[] m_result_isNULLs;

	if(m_result_lens)
		delete[] m_result_lens;
	if(m_result_buffer)
	{
		for (int i = 0;i <m_resultCount;i++)
		{
			if(m_result_buffer[i])
				free(m_result_buffer[i]);
		}
		free(m_result_buffer);
	}
}

MySQLPreparedStatement* MySQLPreparedStatement::Create(MySQLConnection* pConn
														, const char* pszSQL
														, const std::vector<Field>& params)
{
	MySQLPreparedStatement* pStatement = new MySQLPreparedStatement;
	pStatement->m_conn = pConn;

	if(pStatement->Init(pszSQL,params))
		return pStatement;
	else
	{
		delete pStatement;
		return NULL;
	}
}

int MySQLPreparedStatement::FindCharacterCount(const char* pszSQL,char c)
{
	if(NULL == pszSQL)
		return 0;

	size_t len = strlen(pszSQL);
	int nCount = 0;
	for (size_t i = 0;i < len;i++)
	{
		if(pszSQL[i] == c)
			nCount++;
	}
	return nCount;
}

bool MySQLPreparedStatement::Init(const char* pszSQL, const std::vector<Field>& params)
{
	if(NULL == pszSQL || strlen(pszSQL) == 0)
		return false;
	if(!m_conn->Ping())
		return false;

	//查找sql语句中?的个数
	m_paramCount = FindCharacterCount(pszSQL,'\?');
	MYSQL* mysql = (MYSQL*)m_conn->GetMySQLConn();
	m_stmt = mysql_stmt_init(mysql);
	if(NULL ==  m_stmt)
		return false;

	if(m_paramCount > 0 && m_paramCount == params.size())
	{
		m_param_bind = new MYSQL_BIND[m_paramCount];
		m_param_isNULLs = new my_bool[m_paramCount];
		m_param_lens = new unsigned long[m_paramCount];
		m_param_lens_static = new unsigned long[m_paramCount];
		m_param_buffer = (char**)malloc(m_paramCount * sizeof(char*));
		if( NULL == m_param_bind || NULL == m_param_isNULLs ||
			NULL == m_param_lens || NULL == m_param_lens_static ||
			NULL == m_param_buffer)
			return false;
		memset(m_param_bind,0,m_paramCount * sizeof(MYSQL_BIND));
		memset(m_param_isNULLs,1,m_paramCount * sizeof(my_bool));
		memset(m_param_lens,0,m_paramCount * sizeof(unsigned long));
		memset(m_param_lens_static,0,m_paramCount * sizeof(unsigned long));
		memset(m_param_buffer,0,m_paramCount * sizeof(char*));
		for (int i = 0;i < m_paramCount;i++)
		{
			const Field& field = params[i];
			m_param_bind[i].is_null = &(m_param_isNULLs[i]);
			m_param_bind[i].length = &(m_param_lens[i]);
			m_param_bind[i].buffer_type = field.fieldType;

			if(field.fieldLen == MYSQL_TYPE_LONG_BLOB)
			{
				m_param_bind[i].buffer_length = DEFAUT_BLOBSIZE;
				m_param_lens[i] = m_param_lens_static[i] = DEFAUT_BLOBSIZE;
				m_param_buffer[i] = (char*)malloc(DEFAUT_BLOBSIZE);
				m_param_bind[i].buffer = m_param_buffer[i];
				m_blob_lens[i + 1024] = DEFAUT_BLOBSIZE;
				m_blob_buffers[i + 1024] = NULL;
			}
			else
			{
				m_param_bind[i].buffer_length = field.fieldLen;
				m_param_lens[i] = m_param_lens_static[i] = field.fieldLen;
				m_param_buffer[i] = (char*)malloc(field.fieldLen);
				m_param_bind[i].buffer = m_param_buffer[i];
			}
			
		}
	}

	if(mysql_stmt_prepare(m_stmt,pszSQL,strlen(pszSQL)))
	{
		m_conn->SetLastErrorCode(mysql_stmt_errno(m_stmt));
		m_conn->SetLastErrorMsg(mysql_stmt_error(m_stmt));
		return false;
	}
	if(m_paramCount > 0)
	{
		if(mysql_stmt_bind_param(m_stmt,m_param_bind))
		{
			m_conn->SetLastErrorCode(mysql_stmt_errno(m_stmt));
			m_conn->SetLastErrorMsg(mysql_stmt_error(m_stmt));
			return false;
		}
	}

	m_result = mysql_stmt_result_metadata(m_stmt);
	if(m_result)
	{
		m_fields = mysql_fetch_fields(m_result);
		m_resultCount = mysql_num_fields(m_result);
		if(m_resultCount)
		{
			m_result_bind = new MYSQL_BIND[m_resultCount];
			m_result_isNULLs = new my_bool[m_resultCount];
			m_result_lens = new unsigned long[m_resultCount];
			m_result_buffer = (char**)malloc(m_resultCount * sizeof(char*));
			if(NULL == m_result_bind || NULL == m_result_isNULLs ||
				NULL == m_result_lens || NULL == m_result_buffer)
				return false;

			memset(m_result_bind,0,m_resultCount * sizeof(MYSQL_BIND));
			memset(m_result_isNULLs,1,m_resultCount * sizeof(my_bool));
			memset(m_result_lens,0,m_resultCount * sizeof(unsigned long));
			memset(m_result_buffer,0,m_resultCount * sizeof(char*));
			for (int i = 0;i <  m_resultCount;i++)
			{
				m_result_bind[i].is_null = &(m_result_isNULLs[i]);
				m_result_bind[i].length = &(m_result_lens[i]);
				m_result_bind[i].buffer_type = m_fields[i].type;

				if(m_fields[i].type == MYSQL_TYPE_DATETIME)
				{
					int nLen = sizeof(MYSQL_TIME);
					m_result_bind[i].buffer_length = nLen;
					m_result_lens[i] = nLen;
					m_result_buffer[i] = (char*)malloc(nLen);
					m_result_bind[i].buffer = m_result_buffer[i];
				}
				else if(m_fields[i].type == MYSQL_TYPE_LONG_BLOB)
				{
					m_result_lens[i] = m_fields[i].length;
					m_result_buffer[i] = (char*)malloc(m_fields[i].length);
					m_result_bind[i].buffer = NULL;
					m_result_bind[i].buffer_length = 0;

					m_blob_lens[i] = DEFAUT_BLOBSIZE;
					m_blob_buffers[i] = NULL;
				}
				else
				{
					m_result_lens[i] = m_fields[i].length;
					m_result_buffer[i] = (char*)malloc(m_fields[i].length);
					m_result_bind[i].buffer = m_result_buffer[i];
					m_result_bind[i].buffer_length = m_fields[i].length;
				}
				
			}
		}
	}
	return true;
}


bool MySQLPreparedStatement::Fetch()
{
	int n = mysql_stmt_fetch(m_stmt);
	if(n == MYSQL_DATA_TRUNCATED)
		n = 0;
	return n == 0;
}

bool MySQLPreparedStatement::SendLongBinaryData()
{
	int bindBufferLen,nLen;
	char* pBuffer = NULL;
	for (int i = 0;i < m_paramCount;i++)
	{
		nLen = m_param_bind[i].buffer_length;
		if(m_param_bind[i].buffer_type == MYSQL_TYPE_LONG_BLOB && nLen > DEFAUT_BLOBSIZE)
		{
			pBuffer = m_blob_buffers[i + 1024];
			*(m_param_bind[i].length) = 0;
			m_param_bind[i].buffer_length = 0;
			bindBufferLen = DEFAUT_BLOBSIZE >= nLen ? nLen : DEFAUT_BLOBSIZE;
			bool bDone = false;
			unsigned long nWriteLen = bindBufferLen;
			unsigned long nTotalWriteLen = 0;
			while(!bDone)
			{
				if(mysql_stmt_send_long_data(m_stmt,i,pBuffer + nTotalWriteLen,nWriteLen))
				{
					m_conn->SetLastErrorCode(mysql_stmt_errno(m_stmt));
					m_conn->SetLastErrorMsg(mysql_stmt_error(m_stmt));
					return false;
				}
				nTotalWriteLen += nWriteLen;
				if(nTotalWriteLen == nLen)
				{
					bDone = true;
					continue;
				}
				if(nTotalWriteLen + bindBufferLen >= nLen)
					nWriteLen = nLen - nTotalWriteLen;
				else
					nWriteLen = bindBufferLen;
			}
		}
	}
}

bool MySQLPreparedStatement::Excute()
{
	if(m_conn->IsLostConnection())
		return false;
	if(!SendLongBinaryData())
		return false;

	int nErr = mysql_stmt_execute(m_stmt);
	if(nErr)
	{
		m_conn->SetLastErrorCode(mysql_stmt_errno(m_stmt));
		m_conn->SetLastErrorMsg(mysql_stmt_error(m_stmt));
		return false;
	}
	if(m_result)
	{
		if(mysql_stmt_bind_result(m_stmt,m_result_bind))
		{
			m_conn->SetLastErrorCode(mysql_stmt_errno(m_stmt));
			m_conn->SetLastErrorMsg(mysql_stmt_error(m_stmt));
		}
		return false;
	}
	for (int i = 0;i < m_paramCount;i++)
	{
		m_param_isNULLs[i] = 1;
	}
	return true;
}

bool MySQLPreparedStatement::IsNULL(int nPos)
{
	if(nPos > m_resultCount)
		return true;
	return m_result_isNULLs[nPos] == 1;
}

void MySQLPreparedStatement::SetNULL(int nPos)
{
	if(nPos > m_paramCount)
		return;
	*(m_param_bind[nPos].is_null) = 1;

}

void MySQLPreparedStatement::SetDateTime(int nPos,const MYSQL_TIME& tm)
{
	SetValue(nPos,&tm,sizeof(MYSQL_TIME));
}

void MySQLPreparedStatement::SetValue(int nPos,const void* pValue,unsigned int nLen)
{
	if(nPos >= m_paramCount || nLen > m_param_lens_static[nPos])
		return;

	if(nLen <= DEFAUT_BLOBSIZE)
	{
		memcpy(m_param_bind[nPos].buffer,pValue,nLen);
		*(m_param_bind[nPos].is_null) = 0;
		*(m_param_bind[nPos].length) = nLen;
		m_param_bind[nPos].buffer_length = nLen;
	}
	else
	{
		SetBlob(nPos,pValue,nLen);
	}

}

void MySQLPreparedStatement::SetBlob(int nPos,const void* pValue,unsigned int nLen)
{
	int nIndex = nPos + 1024;
	char* pBuffer = m_blob_buffers[nIndex];
	int nBufferLen = m_blob_lens[nIndex];
	if(nLen > nBufferLen)
	{
		if(pBuffer)
			ResizeMemory((void**)&pBuffer,nLen);
		else
			pBuffer = (char*)malloc(nLen);
		m_blob_lens[nIndex] = nLen;
		m_blob_buffers[nIndex] = pBuffer;
	}
	memcpy(m_blob_buffers[nIndex],pValue,nLen);
	*(m_param_bind[nPos].is_null) = 0;
	*(m_param_bind[nPos].length) = nLen;
	m_param_bind[nPos].buffer_length = nLen;
}

void MySQLPreparedStatement::GetValue(int nPos, const void** pValue,unsigned int* pLen)
{
	if(nPos >m_resultCount || m_result_isNULLs[nPos] == 1)
	{
		*pValue = NULL;
		*pLen = 0;
	}
	else
	{
		if(m_result_bind[nPos].buffer_type == MYSQL_TYPE_LONG_BLOB)
		{
			GetBlob(nPos,pValue,pLen);
		}
		else
		{
			*pValue = m_result_buffer[nPos];
			*pLen = m_result_lens[nPos];
		}
	}
}
void MySQLPreparedStatement::GetBlob(int nPos,const void** pValue,unsigned int* pLen)
{
	unsigned long nLen = m_result_lens[nPos];
	*pValue = m_result_buffer[nPos];
	if(nLen > DEFAUT_BLOBSIZE)
	{
		char* pBuffer = m_blob_buffers[nPos];
		if(nLen > m_blob_lens[nPos])
		{
			m_blob_lens[nPos] = nLen;
			if(pBuffer)
				ResizeMemory((void**)&pBuffer,nLen);
			else
				pBuffer = (char*)malloc(nLen);
		}
		m_result_bind[nPos].buffer = pBuffer;
		m_blob_buffers[nPos] = pBuffer;
		*pValue = m_result_buffer[nPos];
	}
	else
	{
		m_result_bind[nPos].buffer = m_result_buffer[nPos];
	}
	m_result_bind[nPos].buffer_length = nLen;
	if(mysql_stmt_fetch_column(m_stmt,&m_result_bind[nPos],nPos,0))
	{
		m_conn->SetLastErrorCode(mysql_stmt_errno(m_stmt));
		m_conn->SetLastErrorMsg(mysql_stmt_error(m_stmt));

		*pValue = NULL;
		*pLen = 0;
		return;
	}
	*pLen = m_result_lens[nPos];
}

void MySQLPreparedStatement::SetInt(int nPos,int nValue)
{
	SetValue(nPos,&nValue,sizeof(nValue));
}

void MySQLPreparedStatement::SetString(int nPos,const char* pszValue)
{
	if(pszValue && strlen(pszValue) > 0)
		SetValue(nPos,pszValue,strlen(pszValue));
	else
		SetNULL(nPos);
}

int MySQLPreparedStatement::GetInt(int nPos)
{
	return *((int*)m_result_buffer[nPos]);
}

MYSQL_TIME MySQLPreparedStatement::GetDateTime(int nPos)
{
	return *((MYSQL_TIME*)m_result_buffer[nPos]);
}

const char* MySQLPreparedStatement::GetString(int nPos)
{
	return (const char*)m_result_buffer[nPos];
}
NS_END