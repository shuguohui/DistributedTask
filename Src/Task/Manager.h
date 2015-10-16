#ifndef _TASK_MANAGER_H_
#define _TASK_MANAGER_H_
#include <Task/stdafx.h>
#include <Core/IDisposable.h>
#include <Core/ConnectionInfo.h>
#include <Core/Ptr.h>
#include <string>
NS_TASK
class GVI_API_TASK Manager : public IDisposable
{
protected:
	Manager();

	virtual ~Manager();

	virtual void Dispose()
	{
		delete this;
	}

public:
	static Manager* Create(const ConnectionInfo* pConn);

	bool CreateRepository();

	bool DropRepository();

private:
	Ptr<ConnectionInfo> m_conn;

};
NS_END
#endif