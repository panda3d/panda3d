/************************************************************************************

Filename    :   OVR_SensorImpl.cpp
Content     :   Oculus Sensor device implementation.
Created     :   March 7, 2013
Authors     :   Lee Cooper

Copyright   :   Copyright 2013 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#include "OVR_SensorImpl.h"

// HMDDeviceDesc can be created/updated through Sensor carrying DisplayInfo.

#include "Kernel/OVR_Timer.h"

namespace OVR {
    
//-------------------------------------------------------------------------------------
// ***** Oculus Sensor-specific packet data structures

enum {    
    Sensor_VendorId  = Oculus_VendorId,
    Sensor_ProductId = 0x0001,

    // ST's VID used originally; should be removed in the future
    Sensor_OldVendorId  = 0x0483,
    Sensor_OldProductId = 0x5750,

    Sensor_DefaultReportRate = 500, // Hz
    Sensor_MaxReportRate     = 1000 // Hz
};

// Reported data is little-endian now
static UInt16 DecodeUInt16(const UByte* buffer)
{
    return (UInt16(buffer[1]) << 8) | UInt16(buffer[0]);
}

static SInt16 DecodeSInt16(const UByte* buffer)
{
    return (SInt16(buffer[1]) << 8) | SInt16(buffer[0]);
}

static UInt32 DecodeUInt32(const UByte* buffer)
{    
    return (buffer[0]) | UInt32(buffer[1] << 8) | UInt32(buffer[2] << 16) | UInt32(buffer[3] << 24);    
}

static float DecodeFloat(const UByte* buffer)
{
    union {
        UInt32 U;
        float  F;
    };

    U = DecodeUInt32(buffer);
    return F;
}


static void UnpackSensor(const UByte* buffer, SInt32* x, SInt32* y, SInt32* z)
{
    // Sign extending trick
    // from http://graphics.stanford.edu/~seander/bithacks.html#FixedSignExtend
    struct {SInt32 x:21;} s;

    *x = s.x = (buffer[0] << 13) | (buffer[1] << 5) | ((buffer[2] & 0xF8) >> 3);
    *y = s.x = ((buffer[2] & 0x07) << 18) | (buffer[3] << 10) | (buffer[4] << 2) |
               ((buffer[5] & 0xC0) >> 6);
    *z = s.x = ((buffer[5] & 0x3F) << 15) | (buffer[6] << 7) | (buffer[7] >> 1);
}

// Messages we care for
enum TrackerMessageType
{
    TrackerMessage_None              = 0,
    TrackerMessage_Sensors           = 1,
    TrackerMessage_Unknown           = 0x100,
    TrackerMessage_SizeError         = 0x101,
};

struct TrackerSample
{
    SInt32 AccelX, AccelY, AccelZ;
    SInt32 GyroX, GyroY, GyroZ;
};


struct TrackerSensors
{
    UByte	SampleCount;
    UInt16	Timestamp;
    UInt16	LastCommandID;
    SInt16	Temperature;

    TrackerSample Samples[3];

    SInt16	MagX, MagY, MagZ;

    TrackerMessageType Decode(const UByte* buffer, int size)
    {
        if (size < 62)
            return TrackerMessage_SizeError;

        SampleCount		= buffer[1];
        Timestamp		= DecodeUInt16(buffer + 2);
        LastCommandID	= DecodeUInt16(buffer + 4);
        Temperature		= DecodeSInt16(buffer + 6);
        
        //if (SampleCount > 2)        
        //    OVR_DEBUG_LOG_TEXT(("TackerSensor::Decode SampleCount=%d\n", SampleCount));        

        // Only unpack as many samples as there actually are
        UByte iterationCount = (SampleCount > 2) ? 3 : SampleCount;

        for (UByte i = 0; i < iterationCount; i++)
        {
            UnpackSensor(buffer + 8 + 16 * i,  &Samples[i].AccelX, &Samples[i].AccelY, &Samples[i].AccelZ);
            UnpackSensor(buffer + 16 + 16 * i, &Samples[i].GyroX,  &Samples[i].GyroY,  &Samples[i].GyroZ);
        }

        MagX = DecodeSInt16(buffer + 56);
        MagY = DecodeSInt16(buffer + 58);
        MagZ = DecodeSInt16(buffer + 60);

        return TrackerMessage_Sensors;
    }
};

struct TrackerMessage
{
    TrackerMessageType Type;
    TrackerSensors     Sensors;
};

bool DecodeTrackerMessage(TrackerMessage* message, UByte* buffer, int size)
{
    memset(message, 0, sizeof(TrackerMessage));

    if (size < 4)
    {
        message->Type = TrackerMessage_SizeError;
        return false;
    }

    switch (buffer[0])
    {
    case TrackerMessage_Sensors:
        message->Type = message->Sensors.Decode(buffer, size);
        break;

    default:
        message->Type = TrackerMessage_Unknown;
        break;
    }

    return (message->Type < TrackerMessage_Unknown) && (message->Type != TrackerMessage_None);
}


// ***** SensorRangeImpl Implementation

// Sensor HW only accepts specific maximum range values, used to maximize
// the 16-bit sensor outputs. Use these ramps to specify and report appropriate values.
static const UInt16 AccelRangeRamp[] = { 2, 4, 8, 16 };
static const UInt16 GyroRangeRamp[]  = { 250, 500, 1000, 2000 };
static const UInt16 MagRangeRamp[]   = { 880, 1300, 1900, 2500 };

static UInt16 SelectSensorRampValue(const UInt16* ramp, unsigned count,
                                    float val, float factor, const char* label)
{    
    UInt16 threshold = (UInt16)(val * factor);

    for (unsigned i = 0; i<count; i++)
    {
        if (ramp[i] >= threshold)
            return ramp[i];
    }
    OVR_DEBUG_LOG(("SensorDevice::SetRange - %s clamped to %0.4f",
                   label, float(ramp[count-1]) / factor));
    OVR_UNUSED2(factor, label);
    return ramp[count-1];
}

// SensorScaleImpl provides buffer packing logic for the Sensor Range
// record that can be applied to DK1 sensor through Get/SetFeature. We expose this
// through SensorRange class, which has different units.
struct SensorRangeImpl
{
    enum  { PacketSize = 8 };
    UByte   Buffer[PacketSize];
    
    UInt16  CommandId;
    UInt16  AccelScale;
    UInt16  GyroScale;
    UInt16  MagScale;

    SensorRangeImpl(const SensorRange& r, UInt16 commandId = 0)
    {
        SetSensorRange(r, commandId);
    }

    void SetSensorRange(const SensorRange& r, UInt16 commandId = 0)
    {
        CommandId  = commandId;
        AccelScale = SelectSensorRampValue(AccelRangeRamp, sizeof(AccelRangeRamp)/sizeof(AccelRangeRamp[0]),
                                           r.MaxAcceleration, (1.0f / 9.81f), "MaxAcceleration");
        GyroScale  = SelectSensorRampValue(GyroRangeRamp, sizeof(GyroRangeRamp)/sizeof(GyroRangeRamp[0]),
                                           r.MaxRotationRate, Math<float>::RadToDegreeFactor, "MaxRotationRate");
        MagScale   = SelectSensorRampValue(MagRangeRamp, sizeof(MagRangeRamp)/sizeof(MagRangeRamp[0]),
                                           r.MaxMagneticField, 1000.0f, "MaxMagneticField");
        Pack();
    }

    void GetSensorRange(SensorRange* r)
    {
        r->MaxAcceleration = AccelScale * 9.81f;
        r->MaxRotationRate = DegreeToRad((float)GyroScale);
        r->MaxMagneticField= MagScale * 0.001f;
    }

    static SensorRange GetMaxSensorRange()
    {
        return SensorRange(AccelRangeRamp[sizeof(AccelRangeRamp)/sizeof(AccelRangeRamp[0]) - 1] * 9.81f,
                           GyroRangeRamp[sizeof(GyroRangeRamp)/sizeof(GyroRangeRamp[0]) - 1] *
                                Math<float>::DegreeToRadFactor,
                           MagRangeRamp[sizeof(MagRangeRamp)/sizeof(MagRangeRamp[0]) - 1] * 0.001f);
    }

    void  Pack()
    {
        Buffer[0] = 4;
        Buffer[1] = UByte(CommandId & 0xFF);
        Buffer[2] = UByte(CommandId >> 8);
        Buffer[3] = UByte(AccelScale);
        Buffer[4] = UByte(GyroScale & 0xFF);
        Buffer[5] = UByte(GyroScale >> 8);
        Buffer[6] = UByte(MagScale & 0xFF);
        Buffer[7] = UByte(MagScale >> 8);
    }

    void Unpack()
    {
        CommandId = Buffer[1] | (UInt16(Buffer[2]) << 8);
        AccelScale= Buffer[3];
        GyroScale = Buffer[4] | (UInt16(Buffer[5]) << 8);
        MagScale  = Buffer[6] | (UInt16(Buffer[7]) << 8);
    }
};


// Sensor configuration command, ReportId == 2.

struct SensorConfigImpl
{
    enum  { PacketSize = 7 };
    UByte   Buffer[PacketSize];

    // Flag values for Flags.
    enum {
        Flag_RawMode            = 0x01,
        Flag_CallibrationTest   = 0x02, // Internal test mode
        Flag_UseCallibration    = 0x04,
        Flag_AutoCallibration   = 0x08,
        Flag_MotionKeepAlive    = 0x10,
        Flag_CommandKeepAlive   = 0x20,
        Flag_SensorCoordinates  = 0x40
    };

    UInt16  CommandId;
    UByte   Flags;
    UInt16  PacketInterval;
    UInt16  KeepAliveIntervalMs;

    SensorConfigImpl() : CommandId(0), Flags(0), PacketInterval(0), KeepAliveIntervalMs(0)
    {
        memset(Buffer, 0, PacketSize);
        Buffer[0] = 2;
    }

    void    SetSensorCoordinates(bool sensorCoordinates)
    { Flags = (Flags & ~Flag_SensorCoordinates) | (sensorCoordinates ? Flag_SensorCoordinates : 0); }
    bool    IsUsingSensorCoordinates() const
    { return (Flags & Flag_SensorCoordinates) != 0; }

    void Pack()
    {
        Buffer[0] = 2;
        Buffer[1] = UByte(CommandId & 0xFF);
        Buffer[2] = UByte(CommandId >> 8);
        Buffer[3] = Flags;
        Buffer[4] = UByte(PacketInterval);
        Buffer[5] = UByte(KeepAliveIntervalMs & 0xFF);
        Buffer[6] = UByte(KeepAliveIntervalMs >> 8);
    }

    void Unpack()
    {
        CommandId          = Buffer[1] | (UInt16(Buffer[2]) << 8);
        Flags              = Buffer[3];
        PacketInterval     = Buffer[4];
        KeepAliveIntervalMs= Buffer[5] | (UInt16(Buffer[6]) << 8);
    }
    
};


// SensorKeepAlive - feature report that needs to be sent at regular intervals for sensor
// to receive commands.
struct SensorKeepAliveImpl
{
    enum  { PacketSize = 5 };
    UByte   Buffer[PacketSize];

    UInt16  CommandId;
    UInt16  KeepAliveIntervalMs;

    SensorKeepAliveImpl(UInt16 interval = 0, UInt16 commandId = 0)
        : CommandId(commandId), KeepAliveIntervalMs(interval)
    {
        Pack();
    }

    void  Pack()
    {
        Buffer[0] = 8;
        Buffer[1] = UByte(CommandId & 0xFF);
        Buffer[2] = UByte(CommandId >> 8);
        Buffer[3] = UByte(KeepAliveIntervalMs & 0xFF);
        Buffer[4] = UByte(KeepAliveIntervalMs >> 8);
    }

    void Unpack()
    {
        CommandId          = Buffer[1] | (UInt16(Buffer[2]) << 8);
        KeepAliveIntervalMs= Buffer[3] | (UInt16(Buffer[4]) << 8);
    }
};


//-------------------------------------------------------------------------------------
// ***** SensorDisplayInfoImpl
SensorDisplayInfoImpl::SensorDisplayInfoImpl()
 :  CommandId(0), DistortionType(Base_None)
{
    memset(Buffer, 0, PacketSize);
    Buffer[0] = 9;
}

void SensorDisplayInfoImpl::Unpack()
{
    CommandId               = Buffer[1] | (UInt16(Buffer[2]) << 8);
    DistortionType          = Buffer[3];
    HResolution             = DecodeUInt16(Buffer+4);
    VResolution             = DecodeUInt16(Buffer+6);
    HScreenSize             = DecodeUInt32(Buffer+8) *  (1/1000000.f);
    VScreenSize             = DecodeUInt32(Buffer+12) * (1/1000000.f);
    VCenter                 = DecodeUInt32(Buffer+16) * (1/1000000.f);
    LensSeparation          = DecodeUInt32(Buffer+20) * (1/1000000.f);
    EyeToScreenDistance[0]  = DecodeUInt32(Buffer+24) * (1/1000000.f);
    EyeToScreenDistance[1]  = DecodeUInt32(Buffer+28) * (1/1000000.f);
    DistortionK[0]          = DecodeFloat(Buffer+32);
    DistortionK[1]          = DecodeFloat(Buffer+36);
    DistortionK[2]          = DecodeFloat(Buffer+40);
    DistortionK[3]          = DecodeFloat(Buffer+44);
    DistortionK[4]          = DecodeFloat(Buffer+48);
    DistortionK[5]          = DecodeFloat(Buffer+52);
}


//-------------------------------------------------------------------------------------
// ***** SensorDeviceFactory

SensorDeviceFactory SensorDeviceFactory::Instance;

void SensorDeviceFactory::EnumerateDevices(EnumerateVisitor& visitor)
{

    class SensorEnumerator : public HIDEnumerateVisitor
    {
        // Assign not supported; suppress MSVC warning.
        void operator = (const SensorEnumerator&) { }

        DeviceFactory*     pFactory;
        EnumerateVisitor&  ExternalVisitor;   
    public:
        SensorEnumerator(DeviceFactory* factory, EnumerateVisitor& externalVisitor)
            : pFactory(factory), ExternalVisitor(externalVisitor) { }

        virtual bool MatchVendorProduct(UInt16 vendorId, UInt16 productId)
        {
            return pFactory->MatchVendorProduct(vendorId, productId);
        }

        virtual void Visit(HIDDevice& device, const HIDDeviceDesc& desc)
        {
            SensorDeviceCreateDesc createDesc(pFactory, desc);
            ExternalVisitor.Visit(createDesc);

            // Check if the sensor returns DisplayInfo. If so, try to use it to override potentially
            // mismatching monitor information (in case wrong EDID is reported by splitter),
            // or to create a new "virtualized" HMD Device.
            
            SensorDisplayInfoImpl displayInfo;

            if (device.GetFeatureReport(displayInfo.Buffer, SensorDisplayInfoImpl::PacketSize))
            {
                displayInfo.Unpack();

                // If we got display info, try to match / create HMDDevice as well
                // so that sensor settings give preference.
                if (displayInfo.DistortionType & SensorDisplayInfoImpl::Mask_BaseFmt)
                {
                    SensorDeviceImpl::EnumerateHMDFromSensorDisplayInfo(displayInfo, ExternalVisitor);
                }
            }
        }
    };

    //double start = Timer::GetProfileSeconds();

    SensorEnumerator sensorEnumerator(this, visitor);
    GetManagerImpl()->GetHIDDeviceManager()->Enumerate(&sensorEnumerator);

    //double totalSeconds = Timer::GetProfileSeconds() - start; 
}

bool SensorDeviceFactory::MatchVendorProduct(UInt16 vendorId, UInt16 productId) const
{
    return ((vendorId == Sensor_VendorId) && (productId == Sensor_ProductId)) ||
        ((vendorId == Sensor_OldVendorId) && (productId == Sensor_OldProductId));
}

bool SensorDeviceFactory::DetectHIDDevice(DeviceManager* pdevMgr, const HIDDeviceDesc& desc)
{
    if (MatchVendorProduct(desc.VendorId, desc.ProductId))
    {
        SensorDeviceCreateDesc createDesc(this, desc);
        return pdevMgr->AddDevice_NeedsLock(createDesc).GetPtr() != NULL;
    }
    return false;
}

//-------------------------------------------------------------------------------------
// ***** SensorDeviceCreateDesc

DeviceBase* SensorDeviceCreateDesc::NewDeviceInstance()
{
    return new SensorDeviceImpl(this);
}

bool SensorDeviceCreateDesc::GetDeviceInfo(DeviceInfo* info) const
{
    if ((info->InfoClassType != Device_Sensor) &&
        (info->InfoClassType != Device_None))
        return false;

    OVR_strcpy(info->ProductName,  DeviceInfo::MaxNameLength, HIDDesc.Product.ToCStr());
    OVR_strcpy(info->Manufacturer, DeviceInfo::MaxNameLength, HIDDesc.Manufacturer.ToCStr());
    info->Type    = Device_Sensor;
    info->Version = 0;

    if (info->InfoClassType == Device_Sensor)
    {
        SensorInfo* sinfo = (SensorInfo*)info;
        sinfo->VendorId  = HIDDesc.VendorId;
        sinfo->ProductId = HIDDesc.ProductId;
        sinfo->MaxRanges = SensorRangeImpl::GetMaxSensorRange();
        OVR_strcpy(sinfo->SerialNumber, sizeof(sinfo->SerialNumber),HIDDesc.SerialNumber.ToCStr());
    }
    return true;
}


//-------------------------------------------------------------------------------------
// ***** SensorDevice

SensorDeviceImpl::SensorDeviceImpl(SensorDeviceCreateDesc* createDesc)
    : OVR::HIDDeviceImpl<OVR::SensorDevice>(createDesc, 0),
      Coordinates(SensorDevice::Coord_Sensor),
      HWCoordinates(SensorDevice::Coord_HMD), // HW reports HMD coordinates by default.
      NextKeepAliveTicks(0),
      MaxValidRange(SensorRangeImpl::GetMaxSensorRange())
{
    SequenceValid  = false;
    LastSampleCount= 0;
    LastTimestamp   = 0;

    OldCommandId = 0;
}

SensorDeviceImpl::~SensorDeviceImpl()
{
    // Check that Shutdown() was called.
    OVR_ASSERT(!pCreateDesc->pDevice);    
}

// Internal creation APIs.
bool SensorDeviceImpl::Initialize(DeviceBase* parent)
{
    if (HIDDeviceImpl<OVR::SensorDevice>::Initialize(parent))
    {
        openDevice();

        LogText("OVR::SensorDevice initialized.\n");

        return true;
    }

    return false;
}

void SensorDeviceImpl::openDevice()
{

    // Read the currently configured range from sensor.
    SensorRangeImpl sr(SensorRange(), 0);

    if (GetInternalDevice()->GetFeatureReport(sr.Buffer, SensorRangeImpl::PacketSize))
    {
        sr.Unpack();
        sr.GetSensorRange(&CurrentRange);
    }


    // If the sensor has "DisplayInfo" data, use HMD coordinate frame by default.
    SensorDisplayInfoImpl displayInfo;
    if (GetInternalDevice()->GetFeatureReport(displayInfo.Buffer, SensorDisplayInfoImpl::PacketSize))
    {
        displayInfo.Unpack();
        Coordinates = (displayInfo.DistortionType & SensorDisplayInfoImpl::Mask_BaseFmt) ?
                      Coord_HMD : Coord_Sensor;
    }

    // Read/Apply sensor config.
    setCoordinateFrame(Coordinates);
    setReportRate(Sensor_DefaultReportRate);

    // Set Keep-alive at 10 seconds.
    SensorKeepAliveImpl skeepAlive(10 * 1000);
    GetInternalDevice()->SetFeatureReport(skeepAlive.Buffer, SensorKeepAliveImpl::PacketSize);
}

void SensorDeviceImpl::closeDeviceOnError()
{
    LogText("OVR::SensorDevice - Lost connection to '%s'\n", getHIDDesc()->Path.ToCStr());
    NextKeepAliveTicks = 0;
}

void SensorDeviceImpl::Shutdown()
{   
    HIDDeviceImpl<OVR::SensorDevice>::Shutdown();

    LogText("OVR::SensorDevice - Closed '%s'\n", getHIDDesc()->Path.ToCStr());
}


void SensorDeviceImpl::OnInputReport(UByte* pData, UInt32 length)
{

    bool processed = false;
    if (!processed)
    {

        TrackerMessage message;
        if (DecodeTrackerMessage(&message, pData, length))
        {
            processed = true;
            onTrackerMessage(&message);
        }
    }
}

UInt64 SensorDeviceImpl::OnTicks(UInt64 ticksMks)
{

    if (ticksMks >= NextKeepAliveTicks)
    {
        // Use 3-seconds keep alive by default.
        UInt64 keepAliveDelta = Timer::MksPerSecond * 3;

        // Set Keep-alive at 10 seconds.
        SensorKeepAliveImpl skeepAlive(10 * 1000);
        // OnTicks is called from background thread so we don't need to add this to the command queue.
        GetInternalDevice()->SetFeatureReport(skeepAlive.Buffer, SensorKeepAliveImpl::PacketSize);

		// Emit keep-alive every few seconds.
        NextKeepAliveTicks = ticksMks + keepAliveDelta;
    }
    return NextKeepAliveTicks - ticksMks;
}

bool SensorDeviceImpl::SetRange(const SensorRange& range, bool waitFlag)
{
    bool                 result = 0;
    ThreadCommandQueue * threadQueue = GetManagerImpl()->GetThreadQueue();

    if (!waitFlag)
    {
        return threadQueue->PushCall(this, &SensorDeviceImpl::setRange, range);
    }
    
    if (!threadQueue->PushCallAndWaitResult(this, 
                                            &SensorDeviceImpl::setRange,
                                            &result, 
                                            range))
    {
        return false;
    }

    return result;
}

void SensorDeviceImpl::GetRange(SensorRange* range) const
{
    Lock::Locker lockScope(GetLock());
    *range = CurrentRange;
}

bool SensorDeviceImpl::setRange(const SensorRange& range)
{
    SensorRangeImpl sr(range);
    
    if (GetInternalDevice()->SetFeatureReport(sr.Buffer, SensorRangeImpl::PacketSize))
    {
        Lock::Locker lockScope(GetLock());
        sr.GetSensorRange(&CurrentRange);
        return true;
    }
    
    return false;
}

void SensorDeviceImpl::SetCoordinateFrame(CoordinateFrame coordframe)
{ 
    // Push call with wait.
    GetManagerImpl()->GetThreadQueue()->
        PushCall(this, &SensorDeviceImpl::setCoordinateFrame, coordframe, true);
}

SensorDevice::CoordinateFrame SensorDeviceImpl::GetCoordinateFrame() const
{
    return Coordinates;
}

Void SensorDeviceImpl::setCoordinateFrame(CoordinateFrame coordframe)
{

    Coordinates = coordframe;

    // Read the original coordinate frame, then try to change it.
    SensorConfigImpl scfg;
    if (GetInternalDevice()->GetFeatureReport(scfg.Buffer, SensorConfigImpl::PacketSize))
    {
        scfg.Unpack();
    }

    scfg.SetSensorCoordinates(coordframe == Coord_Sensor);
    scfg.Pack();

    GetInternalDevice()->SetFeatureReport(scfg.Buffer, SensorConfigImpl::PacketSize);
    
    // Re-read the state, in case of older firmware that doesn't support Sensor coordinates.
    if (GetInternalDevice()->GetFeatureReport(scfg.Buffer, SensorConfigImpl::PacketSize))
    {
        scfg.Unpack();
        HWCoordinates = scfg.IsUsingSensorCoordinates() ? Coord_Sensor : Coord_HMD;
    }
    else
    {
        HWCoordinates = Coord_HMD;
    }
    return 0;
}

void SensorDeviceImpl::SetReportRate(unsigned rateHz)
{ 
    // Push call with wait.
    GetManagerImpl()->GetThreadQueue()->
        PushCall(this, &SensorDeviceImpl::setReportRate, rateHz, true);
}

unsigned SensorDeviceImpl::GetReportRate() const
{
    // Read the original configuration
    SensorConfigImpl scfg;
    if (GetInternalDevice()->GetFeatureReport(scfg.Buffer, SensorConfigImpl::PacketSize))
    {
        scfg.Unpack();
        return Sensor_MaxReportRate / (scfg.PacketInterval + 1);
    }
    return 0; // error
}

Void SensorDeviceImpl::setReportRate(unsigned rateHz)
{
    // Read the original configuration
    SensorConfigImpl scfg;
    if (GetInternalDevice()->GetFeatureReport(scfg.Buffer, SensorConfigImpl::PacketSize))
    {
        scfg.Unpack();
    }

    if (rateHz > Sensor_MaxReportRate)
        rateHz = Sensor_MaxReportRate;
    else if (rateHz == 0)
        rateHz = Sensor_DefaultReportRate;

    scfg.PacketInterval = UInt16((Sensor_MaxReportRate / rateHz) - 1);

    scfg.Pack();

    GetInternalDevice()->SetFeatureReport(scfg.Buffer, SensorConfigImpl::PacketSize);
    return 0;
}

void SensorDeviceImpl::SetMessageHandler(MessageHandler* handler)
{
    if (handler)
    {
        SequenceValid = false;
        DeviceBase::SetMessageHandler(handler);
    }
    else
    {       
        DeviceBase::SetMessageHandler(handler);
    }    
}

// Sensor reports data in the following coordinate system:
// Accelerometer: 10^-4 m/s^2; X forward, Y right, Z Down.
// Gyro:          10^-4 rad/s; X positive roll right, Y positive pitch up; Z positive yaw right.


// We need to convert it to the following RHS coordinate system:
// X right, Y Up, Z Back (out of screen)
//
Vector3f AccelFromBodyFrameUpdate(const TrackerSensors& update, UByte sampleNumber,
                                  bool convertHMDToSensor = false)
{
    const TrackerSample& sample = update.Samples[sampleNumber];
    float                ax = (float)sample.AccelX;
    float                ay = (float)sample.AccelY;
    float                az = (float)sample.AccelZ;

    Vector3f val = convertHMDToSensor ? Vector3f(ax, az, -ay) :  Vector3f(ax, ay, az);
    return val * 0.0001f;
}


Vector3f MagFromBodyFrameUpdate(const TrackerSensors& update,
                                bool convertHMDToSensor = false)
{   
    // Note: Y and Z are swapped in comparison to the Accel.  
    // This accounts for DK1 sensor firmware axis swap, which should be undone in future releases.
    if (!convertHMDToSensor)
    {
        return Vector3f( (float)update.MagX,
                         (float)update.MagZ,
                         (float)update.MagY) * 0.0001f;
    }    

    return Vector3f( (float)update.MagX,
                     (float)update.MagY,
                    -(float)update.MagZ) * 0.0001f;
}

Vector3f EulerFromBodyFrameUpdate(const TrackerSensors& update, UByte sampleNumber,
                                  bool convertHMDToSensor = false)
{
    const TrackerSample& sample = update.Samples[sampleNumber];
    float                gx = (float)sample.GyroX;
    float                gy = (float)sample.GyroY;
    float                gz = (float)sample.GyroZ;

    Vector3f val = convertHMDToSensor ? Vector3f(gx, gz, -gy) :  Vector3f(gx, gy, gz);
    return val * 0.0001f;
}


void SensorDeviceImpl::onTrackerMessage(TrackerMessage* message)
{
    if (message->Type != TrackerMessage_Sensors)
        return;
    
    const float     timeUnit   = (1.0f / 1000.f);
    TrackerSensors& s = message->Sensors;
    

    // Call OnMessage() within a lock to avoid conflicts with handlers.
    Lock::Locker scopeLock(HandlerRef.GetLock());


    if (SequenceValid)
    {
        unsigned timestampDelta;

        if (s.Timestamp < LastTimestamp)
            timestampDelta = ((((int)s.Timestamp) + 0x10000) - (int)LastTimestamp);
        else
            timestampDelta = (s.Timestamp - LastTimestamp);

        // If we missed a small number of samples, replicate the last sample.
        if ((timestampDelta > LastSampleCount) && (timestampDelta <= 254))
        {
            if (HandlerRef.GetHandler())
            {
                MessageBodyFrame sensors(this);
                sensors.TimeDelta     = (timestampDelta - LastSampleCount) * timeUnit;
                sensors.Acceleration  = LastAcceleration;
                sensors.RotationRate  = LastRotationRate;
                sensors.MagneticField = LastMagneticField;
                sensors.Temperature   = LastTemperature;

                HandlerRef.GetHandler()->OnMessage(sensors);
            }
        }
    }
    else
    {
        LastAcceleration = Vector3f(0);
        LastRotationRate = Vector3f(0);
        LastMagneticField= Vector3f(0);
        LastTemperature  = 0;
        SequenceValid    = true;
    }

    LastSampleCount = s.SampleCount;
    LastTimestamp   = s.Timestamp;

    bool convertHMDToSensor = (Coordinates == Coord_Sensor) && (HWCoordinates == Coord_HMD);

    if (HandlerRef.GetHandler())
    {
        MessageBodyFrame sensors(this);                
        UByte            iterations = s.SampleCount;

        if (s.SampleCount > 3)
        {
            iterations        = 3;
            sensors.TimeDelta = (s.SampleCount - 2) * timeUnit;
        }
        else
        {
            sensors.TimeDelta = timeUnit;
        }

        for (UByte i = 0; i < iterations; i++)
        {            
            sensors.Acceleration = AccelFromBodyFrameUpdate(s, i, convertHMDToSensor);
            sensors.RotationRate = EulerFromBodyFrameUpdate(s, i, convertHMDToSensor);
            sensors.MagneticField= MagFromBodyFrameUpdate(s, convertHMDToSensor);
            sensors.Temperature  = s.Temperature * 0.01f;
            HandlerRef.GetHandler()->OnMessage(sensors);
            // TimeDelta for the last two sample is always fixed.
            sensors.TimeDelta = timeUnit;
        }

        LastAcceleration = sensors.Acceleration;
        LastRotationRate = sensors.RotationRate;
        LastMagneticField= sensors.MagneticField;
        LastTemperature  = sensors.Temperature;
    }
    else
    {
        UByte i = (s.SampleCount > 3) ? 2 : (s.SampleCount - 1);
        LastAcceleration  = AccelFromBodyFrameUpdate(s, i, convertHMDToSensor);
        LastRotationRate  = EulerFromBodyFrameUpdate(s, i, convertHMDToSensor);
        LastMagneticField = MagFromBodyFrameUpdate(s, convertHMDToSensor);
        LastTemperature   = s.Temperature * 0.01f;
    }
}

} // namespace OVR


