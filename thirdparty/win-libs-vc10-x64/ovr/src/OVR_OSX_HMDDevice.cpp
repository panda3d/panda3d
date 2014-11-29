/************************************************************************************

Filename    :   OVR_OSX_HMDDevice.cpp
Content     :   OSX Interface to HMD - detects HMD display
Created     :   September 21, 2012
Authors     :   Michael Antonov

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#include "OVR_OSX_HMDDevice.h"
#include <CoreGraphics/CGDirectDisplay.h>
#include <CoreGraphics/CGDisplayConfiguration.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFString.h>
#include <IOKit/graphics/IOGraphicsLib.h>

namespace OVR { namespace OSX {

//-------------------------------------------------------------------------------------

HMDDeviceCreateDesc::HMDDeviceCreateDesc(DeviceFactory* factory, 
                                         UInt32 vend, UInt32 prod, const String& displayDeviceName, long dispId)
        : DeviceCreateDesc(factory, Device_HMD),
          DisplayDeviceName(displayDeviceName),
          DesktopX(0), DesktopY(0), Contents(0),
          HResolution(0), VResolution(0), HScreenSize(0), VScreenSize(0),
          DisplayId(dispId)
{
    /* //??????????
    char idstring[9];
    idstring[0] = 'A'-1+((vend>>10) & 31);
    idstring[1] = 'A'-1+((vend>>5) & 31);
    idstring[2] = 'A'-1+((vend>>0) & 31);
    snprintf(idstring+3, 5, "%04d", prod);
    DeviceId = idstring;*/
    DeviceId = DisplayDeviceName;
}

HMDDeviceCreateDesc::HMDDeviceCreateDesc(const HMDDeviceCreateDesc& other)
        : DeviceCreateDesc(other.pFactory, Device_HMD),
          DeviceId(other.DeviceId), DisplayDeviceName(other.DisplayDeviceName),
          DesktopX(other.DesktopX), DesktopY(other.DesktopY), Contents(other.Contents),
          HResolution(other.HResolution), VResolution(other.VResolution),
          HScreenSize(other.HScreenSize), VScreenSize(other.VScreenSize),
          DisplayId(other.DisplayId)
{
}

HMDDeviceCreateDesc::MatchResult HMDDeviceCreateDesc::MatchDevice(const DeviceCreateDesc& other,
                                                                  DeviceCreateDesc** pcandidate) const
{
    if ((other.Type != Device_HMD) || (other.pFactory != pFactory))
        return Match_None;

    // There are several reasons we can come in here:
    //   a) Matching this HMD Monitor created desc to OTHER HMD Monitor desc
    //          - Require exact device DeviceId/DeviceName match
    //   b) Matching SensorDisplayInfo created desc to OTHER HMD Monitor desc
    //          - This DeviceId is empty; becomes candidate
    //   c) Matching this HMD Monitor created desc to SensorDisplayInfo desc
    //          - This other.DeviceId is empty; becomes candidate

    const HMDDeviceCreateDesc& s2 = (const HMDDeviceCreateDesc&) other;

    if ((DeviceId == s2.DeviceId) &&
        (DisplayId == s2.DisplayId))
    {
        // Non-null DeviceId may match while size is different if screen size was overwritten
        // by SensorDisplayInfo in prior iteration.
        if (!DeviceId.IsEmpty() ||
             ((HScreenSize == s2.HScreenSize) &&
              (VScreenSize == s2.VScreenSize)) )
        {            
            *pcandidate = 0;
            return Match_Found;
        }
    }


    // DisplayInfo takes precedence, although we try to match it first.
    if ((HResolution == s2.HResolution) &&
        (VResolution == s2.VResolution) &&
        (HScreenSize == s2.HScreenSize) &&
        (VScreenSize == s2.VScreenSize))
    {
        if (DeviceId.IsEmpty() && !s2.DeviceId.IsEmpty())
        {
            *pcandidate = const_cast<DeviceCreateDesc*>((const DeviceCreateDesc*)this);
            return Match_Candidate;
        }

        *pcandidate = 0;
        return Match_Found;
    }    
    
    // SensorDisplayInfo may override resolution settings, so store as candidiate.
    if (s2.DeviceId.IsEmpty() && s2.DisplayId == 0)
    {        
        *pcandidate = const_cast<DeviceCreateDesc*>((const DeviceCreateDesc*)this);        
        return Match_Candidate;
    }
    // OTHER HMD Monitor desc may initialize DeviceName/Id
    else if (DeviceId.IsEmpty() && DisplayId == 0)
    {
        *pcandidate = const_cast<DeviceCreateDesc*>((const DeviceCreateDesc*)this);        
        return Match_Candidate;
    }
    
    return Match_None;
}


