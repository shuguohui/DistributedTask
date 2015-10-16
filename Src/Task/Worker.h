#ifndef _TASK_WORKER_H_
#define _TASK_WORKER_H_
#include <Task/stdafx.h>
#include <Core/IDisposable.h>
#include <Core/ConnectionInfo.h>
#include <Core/Ptr.h>
#include <Core/Task.h>
#include <Provider/IProvider/IConnection.h>
#include <Provider/IProvider/IDataCursor.h>
#include <Task/Function.h>
#include <string>
#include <map>
NS_TASK
class GVI_API_TASK Worker : public IDisposable
{
protected:
	Worker();

	virtual ~Worker();

	virtual void Dispose()
	{
		delete this;
	}

public:
	static Worker* Create(const ConnectionInfo* pConn);

	void RegisterFunction(Function* pFunc);

	void UnRegisterFunction(const std::wstring& strFuncName);

	void Run();

	void Stop();

	bool  ReadData(const std::wstring& strNameSpace
					, const std::wstring& strKey
					, const void** pBuffer
					, int& nBufferLen);

	IDataCursor* ReadDataCursor(const std::wstring& strNameSpace
								, const std::vector<std::wstring>& keys);

	bool WriteData(const std::wstring& strNameSpace
					, const std::wstring& strKey
					, const void* pBuffer
					, int nBufferLen);

	const ConnectionInfo* GetConnectionInfo();

	IConnection* GetConnection();
private:
	Ptr<ConnectionInfo> m_connInfo;
	Ptr<IConnection> m_conn;
	bool m_bRunning;
	std::map<std::wstring,Ptr<Function> > m_regiterFuncs;
};
NS_END
#endif