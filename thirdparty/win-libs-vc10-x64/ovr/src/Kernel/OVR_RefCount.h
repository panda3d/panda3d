/************************************************************************************

PublicHeader:   Kernel
Filename    :   OVR_RefCount.h
Content     :   Reference counting implementation headers
Created     :   September 19, 2012
Notes       : 

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

************************************************************************************/

#ifndef OVR_RefCount_h
#define OVR_RefCount_h

#include "OVR_Types.h"
#include "OVR_Allocator.h"

namespace OVR {

//-----------------------------------------------------------------------------------
// ***** Reference Counting

// There are three types of reference counting base classes:
//
//  RefCountBase     - Provides thread-safe reference counting (Default).
//  RefCountBaseNTS  - Non Thread Safe version of reference counting.


// ***** Declared classes

template<class C>
class   RefCountBase;
template<class C>
class   RefCountBaseNTS;

class   RefCountImpl;
class   RefCountNTSImpl;


//-----------------------------------------------------------------------------------
// ***** Implementation For Reference Counting

// RefCountImplCore holds RefCount value and defines a few utility
// functions shared by all implementations.

class RefCountImplCore
{
protected:
    volatile int RefCount;

public:
    // RefCountImpl constructor always initializes RefCount to 1 by default.
    OVR_FORCE_INLINE RefCountImplCore() : RefCount(1) { }

    // Need virtual destructor
    // This:    1. Makes sure the right destructor's called.
    //          2. Makes us have VTable, necessary if we are going to have format needed by InitNewMem()
    virtual ~RefCountImplCore();

    // Debug method only.
    int GetRefCount() const { return RefCount;  }

    // This logic is used to detect invalid 'delete' calls of reference counted
    // objects. Direct delete calls are not allowed on them unless they come in
    // internally from Release.
#ifdef OVR_BUILD_DEBUG    
    static void   OVR_CDECL  reportInvalidDelete(void *pmem);
    inline static void checkInvalidDelete(RefCountImplCore *pmem)
    {
        if (pmem->RefCount != 0)
            reportInvalidDelete(pmem);
    }
#else
    inline static void checkInvalidDelete(RefCountImplCore *) { }
#endif

    // Base class ref-count content should not be copied.
    void operator = (const RefCountImplCore &) { }  
};

class RefCountNTSImplCore
{
protected:
    mutable int RefCount;

public:
    // RefCountImpl constructor always initializes RefCount to 1 by default.
    OVR_FORCE_INLINE RefCountNTSImplCore() : RefCount(1) { }

    // Need virtual destructor
    // This:    1. Makes sure the right destructor's called.
    //          2. Makes us have VTable, necessary if we are going to have format needed by InitNewMem()
    virtual ~RefCountNTSImplCore();

    // Debug method only.
    int             GetRefCount() const { return RefCount;  }

    // This logic is used to detect invalid 'delete' calls of reference counted
    // objects. Direct delete calls are not allowed on them unless they come in
    // internally from Release.
#ifdef OVR_BUILD_DEBUG    
    static void   OVR_CDECL  reportInvalidDelete(void *pmem);
    OVR_FORCE_INLINE static void checkInvalidDelete(RefCountNTSImplCore *pmem)
    {
        if (pmem->RefCount != 0)
            reportInvalidDelete(pmem);
    }
#else
    OVR_FORCE_INLINE static void checkInvalidDelete(RefCountNTSImplCore *) { }
#endif

    // Base class ref-count content should not be copied.
    void operator = (const RefCountNTSImplCore &) { }  
};



// RefCountImpl provides Thread-Safe implementation of reference counting, so
// it should be used by default in most places.

class RefCountImpl : public RefCountImplCore
{
public:
    // Thread-Safe Ref-Count Implementation.
    void    AddRef();
    void    Release();   
};

// RefCountVImpl provides Thread-Safe implementation of reference counting, plus,
// virtual AddRef and Release.

class RefCountVImpl : public RefCountImplCore
{
public:
    // Thread-Safe Ref-Count Implementation.
    virtual void      AddRef();
    virtual void      Release();   
};


// RefCountImplNTS provides Non-Thread-Safe implementation of reference counting,
// which is slightly more efficient since it doesn't use atomics.

class RefCountNTSImpl : public RefCountNTSImplCore
{
public:
    OVR_FORCE_INLINE void    AddRef() const { RefCount++; }
    void    Release() const;   
};



// RefCountBaseStatImpl<> is a common class that adds new/delete override with Stat tracking
// to the reference counting implementation. Base must be one of the RefCountImpl classes.

template<class Base>
class RefCountBaseStatImpl : public Base
{
public:
    RefCountBaseStatImpl() { }
     
    // *** Override New and Delete