bool HMDDeviceCreateDesc::UpdateMatchedCandidate(const DeviceCreateDesc& other, bool* newDeviceFlag)
{
    // This candidate was the the "best fit" to apply sensor DisplayInfo to.
    OVR_ASSERT(other.Type == Device_HMD);
    
    const HMDDeviceCreateDesc& s2 = (const HMDDeviceCreateDesc&) other;

    // Force screen size on resolution from SensorDisplayInfo.
    // We do this because USB detection is more reliable as compared to HDMI EDID,
    // which may be corrupted by splitter reporting wrong monitor 
    if (s2.DeviceId.IsEmpty() && s2.DisplayId == 0)
    {
        // disconnected HMD: replace old descriptor by the 'fake' one.
        HScreenSize = s2.HScreenSize;
        VScreenSize = s2.VScreenSize;
        Contents |= Contents_Screen;

        if (s2.Contents & HMDDeviceCreateDesc::Contents_Distortion)
        {
            memcpy(DistortionK, s2.DistortionK, sizeof(float)*4);
            Contents |= Contents_Distortion;
        }
        DeviceId          = s2.DeviceId;
        DisplayId         = s2.DisplayId;
        DisplayDeviceName = s2.DisplayDeviceName;
        if (newDeviceFlag) *newDeviceFlag = true;
    }
    else if (DeviceId.IsEmpty())
    {
        // This branch is executed when 'fake' HMD descriptor is being replaced by
        // the real one.
        DeviceId          = s2.DeviceId;
        DisplayId         = s2.DisplayId;
        DisplayDeviceName = s2.DisplayDeviceName;
        if (newDeviceFlag) *newDeviceFlag = true;
    }
    else
    {
        if (newDeviceFlag) *newDeviceFlag = false;
    }

    return true;
}

    
//-------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------
// ***** HMDDeviceFactory

HMDDeviceFactory HMDDeviceFactory::Instance;

void HMDDeviceFactory::EnumerateDevices(EnumerateVisitor& visitor)
{
    CGDirectDisplayID Displays[32];
    uint32_t NDisplays = 0;
    CGGetOnlineDisplayList(32, Displays, &NDisplays);

    for (int i = 0; i < NDisplays; i++)
    {
        io_service_t port = CGDisplayIOServicePort(Displays[i]);
        CFDictionaryRef DispInfo = IODisplayCreateInfoDictionary(port, kIODisplayMatchingInfo);

        uint32_t vendor = CGDisplayVendorNumber(Displays[i]);
        uint32_t product = CGDisplayModelNumber(Displays[i]);
        unsigned mwidth = (unsigned)CGDisplayPixelsWide(Displays[i]);
        unsigned mheight = (unsigned)CGDisplayPixelsHigh(Displays[i]);
        CGRect desktop = CGDisplayBounds(Displays[i]);
        
        if (vendor == 16082 && product == 1)
        {
            char idstring[9];
            idstring[0] = 'A'-1+((vendor>>10) & 31);
            idstring[1] = 'A'-1+((vendor>>5) & 31);
            idstring[2] = 'A'-1+((vendor>>0) & 31);
            snprintf(idstring+3, 5, "%04d", product);

            HMDDeviceCreateDesc hmdCreateDesc(this, vendor, product, idstring, Displays[i]);
            
			if (product == 2)
			{
                hmdCreateDesc.SetScreenParameters(desktop.origin.x, desktop.origin.y,
                                                  mwidth, mheight, 0.12096f, 0.06804f);
			}
			else
			{
				if (hmdCreateDesc.Is7Inch())
				{
					// Physical dimension of SLA screen.
					hmdCreateDesc.SetScreenParameters(desktop.origin.x, desktop.origin.y,
                                                      mwidth, mheight, 0.14976f, 0.0936f);
				}
				else
				{
					hmdCreateDesc.SetScreenParameters(desktop.origin.x, desktop.origin.y,
                                                      mwidth, mheight, 0.12096f, 0.0756f);
				}
			}

            OVR_DEBUG_LOG_TEXT(("DeviceManager - HMD Found %x:%x\n", vendor, product));
            
            // Notify caller about detected device. This will call EnumerateAddDevice
            // if the this is the first time device was detected.
            visitor.Visit(hmdCreateDesc);
        }
        CFRelease(DispInfo);
    }
}

DeviceBase* HMDDeviceCreateDesc::NewDeviceInstance()
{
    return new HMDDevice(this);
}

bool HMDDeviceCreateDesc::Is7Inch() const
{
    return (strstr(DeviceId.ToCStr(), "OVR0001") != 0) || (Contents & Contents_7Inch);
}

Profile* HMDDeviceCreateDesc::GetProfileAddRef() const
{
    // Create device may override profile name, so get it from there is possible.
    ProfileManager* profileManager = GetManagerImpl()->GetProfileManager();
    ProfileType     profileType    = GetProfileType();
    const char *    profileName    = pDevice ?
                        ((HMDDevice*)pDevice)->GetProfileName() :
                        profileManager->GetDefaultProfileName(profileType);

    return profileName ?
        profileManager->LoadProfile(profileType, profileName) :
        profileManager->GetDeviceDefaultProfile(profileType);
}

