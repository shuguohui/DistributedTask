#ifndef _IDATACURSOR_H_
#define _IDATACURSOR_H_
#include <Core/IDisposable.h>
#include <Core/Ptr.h>
#include <Core/Task.h>
#include <string>
#include <vector>
NS_TASK
class IDataCursor : public IDisposable
{
public:
	virtual bool Next() = 0;

	virtual void GetKeyValue(std::wstring* strKey
							, const void** pDataBuffer
							, unsigned int* pnDataBufferLen) = 0;
};
NS_END
#endif