    // DOM-IGNORE-BEGIN
    // Undef new temporarily if it is being redefined
#ifdef OVR_DEFINE_NEW
#undef new
#endif

#ifdef OVR_BUILD_DEBUG
    // Custom check used to detect incorrect calls of 'delete' on ref-counted objects.
    #define OVR_REFCOUNTALLOC_CHECK_DELETE(class_name, p)   \
        do {if (p) Base::checkInvalidDelete((class_name*)p); } while(0)
#else
    #define OVR_REFCOUNTALLOC_CHECK_DELETE(class_name, p)
#endif

    // Redefine all new & delete operators.
    OVR_MEMORY_REDEFINE_NEW_IMPL(Base, OVR_REFCOUNTALLOC_CHECK_DELETE)

#ifdef OVR_DEFINE_NEW
#define new OVR_DEFINE_NEW
#endif
        // OVR_BUILD_DEFINE_NEW
        // DOM-IGNORE-END
};



//-----------------------------------------------------------------------------------
// *** End user RefCountBase<> classes


// RefCountBase is a base class for classes that require thread-safe reference
// counting; it also overrides the new and delete operators to use MemoryHeap.
//
// Reference counted objects start out with RefCount value of 1. Further lifetime
// management is done through the AddRef() and Release() methods, typically
// hidden by Ptr<>.

template<class C>
class RefCountBase : public RefCountBaseStatImpl<RefCountImpl>
{
public:    
    // Constructor.
    OVR_FORCE_INLINE RefCountBase() : RefCountBaseStatImpl<RefCountImpl>() { }    
};

// RefCountBaseV is the same as RefCountBase but with virtual AddRef/Release

template<class C>
class RefCountBaseV : public RefCountBaseStatImpl<RefCountVImpl>
{
public:    
    // Constructor.
    OVR_FORCE_INLINE RefCountBaseV() : RefCountBaseStatImpl<RefCountVImpl>() { }    
};


// RefCountBaseNTS is a base class for classes that require Non-Thread-Safe reference
// counting; it also overrides the new and delete operators to use MemoryHeap.
// This class should only be used if all pointers to it are known to be assigned,
// destroyed and manipulated within one thread.
//
// Reference counted objects start out with RefCount value of 1. Further lifetime
// management is done through the AddRef() and Release() methods, typically
// hidden by Ptr<>.

template<class C>
class RefCountBaseNTS : public RefCountBaseStatImpl<RefCountNTSImpl>
{
public:    
    // Constructor.
    OVR_FORCE_INLINE RefCountBaseNTS() : RefCountBaseStatImpl<RefCountNTSImpl>() { }    
};

//-----------------------------------------------------------------------------------
// ***** Pickable template pointer
enum PickType { PickValue };

template <typename T>
class Pickable
{
public:
    Pickable() : pV(NULL) {}
    explicit Pickable(T* p) : pV(p) {}
    Pickable(T* p, PickType) : pV(p) 
    {
        OVR_ASSERT(pV);
        if (pV)
            pV->AddRef();
    }
    template <typename OT>
    Pickable(const Pickable<OT>& other) : pV(other.GetPtr()) {}

public:
    Pickable& operator =(const Pickable& other)
    {
        OVR_ASSERT(pV == NULL);
        pV = other.pV;
        // Extra check.
        //other.pV = NULL;
        return *this;
    }

public:
    T* GetPtr() const { return pV; }
    T* operator->() const
    {
        return pV;
    }
    T& operator*() const
    {
        OVR_ASSERT(pV);
        return *pV;
    }

private:
    T* pV;
};

template <typename T>
OVR_FORCE_INLINE
Pickable<T> MakePickable(T* p)
{
    return Pickable<T>(p);
}

//-----------------------------------------------------------------------------------
// ***** Ref-Counted template pointer

// Automatically AddRefs and Releases interfaces

void* ReturnArg0(void* p);

template<class C>
class Ptr
{
#ifdef OVR_CC_ARM
    static C* ReturnArg(void* p) { return (C*)ReturnArg0(p); }
#endif

protected:
    C   *pObject;

public:

    // Constructors
    OVR_FORCE_INLINE Ptr() : pObject(0)
    { }
#ifdef OVR_CC_ARM
    OVR_FORCE_INLINE Ptr(C &robj) : pObject(ReturnArg(&robj))
#else
    OVR_FORCE_INLINE Ptr(C &robj) : pObject(&robj)
#endif
    { }
    OVR_FORCE_INLINE Ptr(Pickable<C> v) : pObject(v.GetPtr())
    {
        // No AddRef() on purpose.
    }
    OVR_FORCE_INLINE Ptr(Ptr<C>& other, PickType) : pObject(other.pObject)
    {
        other.pObject = NULL;
        // No AddRef() on purpose.
    }
    OVR_FORCE_INLINE Ptr(C *pobj)
    {
        if (pobj) pobj->AddRef();   
        pObject = pobj;
    }
    OVR_FORCE_INLINE Ptr(const Ptr<C> &src)
    {
        if (src.pObject) src.pObject->AddRef();     
        pObject = src.pObject;
    }