bool HMDDeviceCreateDesc::GetDeviceInfo(DeviceInfo* info) const
{
    if ((info->InfoClassType != Device_HMD) &&
        (info->InfoClassType != Device_None))
        return false;

    bool is7Inch = Is7Inch();

    OVR_strcpy(info->ProductName,  DeviceInfo::MaxNameLength,
               is7Inch ? "Oculus Rift DK1" :
			   ((HResolution >= 1920) ? "Oculus Rift DK HD" : "Oculus Rift DK1-Prototype") );
    OVR_strcpy(info->Manufacturer, DeviceInfo::MaxNameLength, "Oculus VR");
    info->Type    = Device_HMD;
    info->Version = 0;

    // Display detection.
    if (info->InfoClassType == Device_HMD)
    {
        HMDInfo* hmdInfo = static_cast<HMDInfo*>(info);

        hmdInfo->DesktopX               = DesktopX;
        hmdInfo->DesktopY               = DesktopY;
        hmdInfo->HResolution            = HResolution;
        hmdInfo->VResolution            = VResolution;
        hmdInfo->HScreenSize            = HScreenSize;
        hmdInfo->VScreenSize            = VScreenSize;
        hmdInfo->VScreenCenter          = VScreenSize * 0.5f;
        hmdInfo->InterpupillaryDistance = 0.064f;  // Default IPD; should be configurable.
        hmdInfo->LensSeparationDistance = 0.0635f;

        // Obtain IPD from profile.
        Ptr<Profile> profile = *GetProfileAddRef();

        if (profile)
        {
            hmdInfo->InterpupillaryDistance = profile->GetIPD();
            // TBD: Switch on EyeCup type.
        }
        
        if (Contents & Contents_Distortion)
        {
            memcpy(hmdInfo->DistortionK, DistortionK, sizeof(float)*4);
        }
        else
        {
            if (is7Inch)
            {
                // 7" screen.
                hmdInfo->DistortionK[0]        = 1.0f;
                hmdInfo->DistortionK[1]        = 0.22f;
                hmdInfo->DistortionK[2]        = 0.24f;
                hmdInfo->EyeToScreenDistance   = 0.041f;                
            }
            else
            {
                hmdInfo->DistortionK[0]        = 1.0f;
                hmdInfo->DistortionK[1]        = 0.18f;
                hmdInfo->DistortionK[2]        = 0.115f;

                if (HResolution == 1920)
                    hmdInfo->EyeToScreenDistance = 0.040f;
                else
                    hmdInfo->EyeToScreenDistance = 0.0387f;
            }

            hmdInfo->ChromaAbCorrection[0] = 0.996f;
            hmdInfo->ChromaAbCorrection[1] = -0.004f;
            hmdInfo->ChromaAbCorrection[2] = 1.014f;
            hmdInfo->ChromaAbCorrection[3] = 0.0f;
        }

        OVR_strcpy(hmdInfo->DisplayDeviceName, sizeof(hmdInfo->DisplayDeviceName),
                   DisplayDeviceName.ToCStr());
        hmdInfo->DisplayId = DisplayId;
    }

    return true;
}

//-------------------------------------------------------------------------------------
// ***** HMDDevice

HMDDevice::HMDDevice(HMDDeviceCreateDesc* createDesc)
    : OVR::DeviceImpl<OVR::HMDDevice>(createDesc, 0)
{
}
HMDDevice::~HMDDevice()
{
}

bool HMDDevice::Initialize(DeviceBase* parent)
{
    pParent = parent;

    // Initialize user profile to default for device.
    ProfileManager* profileManager = GetManager()->GetProfileManager();    
    ProfileName = profileManager->GetDefaultProfileName(getDesc()->GetProfileType());

    return true;
}
void HMDDevice::Shutdown()
{
    ProfileName.Clear();
    pCachedProfile.Clear();
    pParent.Clear();
}

Profile* HMDDevice::GetProfile() const
{    
    if (!pCachedProfile)
        pCachedProfile = *getDesc()->GetProfileAddRef();
    return pCachedProfile.GetPtr();
}

const char* HMDDevice::GetProfileName() const
{
    return ProfileName.ToCStr();
}

bool HMDDevice::SetProfileName(const char* name)
{
    pCachedProfile.Clear();
    if (!name)
    {
        ProfileName.Clear();
        return 0;
    }
    if (GetManager()->GetProfileManager()->HasProfile(getDesc()->GetProfileType(), name))
    {
        ProfileName = name;
        return true;
    }
    return false;
}

OVR::SensorDevice* HMDDevice::GetSensor()
{
    // Just return first sensor found since we have no way to match it yet.
    OVR::SensorDevice* sensor = GetManager()->EnumerateDevices<SensorDevice>().CreateDevice();
    if (sensor)
        sensor->SetCoordinateFrame(SensorDevice::Coord_HMD);
    return sensor;
}


}} // namespace OVR::OSX


