#ifndef _IDISPOSABLE_H_
#define _IDISPOSABLE_H_
#include "stdafx.h"
#ifdef _WIN32
#pragma once
#endif

class IDisposable
{
protected:

    GVI_API_TASK_CORE IDisposable() : m_refCount(0), m_objectThreadLockingEnabled(false) {};

    
    GVI_API_TASK_CORE virtual ~IDisposable() {};


    GVI_API_TASK_CORE virtual void Dispose() = 0;
	
public:

    GVI_API_TASK_CORE static void EnableGlobalThreadLocking(bool enable) { m_globalThreadLockingEnabled = enable; }

 
    GVI_API_TASK_CORE static bool GetGlobalThreadLockingFlag() { return m_globalThreadLockingEnabled; }

public:

    GVI_API_TASK_CORE virtual int AddRef();

 
    GVI_API_TASK_CORE virtual int Release();


    GVI_API_TASK_CORE virtual int GetRefCount() { return m_refCount; }


    GVI_API_TASK_CORE virtual void EnableObjectThreadLocking(bool enable) { m_objectThreadLockingEnabled = enable; }

protected:
    bool m_objectThreadLockingEnabled;

private:
    int m_refCount;
    static bool m_globalThreadLockingEnabled;
};


#define SAFE_RELEASE(x) {if (x) (x)->Release(); (x) = NULL;}
#define SAFE_ADDREF(x)  ((x != NULL) ? (x)->AddRef(), (x) : (NULL))

#endif


