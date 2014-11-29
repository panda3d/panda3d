/************************************************************************************

Filename    :   OVR_SensorImpl.h
Content     :   Sensor device specific implementation.
Created     :   March 7, 2013
Authors     :   Lee Cooper

Copyright   :   Copyright 2013 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#ifndef OVR_SensorImpl_h
#define OVR_SensorImpl_h

#include "OVR_HIDDeviceImpl.h"

namespace OVR {
    
struct TrackerMessage;
class ExternalVisitor;

//-------------------------------------------------------------------------------------
// SensorDeviceFactory enumerates Oculus Sensor devices.
class SensorDeviceFactory : public DeviceFactory
{
public:
    static SensorDeviceFactory Instance;

    // Enumerates devices, creating and destroying relevant objects in manager.
    virtual void EnumerateDevices(EnumerateVisitor& visitor);

    virtual bool MatchVendorProduct(UInt16 vendorId, UInt16 productId) const;
    virtual bool DetectHIDDevice(DeviceManager* pdevMgr, const HIDDeviceDesc& desc);
protected:
    DeviceManager* getManager() const { return (DeviceManager*) pManager; }   
};


// Describes a single a Oculus Sensor device and supports creating its instance.
class SensorDeviceCreateDesc : public HIDDeviceCreateDesc
{
public:
    SensorDeviceCreateDesc(DeviceFactory* factory, const HIDDeviceDesc& hidDesc)
        : HIDDeviceCreateDesc(factory, Device_Sensor, hidDesc) { }
    
    virtual DeviceCreateDesc* Clone() const
    {
        return new SensorDeviceCreateDesc(*this);
    }

    virtual DeviceBase* NewDeviceInstance();

    virtual MatchResult MatchDevice(const DeviceCreateDesc& other,
                                    DeviceCreateDesc**) const
    {
        if ((other.Type == Device_Sensor) && (pFactory == other.pFactory))
        {
            const SensorDeviceCreateDesc& s2 = (const SensorDeviceCreateDesc&) other;
            if (MatchHIDDevice(s2.HIDDesc))
                return Match_Found;
        }
        return Match_None;
    }

    virtual bool MatchHIDDevice(const HIDDeviceDesc& hidDesc) const
    {
        // should paths comparison be case insensitive?
        return ((HIDDesc.Path.CompareNoCase(hidDesc.Path) == 0) &&
                (HIDDesc.SerialNumber == hidDesc.SerialNumber));
    }

    virtual bool        GetDeviceInfo(DeviceInfo* info) const;
};


//-------------------------------------------------------------------------------------
// ***** OVR::SensorDisplayInfoImpl

// DisplayInfo obtained from sensor; these values are used to report distortion
// settings and other coefficients.
// Older SensorDisplayInfo will have all zeros, causing the library to apply hard-coded defaults.
// Currently, only resolutions and sizes are used.
struct SensorDisplayInfoImpl
{
    enum  { PacketSize = 56 };
    UByte   Buffer[PacketSize];

    enum
    {
        Mask_BaseFmt    = 0x0f,
        Mask_OptionFmts = 0xf0,
        Base_None       = 0,
        Base_ScreenOnly = 1,
        Base_Distortion = 2,
    };

    UInt16  CommandId;
    UByte   DistortionType;    
    UInt16  HResolution, VResolution;
    float   HScreenSize, VScreenSize;
    float   VCenter;
    float   LensSeparation;
    float   EyeToScreenDistance[2];
    float   DistortionK[6];

    SensorDisplayInfoImpl();

    void Unpack();
};


//-------------------------------------------------------------------------------------
// ***** OVR::SensorDeviceImpl

// Oculus Sensor interface.

class SensorDeviceImpl : public HIDDeviceImpl<OVR::SensorDevice>
{
public:
     SensorDeviceImpl(SensorDeviceCreateDesc* createDesc);
    ~SensorDeviceImpl();


    // DeviceCommaon interface
    virtual bool Initialize(DeviceBase* parent);
    virtual void Shutdown();
    
    virtual void SetMessageHandler(MessageHandler* handler);

    // HIDDevice::Notifier interface.
    virtual void OnInputReport(UByte* pData, UInt32 length);
    virtual UInt64 OnTicks(UInt64 ticksMks);

    // HMD-Mounted sensor has a different coordinate frame.
    virtual void SetCoordinateFrame(CoordinateFrame coordframe);    
    virtual CoordinateFrame GetCoordinateFrame() const;    

    // SensorDevice interface
    virtual bool SetRange(const SensorRange& range, bool waitFlag);
    virtual void GetRange(SensorRange* range) const;

    // Sets report rate (in Hz) of MessageBodyFrame messages (delivered through MessageHandler::OnMessage call). 
    // Currently supported maximum rate is 1000Hz. If the rate is set to 500 or 333 Hz then OnMessage will be 
    // called twice or thrice at the same 'tick'. 
    // If the rate is  < 333 then the OnMessage / MessageBodyFrame will be called three
    // times for each 'tick': the first call will contain averaged values, the second
    // and third calls will provide with most recent two recorded samples.
    virtual void        SetReportRate(unsigned rateHz);
    // Returns currently set report rate, in Hz. If 0 - error occurred.
    // Note, this value may be different from the one provided for SetReportRate. The return
    // value will contain the actual rate.
    virtual unsigned    GetReportRate() const;

    // Hack to create HMD device from sensor display info.
    static void EnumerateHMDFromSensorDisplayInfo(const SensorDisplayInfoImpl& displayInfo, 
                                                  DeviceFactory::EnumerateVisitor& visitor);
protected:

    void openDevice();
    void closeDeviceOnError();

    Void    setCoordinateFrame(CoordinateFrame coordframe);
    bool    setRange(const SensorRange& range);

    Void    setReportRate(unsigned rateHz);

    // Called for decoded messages
    void        onTrackerMessage(TrackerMessage* message);

    // Helpers to reduce casting.
/*
    SensorDeviceCreateDesc* getCreateDesc() const
    { return (SensorDeviceCreateDesc*)pCreateDesc.GetPtr(); }

    HIDDeviceDesc* getHIDDesc() const
    { return &getCreateDesc()->HIDDesc; }    
*/

    // Set if the sensor is located on the HMD.
    // Older prototype firmware doesn't support changing HW coordinates,
    // so we track its state.
    CoordinateFrame Coordinates;
    CoordinateFrame HWCoordinates;
    UInt64      NextKeepAliveTicks;

    bool        SequenceValid;
    SInt16      LastTimestamp;
    UByte       LastSampleCount;
    float       LastTemperature;
    Vector3f    LastAcceleration;
    Vector3f    LastRotationRate;
    Vector3f    LastMagneticField;

    // Current sensor range obtained from device. 
    SensorRange MaxValidRange;
    SensorRange CurrentRange;
    
    UInt16      OldCommandId;
};


} // namespace OVR

#endif // OVR_SensorImpl_h
