
#include "IDisposable.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#ifdef UNICODE
#define GetClassNameW  GetClassName
#else
#define GetClassNameA  GetClassName
#endif // !UNICODE
#endif
NS_TASK
/// Define the global state of the FdoIDisposable thread locking
///
bool IDisposable::m_globalThreadLockingEnabled = false;

/// \brief
/// Default implementation of FdoIDisposable::AddRef()
///
inline
int IDisposable::AddRef()
{
    if (m_globalThreadLockingEnabled || m_objectThreadLockingEnabled)
    {
#if defined(__GNUC__) && defined (__GNUC_MINOR__) && ((__GNUC__ < 4) || (__GNUC__ == 4 && __GNUC_MINOR__ < 1))
        int ret = 1;
        __asm__ __volatile__("lock;xaddl %0,%1"
            : "=r"(ret),"=m"(m_refCount)
            : "0"(ret),"m"(m_refCount)
            : "memory","cc");
        return ret + 1;
#elif defined (__GNUC__)
        return __sync_add_and_fetch(&m_refCount,1);
#elif defined(_WIN32)
        return InterlockedIncrement((LONG*)&m_refCount);
#endif
    }

    return ++m_refCount;
}

/// \brief
/// Default implementation of FdoIDisposable::Release()
///
inline
int IDisposable::Release()
{
    if (m_globalThreadLockingEnabled || m_objectThreadLockingEnabled)
    {
#if defined(__GNUC__) && defined (__GNUC_MINOR__) && ((__GNUC__ < 4) || (__GNUC__ == 4 && __GNUC_MINOR__ < 1))
        int ret = -1;
        __asm__ __volatile__("lock;xaddl %0,%1"
            : "=r"(ret),"=m"(m_refCount)
            : "0"(ret),"m"(m_refCount)
            : "memory","cc");
#elif defined (__GNUC__)
        __sync_sub_and_fetch(&m_refCount, 1);
#elif defined(_WIN32)
        InterlockedDecrement((LONG*)&m_refCount);
#endif
    }
    else
    {
        --m_refCount;
    }

    if (0 != m_refCount)
        return m_refCount;

    Dispose();
    return 0;
}

NS_END