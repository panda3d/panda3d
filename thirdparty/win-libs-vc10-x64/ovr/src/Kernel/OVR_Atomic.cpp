/************************************************************************************

Filename    :   OVR_Atomic.cpp
Content     :   Contains atomic operations and inline fastest locking
                functionality. Will contain #ifdefs for OS efficiency.
                Have non-thread-safe implementation if not available.
Created     :   September 19, 2012
Notes       : 

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

************************************************************************************/

#include "OVR_Atomic.h"

#ifdef OVR_ENABLE_THREADS

// Include Windows 8-Metro compatible Synchronization API
#if defined(OVR_OS_WIN32) && defined(NTDDI_WIN8) && (NTDDI_VERSION >= NTDDI_WIN8)
#include <synchapi.h>
#endif


namespace OVR {

// ***** Windows Lock implementation

#if defined(OVR_OS_WIN32)

// ***** Standard Win32 Lock implementation

// Constructors
Lock::Lock(unsigned spinCount)
{
#if defined(NTDDI_WIN8) && (NTDDI_VERSION >= NTDDI_WIN8)
   // On Windows 8 we use InitializeCriticalSectionEx due to Metro-Compatibility
   InitializeCriticalSectionEx(&cs, spinCount,
                               OVR_DEBUG_SELECT(NULL, CRITICAL_SECTION_NO_DEBUG_INFO));
#else
    // Spin count init critical section function prototype for Window NT
    typedef BOOL (WINAPI *Function_InitializeCriticalSectionAndSpinCount) 
                 (LPCRITICAL_SECTION lpCriticalSection, DWORD dwSpinCount);


    // Try to load function dynamically so that we don't require NT
    // On Windows NT we will use InitializeCriticalSectionAndSpinCount
    static  bool initTried = 0;
    static  Function_InitializeCriticalSectionAndSpinCount pInitFn = 0;

    if (!initTried)
    {
        HMODULE hmodule = ::LoadLibrary(OVR_STR("kernel32.dll"));
        pInitFn     = (Function_InitializeCriticalSectionAndSpinCount)
                      ::GetProcAddress(hmodule, "InitializeCriticalSectionAndSpinCount");
        initTried   = true;
    }

    // Initialize the critical section
    if (pInitFn)
        pInitFn(&cs, spinCount);
    else
        ::InitializeCriticalSection(&cs);
#endif
   
}


Lock::~Lock()
{
    DeleteCriticalSection(&cs);
}


#endif

} // OVR

#endif // OVR_ENABLE_THREADS
