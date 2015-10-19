#ifndef _TRANSACTIONGURAD_H_
#define _TRANSACTIONGURAD_H_
#include <Provider/IProvider/IConnection.h>
#include <Provider/IProvider/stdafx.h>
#include <string>
NS_TASK
class  GVI_API_TASK_PROVIDER TransactionGurad
{
public:
	TransactionGurad(IConnection* pConn);
	~TransactionGurad();

	bool Commit();
private:
	Ptr<IConnection> m_conn;
	bool m_bCommit;
	bool m_bInControl;
};
NS_END
#endif
