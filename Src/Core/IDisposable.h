#ifndef _IDISPOSABLE_H_
#define _IDISPOSABLE_H_
#include "stdafx.h"
#ifdef _WIN32
#pragma once
#endif

class IDisposable
{
protected:

    GVI_API_TASK IDisposable() : m_refCount(0), m_objectThreadLockingEnabled(false) {};

    
    GVI_API_TASK virtual ~IDisposable() {};


    GVI_API_TASK virtual void Dispose() = 0;
	
public:

    GVI_API_TASK static void EnableGlobalThreadLocking(bool enable) { m_globalThreadLockingEnabled = enable; }

 
    GVI_API_TASK static bool GetGlobalThreadLockingFlag() { return m_globalThreadLockingEnabled; }

public:

    GVI_API_TASK virtual int AddRef();

 
    GVI_API_TASK virtual int Release();


    GVI_API_TASK virtual int GetRefCount() { return m_refCount; }


    GVI_API_TASK virtual void EnableObjectThreadLocking(bool enable) { m_objectThreadLockingEnabled = enable; }

protected:
    bool m_objectThreadLockingEnabled;

private:
    int m_refCount;
    static bool m_globalThreadLockingEnabled;
};


#define SAFE_RELEASE(x) {if (x) (x)->Release(); (x) = NULL;}
#define SAFE_ADDREF(x)  ((x != NULL) ? (x)->AddRef(), (x) : (NULL))

#endif


