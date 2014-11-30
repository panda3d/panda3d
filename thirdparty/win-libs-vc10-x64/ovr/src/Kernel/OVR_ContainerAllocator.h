/************************************************************************************

PublicHeader:   OVR.h
Filename    :   OVR_ContainerAllocator.h
Content     :   Template allocators and constructors for containers.
Created     :   September 19, 2012
Notes       : 

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

************************************************************************************/

#ifndef OVR_ContainerAllocator_h
#define OVR_ContainerAllocator_h

#include "OVR_Allocator.h"
#include <string.h>


namespace OVR {


//-----------------------------------------------------------------------------------
// ***** Container Allocator

// ContainerAllocator serves as a template argument for allocations done by
// containers, such as Array and Hash; replacing it could allow allocator
// substitution in containers.

class ContainerAllocatorBase
{
public:
    static void* Alloc(UPInt size)                { return OVR_ALLOC(size); }
    static void* Realloc(void* p, UPInt newSize)  { return OVR_REALLOC(p, newSize); }
    static void  Free(void *p)                    { OVR_FREE(p); }
};



//-----------------------------------------------------------------------------------
// ***** Constructors, Destructors, Copiers

// Plain Old Data - movable, no special constructors/destructor.
template<class T> 
class ConstructorPOD
{
public:
    static void Construct(void *) {}
    static void Construct(void *p, const T& source) 
    { 
        *(T*)p = source;
    }

    // Same as above, but allows for a different type of constructor.
    template <class S> 
    static void ConstructAlt(void *p, const S& source)
    {
        *(T*)p = source;
    }

    static void ConstructArray(void*, UPInt) {}

    static void ConstructArray(void* p, UPInt count, const T& source)
    {
        UByte *pdata = (UByte*)p;
        for (UPInt i=0; i< count; ++i, pdata += sizeof(T))
            *(T*)pdata = source;
    }

    static void ConstructArray(void* p, UPInt count, const T* psource)
    {
        memcpy(p, psource, sizeof(T) * count);
    }

    static void Destruct(T*) {}
    static void DestructArray(T*, UPInt) {}

    static void CopyArrayForward(T* dst, const T* src, UPInt count)
    {
        memmove(dst, src, count * sizeof(T));
    }

    static void CopyArrayBackward(T* dst, const T* src, UPInt count)
    {
        memmove(dst, src, count * sizeof(T));
    }

    static bool IsMovable() { return true; }
};


//-----------------------------------------------------------------------------------
// ***** ConstructorMov
//
// Correct C++ construction and destruction for movable objects
template<class T> 
class ConstructorMov
{
public:
    static void Construct(void* p) 
    { 
        OVR::Construct<T>(p);
    }

    static void Construct(void* p, const T& source) 
    { 
        OVR::Construct<T>(p, source);
    }

    // Same as above, but allows for a different type of constructor.
    template <class S> 
    static void ConstructAlt(void* p, const S& source)
    {
        OVR::ConstructAlt<T,S>(p, source);
    }

    static void ConstructArray(void* p, UPInt count)
    {
        UByte* pdata = (UByte*)p;
        for (UPInt i=0; i< count; ++i, pdata += sizeof(T))
            Construct(pdata);
    }

    static void ConstructArray(void* p, UPInt count, const T& source)
    {
        UByte* pdata = (UByte*)p;
        for (UPInt i=0; i< count; ++i, pdata += sizeof(T))
            Construct(pdata, source);
    }

    static void ConstructArray(void* p, UPInt count, const T* psource)
    {
        UByte* pdata = (UByte*)p;
        for (UPInt i=0; i< count; ++i, pdata += sizeof(T))
            Construct(pdata, *psource++);
    }

    static void Destruct(T* p)
    {
        p->~T();
        OVR_UNUSED(p); // Suppress silly MSVC warning
    }

    static void DestructArray(T* p, UPInt count)
    {   
        p += count - 1;
        for (UPInt i=0; i<count; ++i, --p)
            p->~T();
    }

    static void CopyArrayForward(T* dst, const T* src, UPInt count)
    {
        memmove(dst, src, count * sizeof(T));
    }

    static void CopyArrayBackward(T* dst, const T* src, UPInt count)
    {
        memmove(dst, src, count * sizeof(T));
    }

    static bool IsMovable() { return true; }
};


//-----------------------------------------------------------------------------------
// ***** ConstructorCPP
//
// Correct C++ construction and destruction for movable objects
template<class T> 
class ConstructorCPP
{
public:
    static void Construct(void* p) 
    { 
        OVR::Construct<T>(p);        
    }

    static void Construct(void* p, const T& source) 
    { 
        OVR::Construct<T>(p, source);        
    }

    // Same as above, but allows for a different type of constructor.
    template <class S> 
    static void ConstructAlt(void* p, const S& source)
    {
        OVR::ConstructAlt<T,S>(p, source);        
    }

    static void ConstructArray(void* p, UPInt count)
    {
        UByte* pdata = (UByte*)p;
        for (UPInt i=0; i< count; ++i, pdata += sizeof(T))
            Construct(pdata);
    }

    static void ConstructArray(void* p, UPInt count, const T& source)
    {
        UByte* pdata = (UByte*)p;
        for (UPInt i=0; i< count; ++i, pdata += sizeof(T))
            Construct(pdata, source);
    }

    static void ConstructArray(void* p, UPInt count, const T* psource)
    {
        UByte* pdata = (UByte*)p;
        for (UPInt i=0; i< count; ++i, pdata += sizeof(T))
            Construct(pdata, *psource++);
    }

    static void Destruct(T* p)
    {
        p->~T();
        OVR_UNUSED(p); // Suppress silly MSVC warning
    }

    static void DestructArray(T* p, UPInt count)
    {   
        p += count - 1;
        for (UPInt i=0; i<count; ++i, --p)
            p->~T();
    }

    static void CopyArrayForward(T* dst, const T* src, UPInt count)
    {
        for(UPInt i = 0; i < count; ++i)
            dst[i] = src[i];
    }

    static void CopyArrayBackward(T* dst, const T* src, UPInt count)
    {
        for(UPInt i = count; i; --i)
            dst[i-1] = src[i-1];
    }

    static bool IsMovable() { return false; }
};


//-----------------------------------------------------------------------------------
// ***** Container Allocator with movement policy
//
// Simple wraps as specialized allocators
template<class T> struct ContainerAllocator_POD : ContainerAllocatorBase, ConstructorPOD<T> {};
template<class T> struct ContainerAllocator     : ContainerAllocatorBase, ConstructorMov<T> {};
template<class T> struct ContainerAllocator_CPP : ContainerAllocatorBase, ConstructorCPP<T> {};


} // OVR


#endif
