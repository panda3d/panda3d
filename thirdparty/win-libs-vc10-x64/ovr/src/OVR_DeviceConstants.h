/************************************************************************************

PublicHeader:   OVR.h
Filename    :   OVR_DeviceConstants.h
Content     :   Device constants
Created     :   February 5, 2013
Authors     :   Lee Cooper

Copyright   :   Copyright 2013 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#ifndef OVR_DeviceConstants_h
#define OVR_DeviceConstants_h

namespace OVR {


//-------------------------------------------------------------------------------------
// Different device types supported by OVR; this type is reported by DeviceBase::GetType.
// 
enum DeviceType
{
    Device_None             = 0,
    Device_Manager          = 1,
    Device_HMD              = 2,
    Device_Sensor           = 3,
    Device_LatencyTester    = 4,
    Device_All              = 0xFF // Set for enumeration only, to enumerate all device types.
};

} // namespace OVR

#endif
