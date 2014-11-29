/************************************************************************************

PublicHeader:   OVR.h
Filename    :   OVR_HIDDeviceBase.h
Content     :   Definition of HID device interface.
Created     :   March 11, 2013
Authors     :   Lee Cooper

Copyright   :   Copyright 2013 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#ifndef OVR_HIDDeviceBase_h
#define OVR_HIDDeviceBase_h

#include "Kernel/OVR_Types.h"

namespace OVR {

//-------------------------------------------------------------------------------------
// ***** HIDDeviceBase

// Base interface for HID devices.
class HIDDeviceBase
{
public:

    virtual ~HIDDeviceBase() { }

    virtual bool SetFeatureReport(UByte* data, UInt32 length) = 0;
    virtual bool GetFeatureReport(UByte* data, UInt32 length) = 0;
};

} // namespace OVR

#endif
