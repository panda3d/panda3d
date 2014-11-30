/************************************************************************************

Filename    :   OVR_Linux_HMDDevice.h
Content     :   Linux HMDDevice implementation
Created     :   June 17, 2013
Authors     :   Brant Lewis

Copyright   :   Copyright 2013 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#ifndef OVR_Linux_HMDDevice_h
#define OVR_Linux_HMDDevice_h

#include "OVR_Linux_DeviceManager.h"
#include "OVR_Profile.h"

namespace OVR { namespace Linux {

class HMDDevice;

//-------------------------------------------------------------------------------------

// HMDDeviceFactory enumerates attached Oculus HMD devices.
//
// This is currently done by matching monitor device strings.

class HMDDeviceFactory : public DeviceFactory
{
public:
    static HMDDeviceFactory Instance;

    // Enumerates devices, creating and destroying relevant objects in manager.
    virtual void EnumerateDevices(EnumerateVisitor& visitor);

protected:
    DeviceManager* getManager() const { return (DeviceManager*) pManager; }
};


class HMDDeviceCreateDesc : public DeviceCreateDesc
{
    friend class HMDDevice;

protected:
    enum
    {
        Contents_Screen     = 1,
        Contents_Distortion = 2,
        Contents_7Inch      = 4,
    };
    String      DeviceId;
    String      DisplayDeviceName;
    int         DesktopX, DesktopY;
    unsigned    Contents;
    unsigned    HResolution, VResolution;
    float       HScreenSize, VScreenSize;
    long        DisplayId;
    float       DistortionK[4];

public:
    HMDDeviceCreateDesc(DeviceFactory* factory, const String& displayDeviceName, long dispId);
    HMDDeviceCreateDesc(const HMDDeviceCreateDesc& other);

    virtual DeviceCreateDesc* Clone() const
    {
        return new HMDDeviceCreateDesc(*this);
    }

    virtual DeviceBase* NewDeviceInstance();

    virtual MatchResult MatchDevice(const DeviceCreateDesc& other,
                                    DeviceCreateDesc**) const;

    // Matches device by path.
    virtual bool MatchDevice(const String& path);

    virtual bool UpdateMatchedCandidate(const DeviceCreateDesc&, bool* newDeviceFlag = NULL);

    virtual bool GetDeviceInfo(DeviceInfo* info) const;

    // Requests the currently used default profile. This profile affects the
    // settings reported by HMDInfo. 
    Profile* GetProfileAddRef() const;

    ProfileType GetProfileType() const
    {
        return (HResolution >= 1920) ? Profile_RiftDKHD : Profile_RiftDK1;
    }


    void  SetScreenParameters(int x, int y, unsigned hres, unsigned vres, float hsize, float vsize)
    {
        DesktopX = x;
        DesktopY = y;
        HResolution = hres;
        VResolution = vres;
        HScreenSize = hsize;
        VScreenSize = vsize;
        Contents |= Contents_Screen;
    }
    void SetDistortion(const float* dks)
    {
        for (int i = 0; i < 4; i++)
            DistortionK[i] = dks[i];
        Contents |= Contents_Distortion;
    }

    void Set7Inch() { Contents |= Contents_7Inch; }

    bool Is7Inch() const;
};


//-------------------------------------------------------------------------------------

// HMDDevice represents an Oculus HMD device unit. An instance of this class
// is typically created from the DeviceManager.
//  After HMD device is created, we its sensor data can be obtained by 
//  first creating a Sensor object and then wrappig it in SensorFusion.

class HMDDevice : public DeviceImpl<OVR::HMDDevice>
{
public:
    HMDDevice(HMDDeviceCreateDesc* createDesc);
    ~HMDDevice();    

    virtual bool Initialize(DeviceBase* parent);
    virtual void Shutdown();

    // Requests the currently used default profile. This profile affects the
    // settings reported by HMDInfo. 
    virtual Profile*    GetProfile() const;
    virtual const char* GetProfileName() const;
    virtual bool        SetProfileName(const char* name);

    // Query associated sensor.
    virtual OVR::SensorDevice* GetSensor();  

protected:
    HMDDeviceCreateDesc* getDesc() const { return (HMDDeviceCreateDesc*)pCreateDesc.GetPtr(); }

    // User name for the profile used with this device.
    String               ProfileName;
    mutable Ptr<Profile> pCachedProfile;
};


}} // namespace OVR::Linux

#endif // OVR_Linux_HMDDevice_h

