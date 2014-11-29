/************************************************************************************

Filename    :   OVR_RefCount.cpp
Content     :   Reference counting implementation
Created     :   September 19, 2012
Notes       : 

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

************************************************************************************/

#include "OVR_RefCount.h"
#include "OVR_Atomic.h"
#include "OVR_Log.h"

namespace OVR {

#ifdef OVR_CC_ARM
void* ReturnArg0(void* p)
{
    return p;
}
#endif

// ***** Reference Count Base implementation

RefCountImplCore::~RefCountImplCore()
{
    // RefCount can be either 1 or 0 here.
    //  0 if Release() was properly called.
    //  1 if the object was declared on stack or as an aggregate.
    OVR_ASSERT(RefCount <= 1);
}

#ifdef OVR_BUILD_DEBUG
void RefCountImplCore::reportInvalidDelete(void *pmem)
{
    OVR_DEBUG_LOG(
        ("Invalid delete call on ref-counted object at %p. Please use Release()", pmem));
    OVR_ASSERT(0);
}
#endif

RefCountNTSImplCore::~RefCountNTSImplCore()
{
    // RefCount can be either 1 or 0 here.
    //  0 if Release() was properly called.
    //  1 if the object was declared on stack or as an aggregate.
    OVR_ASSERT(RefCount <= 1);
}

#ifdef OVR_BUILD_DEBUG
void RefCountNTSImplCore::reportInvalidDelete(void *pmem)
{
    OVR_DEBUG_LOG(
        ("Invalid delete call on ref-counted object at %p. Please use Release()", pmem));
    OVR_ASSERT(0);
}
#endif


// *** Thread-Safe RefCountImpl

void    RefCountImpl::AddRef()
{
    AtomicOps<int>::ExchangeAdd_NoSync(&RefCount, 1);
}
void    RefCountImpl::Release()
{
    if ((AtomicOps<int>::ExchangeAdd_NoSync(&RefCount, -1) - 1) == 0)
        delete this;
}

// *** Thread-Safe RefCountVImpl w/virtual AddRef/Release

void    RefCountVImpl::AddRef()
{
    AtomicOps<int>::ExchangeAdd_NoSync(&RefCount, 1);
}
void    RefCountVImpl::Release()
{
    if ((AtomicOps<int>::ExchangeAdd_NoSync(&RefCount, -1) - 1) == 0)
        delete this;
}

// *** NON-Thread-Safe RefCountImpl

void    RefCountNTSImpl::Release() const
{
    RefCount--;
    if (RefCount == 0)
        delete this;
}


} // OVR
