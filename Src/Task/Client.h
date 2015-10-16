#ifndef _TASK_CLIENT_H_
#define _TASK_CLIENT_H_
#include <Task/stdafx.h>
#include <Core/IDisposable.h>
#include <Core/ConnectionInfo.h>
#include <Core/Ptr.h>
#include <Core/Task.h>
#include <Provider/IProvider/IConnection.h>
#include <string>
#include <vector>
NS_TASK
class GVI_API_TASK Client : public IDisposable
{
protected:
	Client();

	virtual ~Client();

	virtual void Dispose()
	{
		delete this;
	}

public:
	static Client* Create(const ConnectionInfo* pConn);

	void CreateTasks(const std::vector<Ptr<Task> >& tasks
					, const std::wstring& strNameSapce);

	void GetTaskStatus(const std::wstring& strFunction
						, const std::wstring& strNameSpace
						, int& nFinished
						, int& nTotal);

	//todo 只有全完成的Task才能删除？只要有一个还没完成就删除失败？
	void DeleteTasks(const std::wstring& strFunction
					, const std::wstring& strNameSpace);

	bool CreateNameSpace(const std::wstring& strNameSpace);

	bool DeleteNameSpace(const std::wstring& strNameSpace);

	std::vector<std::wstring> GetNameSpaces();

	const ConnectionInfo* GetConnectionInfo();

	IConnection* GetConnection();
private:
	Ptr<ConnectionInfo> m_connInfo;
	Ptr<IConnection> m_conn;
};
NS_END
#endif