
#ifndef _Ptr_H_
#define _Ptr_H_

template <class T>
class _NoAddRefReleaseOnPtr : public T
{
    private:
        virtual int AddRef()  = 0;
        virtual int Release() = 0;
};

template <class T>
#ifdef _WIN32
inline __declspec(nothrow) T* __stdcall PtrAssign(T** pp, T* lp)
#else
inline T* PtrAssign(T** pp, T* lp)
#endif
{
    if (lp != NULL)
        lp->AddRef();
    if (*pp)
        (*pp)->Release();
    *pp = lp;
    return lp;
}


template <class T>
class Ptr
{
public:
    typedef T _PtrClass;
    
    Ptr() throw()
    {
        p = NULL;
    }

    Ptr(T* lp) throw()
    {
        p = lp;
    }


    Ptr(const Ptr<T>& lp) throw()
    {
        p = lp.p;
        if (p != NULL)
            p->AddRef();
    }

    template <class U>                                              
    Ptr(const Ptr<U>& lp) throw()                             
    {
        p = lp.p;
        if (p != NULL)
            p->AddRef();
    }


    ~Ptr() throw()
    {
        if (p)
            p->Release();
    }


    operator T*() const throw()
    {
        return p;
    }


    T& operator*() const throw()
    {
        return *p;
    }


    T** operator&() throw()
    {
        return &p;
    }
    
    _NoAddRefReleaseOnPtr<T>* operator->() const throw()
    {
        return (_NoAddRefReleaseOnPtr<T>*)p;
    }
    

    bool operator!() const throw()
    {
        return (p == NULL);
    }
    

    bool operator<(T* pT) const  throw()
    {
        return p < pT;
    }
    

    bool operator==(T* pT) const throw()
    {
        return p == pT;
    }

    
    T* operator=(T* lp) throw()
    {
        SAFE_RELEASE(p);
        p = lp;
        return p;
    }
    
    T* operator=(const Ptr<T>& lp) throw()
    {
        return static_cast<T*>(PtrAssign<T>(&p, lp));
    }

    
    template<class U>
    T* operator=(const Ptr<U>& lp) throw()
    {
        return static_cast<T*>(PtrAssign<T>(&p, lp));
    }

    void Release() throw()
    {
        T* pTemp = p;
        if (pTemp)
        {
            p = NULL;
            pTemp->Release();
        }
    }

    void Attach(T* p2) throw()
    {
        if (p)
            p->Release();
        p = p2;
    }

    
    T* Detach() throw()
    {
        T* pt = p;
        p = NULL;
        return pt;
    }

    
    bool CopyTo(T** ppT) throw()
    {
        if (ppT == NULL)
            return false;
        *ppT = p;
        if (p)
            p->AddRef();
        return true;
    }

    T* p;
};
#endif


