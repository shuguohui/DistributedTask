#include "TransactionGurad.h"
TransactionGurad::TransactionGurad(IConnection* pConn)
	:m_conn(pConn)
	, m_bCommit(false)
	, m_bInControl(true)
{
	if(pConn->TranStarted())
	{
		m_bInControl = false;
		return;
	}
	if(!pConn->BeginTrans())
		return;
}

TransactionGurad::~TransactionGurad()
{
	if(!m_bInControl)
		return;
	if(!m_bCommit)
	{
		if(!m_conn->RollbackTrans())
			//TODO 再来一次？
			m_conn->RollbackTrans();
	}
}

bool TransactionGurad::Commit()
{
	if(!m_bInControl)
		return false;

	if(!m_bCommit && m_conn->TranStarted())
	{
		if(m_conn->CommitTrans())
		{
			m_bCommit = true;
			return true;
		}
	}
	return false;
}