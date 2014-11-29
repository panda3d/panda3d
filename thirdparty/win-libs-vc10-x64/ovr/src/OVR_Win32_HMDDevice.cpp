/************************************************************************************

Filename    :   OVR_Win32_HMDDevice.cpp
Content     :   Win32 Interface to HMD - detects HMD display
Created     :   September 21, 2012
Authors     :   Michael Antonov

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#include "OVR_Win32_HMDDevice.h"

#include "OVR_Win32_DeviceManager.h"

#include <tchar.h>

namespace OVR { namespace Win32 {

//-------------------------------------------------------------------------------------

HMDDeviceCreateDesc::HMDDeviceCreateDesc(DeviceFactory* factory, 
                                         const String& deviceId, const String& displayDeviceName)
        : DeviceCreateDesc(factory, Device_HMD),
          DeviceId(deviceId), DisplayDeviceName(displayDeviceName),
          DesktopX(0), DesktopY(0), Contents(0),
          HResolution(0), VResolution(0), HScreenSize(0), VScreenSize(0)
{
}
HMDDeviceCreateDesc::HMDDeviceCreateDesc(const HMDDeviceCreateDesc& other)
        : DeviceCreateDesc(other.pFactory, Device_HMD),
          DeviceId(other.DeviceId), DisplayDeviceName(other.DisplayDeviceName),
          DesktopX(other.DesktopX), DesktopY(other.DesktopY), Contents(other.Contents),
          HResolution(other.HResolution), VResolution(other.VResolution),
          HScreenSize(other.HScreenSize), VScreenSize(other.VScreenSize)
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
        (DisplayDeviceName == s2.DisplayDeviceName))
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
    
    // SensorDisplayInfo may override resolution settings, so store as candidate.
    if (s2.DeviceId.IsEmpty())
    {        
        *pcandidate = const_cast<DeviceCreateDesc*>((const DeviceCreateDesc*)this);
        return Match_Candidate;
    }
    // OTHER HMD Monitor desc may initialize DeviceName/Id
    else if (DeviceId.IsEmpty())
    {
        *pcandidate = const_cast<DeviceCreateDesc*>((const DeviceCreateDesc*)this);
        return Match_Candidate;
    }
    
    return Match_None;
}


bool HMDDeviceCreateDesc::UpdateMatchedCandidate(const DeviceCreateDesc& other, 
                                                 bool* newDeviceFlag)
{
    // This candidate was the the "best fit" to apply sensor DisplayInfo to.
    OVR_ASSERT(other.Type == Device_HMD);
    
    const HMDDeviceCreateDesc& s2 = (const HMDDeviceCreateDesc&) other;

    // Force screen size on resolution from SensorDisplayInfo.
    // We do this because USB detection is more reliable as compared to HDMI EDID,
    // which may be corrupted by splitter reporting wrong monitor 
    if (s2.DeviceId.IsEmpty())
    {
        HScreenSize = s2.HScreenSize;
        VScreenSize = s2.VScreenSize;
        Contents |= Contents_Screen;

        if (s2.Contents & HMDDeviceCreateDesc::Contents_Distortion)
        {
            memcpy(DistortionK, s2.DistortionK, sizeof(float)*4);
            Contents |= Contents_Distortion;
        }
        DeviceId          = s2.DeviceId;
        DisplayDeviceName = s2.DisplayDeviceName;
        DesktopX          = s2.DesktopX;
        DesktopY          = s2.DesktopY;
        if (newDeviceFlag) *newDeviceFlag = true;
    }
    else if (DeviceId.IsEmpty())
    {
        DeviceId          = s2.DeviceId;
        DisplayDeviceName = s2.DisplayDeviceName;
        DesktopX          = s2.DesktopX;
        DesktopY          = s2.DesktopY;

		// ScreenSize and Resolution are NOT assigned here, since they may have
		// come from a sensor DisplayInfo (which has precedence over HDMI).

        if (newDeviceFlag) *newDeviceFlag = true;
    }
    else
    {
        if (newDeviceFlag) *newDeviceFlag = false;
    }

    return true;
}

bool HMDDeviceCreateDesc::MatchDevice(const String& path)
{
    return DeviceId.CompareNoCase(path) == 0;
}
    
//-------------------------------------------------------------------------------------


const wchar_t* FormatDisplayStateFlags(wchar_t* buff, int length, DWORD flags)
{
    buff[0] = 0;
    if (flags & DISPLAY_DEVICE_ACTIVE)
        wcscat_s(buff, length, L"Active ");
    if (flags & DISPLAY_DEVICE_MIRRORING_DRIVER)
        wcscat_s(buff, length, L"Mirroring_Driver ");
    if (flags & DISPLAY_DEVICE_MODESPRUNED)
        wcscat_s(buff, length, L"ModesPruned ");
    if (flags & DISPLAY_DEVICE_PRIMARY_DEVICE)
        wcscat_s(buff, length, L"Primary ");
    if (flags & DISPLAY_DEVICE_REMOVABLE)
        wcscat_s(buff, length, L"Removable ");
    if (flags & DISPLAY_DEVICE_VGA_COMPATIBLE)
        wcscat_s(buff, length, L"VGA_Compatible ");
    return buff;
}


//-------------------------------------------------------------------------------------
// Callback for monitor enumeration to store all the monitor handles

// Used to capture all the active monitor handles
struct MonitorSet
{
    enum { MaxMonitors = 8 };
    HMONITOR Monitors[MaxMonitors];
    int      MonitorCount;
};

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC, LPRECT, LPARAM dwData)
{
    MonitorSet* monitorSet = (MonitorSet*)dwData;
    if (monitorSet->MonitorCount > MonitorSet::MaxMonitors)
        return FALSE;

    monitorSet->Monitors[monitorSet->MonitorCount] = hMonitor;
    monitorSet->MonitorCount++;
    return TRUE;
};

//-------------------------------------------------------------------------------------
// ***** HMDDeviceFactory

HMDDeviceFactory HMDDeviceFactory::Instance;

void HMDDeviceFactory::EnumerateDevices(EnumerateVisitor& visitor)
{
    MonitorSet monitors;
    monitors.MonitorCount = 0;
    // Get all the monitor handles 
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&monitors);

    bool foundHMD = false;
    
   // DeviceManager* manager = getManager();
    DISPLAY_DEVICE dd, ddm;
    UINT           i, j;    

    for (i = 0; 
        (ZeroMemory(&dd, sizeof(dd)), dd.cb = sizeof(dd),
        EnumDisplayDevices(0, i, &dd, 0)) != 0;  i++)
    {
        
        /*
        wchar_t buff[500], flagsBuff[200];
        
        swprintf_s(buff, 500, L"\nDEV: \"%s\" \"%s\" 0x%08x=%s\n     \"%s\" \"%s\"\n",
            dd.DeviceName, dd.DeviceString,
            dd.StateFlags, FormatDisplayStateFlags(flagsBuff, 200, dd.StateFlags),
            dd.DeviceID, dd.DeviceKey);
        ::OutputDebugString(buff);
        */

        for (j = 0; 
            (ZeroMemory(&ddm, sizeof(ddm)), ddm.cb = sizeof(ddm),
            EnumDisplayDevices(dd.DeviceName, j, &ddm, 0)) != 0;  j++)
        {
            /*
            wchar_t mbuff[500];
            swprintf_s(mbuff, 500, L"MON: \"%s\" \"%s\" 0x%08x=%s\n     \"%s\" \"%s\"\n",
                ddm.DeviceName, ddm.DeviceString,
                ddm.StateFlags, FormatDisplayStateFlags(flagsBuff, 200, ddm.StateFlags),
                ddm.DeviceID, ddm.DeviceKey);
            ::OutputDebugString(mbuff);
            */

            // Our monitor hardware has string "RTD2205" in it
            // Nate's device "CVT0003"
            if (wcsstr(ddm.DeviceID, L"RTD2205") || 
                wcsstr(ddm.DeviceID, L"CVT0003") || 
                wcsstr(ddm.DeviceID, L"MST0030") ||
                wcsstr(ddm.DeviceID, L"OVR00") ) // Part of Oculus EDID.
            {
                String deviceId(ddm.DeviceID);
                String displayDeviceName(ddm.DeviceName);

                // The default monitor coordinates
                int mx      = 0;
                int my      = 0;
                int mwidth  = 1280;
                int mheight = 800;

                // Find the matching MONITORINFOEX for this device so we can get the 
                // screen coordinates
                MONITORINFOEX info;
                for (int m=0; m < monitors.MonitorCount; m++)
                {
                    info.cbSize = sizeof(MONITORINFOEX);
                    GetMonitorInfo(monitors.Monitors[m], &info);
                    if (_tcsstr(ddm.DeviceName, info.szDevice) == ddm.DeviceName)
                    {   // If the device name starts with the monitor name
                        // then we found the matching DISPLAY_DEVICE and MONITORINFO
                        // so we can gather the monitor coordinates
                        mx = info.rcMonitor.left;
                        my = info.rcMonitor.top;
                        //mwidth = info.rcMonitor.right - info.rcMonitor.left;
                        //mheight = info.rcMonitor.bottom - info.rcMonitor.top;
                        break;
                    }
                }

                HMDDeviceCreateDesc hmdCreateDesc(this, deviceId, displayDeviceName);
				
				if (wcsstr(ddm.DeviceID, L"OVR0002"))
				{
					hmdCreateDesc.SetScreenParameters(mx, my, 1920, 1080, 0.12096f, 0.06804f);
				}
				else
				{
					if (hmdCreateDesc.Is7Inch())
					{
						// Physical dimension of SLA screen.
						hmdCreateDesc.SetScreenParameters(mx, my, mwidth, mheight, 0.14976f, 0.0936f);
					}
					else
					{
						hmdCreateDesc.SetScreenParameters(mx, my, mwidth, mheight, 0.12096f, 0.0756f);
					}
				}


                OVR_DEBUG_LOG_TEXT(("DeviceManager - HMD Found %s - %s\n",
                                    deviceId.ToCStr(), displayDeviceName.ToCStr()));

                // Notify caller about detected device. This will call EnumerateAddDevice
                // if the this is the first time device was detected.
                visitor.Visit(hmdCreateDesc);
                foundHMD = true;
                break;
            }
        }
    }

    // Real HMD device is not found; however, we still may have a 'fake' HMD
    // device created via SensorDeviceImpl::EnumerateHMDFromSensorDisplayInfo.
    // Need to find it and set 'Enumerated' to true to avoid Removal notification.
    if (!foundHMD)
    {
        Ptr<DeviceCreateDesc> hmdDevDesc = getManager()->FindDevice("", Device_HMD);
        if (hmdDevDesc)
            hmdDevDesc->Enumerated = true;
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
                hmdInfo->DistortionK[0]      = 1.0f;
                hmdInfo->DistortionK[1]      = 0.22f;
                hmdInfo->DistortionK[2]      = 0.24f;
                hmdInfo->EyeToScreenDistance = 0.041f;
            }
            else
            {
                hmdInfo->DistortionK[0]      = 1.0f;
                hmdInfo->DistortionK[1]      = 0.18f;
                hmdInfo->DistortionK[2]      = 0.115f;

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

}} // namespace OVR::Win32


