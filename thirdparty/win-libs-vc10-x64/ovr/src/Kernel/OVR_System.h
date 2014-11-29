/************************************************************************************

PublicHeader:   OVR
Filename    :   OVR_System.h
Content     :   General kernel initialization/cleanup, including that
                of the memory allocator.
Created     :   September 19, 2012
Notes       : 

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

************************************************************************************/

#ifndef OVR_System_h
#define OVR_System_h

#include "OVR_Allocator.h"
#include "OVR_Log.h"

namespace OVR {

// ***** System Core Initialization class

// System initialization must take place before any other OVR_Kernel objects are used;
// this is done my calling System::Init(). Among other things, this is necessary to
// initialize the memory allocator. Similarly, System::Destroy must be
// called before program exist for proper cleanup. Both of these tasks can be achieved by
// simply creating System object first, allowing its constructor/destructor do the work.

// TBD: Require additional System class for Oculus Rift API?

class System
{
public:

    // System constructor expects allocator to be specified, if it is being substituted.
    System(Log* log = Log::ConfigureDefaultLog(LogMask_Debug),
           Allocator* palloc = DefaultAllocator::InitSystemSingleton())
    {
        Init(log, palloc);
    }

    ~System()
    {
        Destroy();
    }

    // Returns 'true' if system was properly initialized.
    static bool OVR_CDECL IsInitialized();

    // Initializes System core.  Users can override memory implementation by passing
    // a different Allocator here.
    static void OVR_CDECL Init(Log* log = Log::ConfigureDefaultLog(LogMask_Debug),
                               Allocator *palloc = DefaultAllocator::InitSystemSingleton());

    // De-initializes System more, finalizing the threading system and destroying
    // the global memory allocator.
    static void OVR_CDECL Destroy();    
};

} // OVR

#endif