    template<class R>
    OVR_FORCE_INLINE Ptr(Ptr<R> &src)
    {
        if (src) src->AddRef();
        pObject = src;
    }
    template<class R>
    OVR_FORCE_INLINE Ptr(Pickable<R> v) : pObject(v.GetPtr())
    {
        // No AddRef() on purpose.
    }

    // Destructor
    OVR_FORCE_INLINE ~Ptr()
    {
        if (pObject) pObject->Release();        
    }

    // Compares
    OVR_FORCE_INLINE bool operator == (const Ptr &other) const       { return pObject == other.pObject; }
    OVR_FORCE_INLINE bool operator != (const Ptr &other) const       { return pObject != other.pObject; }

    OVR_FORCE_INLINE bool operator == (C *pother) const              { return pObject == pother; }
    OVR_FORCE_INLINE bool operator != (C *pother) const              { return pObject != pother; }


    OVR_FORCE_INLINE bool operator < (const Ptr &other) const       { return pObject < other.pObject; }

    // Assignment
    template<class R>
    OVR_FORCE_INLINE const Ptr<C>& operator = (const Ptr<R> &src)
    {
        if (src) src->AddRef();
        if (pObject) pObject->Release();        
        pObject = src;
        return *this;
    }   
    // Specialization
    OVR_FORCE_INLINE const Ptr<C>& operator = (const Ptr<C> &src)
    {
        if (src) src->AddRef();
        if (pObject) pObject->Release();        
        pObject = src;
        return *this;
    }   
    
    OVR_FORCE_INLINE const Ptr<C>& operator = (C *psrc)
    {
        if (psrc) psrc->AddRef();
        if (pObject) pObject->Release();        
        pObject = psrc;
        return *this;
    }   
    OVR_FORCE_INLINE const Ptr<C>& operator = (C &src)
    {       
        if (pObject) pObject->Release();        
        pObject = &src;
        return *this;
    }
    OVR_FORCE_INLINE Ptr<C>& operator = (Pickable<C> src)
    {
        return Pick(src);
    }
    template<class R>
    OVR_FORCE_INLINE Ptr<C>& operator = (Pickable<R> src)
    {
        return Pick(src);
    }
    
    // Set Assignment
    template<class R>
    OVR_FORCE_INLINE Ptr<C>& SetPtr(const Ptr<R> &src)
    {
        if (src) src->AddRef();
        if (pObject) pObject->Release();
        pObject = src;
        return *this;
    }
    // Specialization
    OVR_FORCE_INLINE Ptr<C>& SetPtr(const Ptr<C> &src)
    {
        if (src) src->AddRef();
        if (pObject) pObject->Release();
        pObject = src;
        return *this;
    }   
    
    OVR_FORCE_INLINE Ptr<C>& SetPtr(C *psrc)
    {
        if (psrc) psrc->AddRef();
        if (pObject) pObject->Release();
        pObject = psrc;
        return *this;
    }   
    OVR_FORCE_INLINE Ptr<C>& SetPtr(C &src)
    {       
        if (pObject) pObject->Release();
        pObject = &src;
        return *this;
    }
    OVR_FORCE_INLINE Ptr<C>& SetPtr(Pickable<C> src)
    {       
        return Pick(src);
    }

    // Nulls ref-counted pointer without decrement
    OVR_FORCE_INLINE void    NullWithoutRelease()    
    { 
        pObject = 0;    
    }

    // Clears the pointer to the object
    OVR_FORCE_INLINE void    Clear()
    {
        if (pObject) pObject->Release();
        pObject = 0;
    }

    // Obtain pointer reference directly, for D3D interfaces
    OVR_FORCE_INLINE C*& GetRawRef()                 { return pObject; }

    // Access Operators
    OVR_FORCE_INLINE C* GetPtr() const               { return pObject; }
    OVR_FORCE_INLINE C& operator * () const          { return *pObject; }
    OVR_FORCE_INLINE C* operator -> ()  const        { return pObject; }
    // Conversion                   
    OVR_FORCE_INLINE operator C* () const            { return pObject; }

    // Pickers.

    // Pick a value.
    OVR_FORCE_INLINE Ptr<C>& Pick(Ptr<C>& other)
    {
        if (&other != this)
        {
            if (pObject) pObject->Release();
            pObject = other.pObject;
            other.pObject = 0;
        }

        return *this;
    }

    OVR_FORCE_INLINE Ptr<C>& Pick(Pickable<C> v)
    {
        if (v.GetPtr() != pObject)
        {
            if (pObject) pObject->Release();
            pObject = v.GetPtr();
        }

        return *this;
    }

    template<class R>
    OVR_FORCE_INLINE Ptr<C>& Pick(Pickable<R> v)
    {
        if (v.GetPtr() != pObject)
        {
            if (pObject) pObject->Release();
            pObject = v.GetPtr();
        }

        return *this;
    }

    OVR_FORCE_INLINE Ptr<C>& Pick(C* p)
    {
        if (p != pObject)
        {
            if (pObject) pObject->Release();
            pObject = p;
        }

        return *this;
    }
};

} // OVR

#endif
