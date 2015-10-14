#ifndef _MYSQLCONNECTION_H_
#define _MYSQLCONNECTION_H_
#include <Core/ConnectionInfo.h>
#include <Provider/IProvider/IConnection.h>
#include <Provider/IProvider/stdafx.h>
#include <string>


class GVI_API_TASK_PROVIDER MySQLConnection : public IConnection
{
protected:
	MySQLConnection();
	virtual ~MySQLConnection();

	virtual void Dispose()
	{
		delete this;
	}
public:
	static MySQLConnection* Create(const ConnectionInfo* pConnInfo);

	virtual bool CreateRepository();

	virtual bool DropRepository();

	virtual Task* GetTask(const std::wstring& strFunctionName);

	virtual void FinishTask(const Task* pTask);

	virtual void CreateTasks(const std::vector<Ptr<Task> >& tasks);

	virtual void GetTaskStatus(const std::wstring& strFunction
								, const std::wstring& strNameSpace
								, int& nFinished
								, int& nTotal);

	virtual void DeleteTasks(const std::wstring& strFunction
							, const std::wstring& strNameSpace);

private:
	void Close();

	bool Open(const ConnectionInfo* pConnInfo);
private:
	void* m_mysql;

};
#endif
