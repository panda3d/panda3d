/************************************************************************************

Filename    :   OVR_LatencyTestImpl.cpp
Content     :   Oculus Latency Tester device implementation.
Created     :   March 7, 2013
Authors     :   Lee Cooper

Copyright   :   Copyright 2013 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#include "OVR_LatencyTestImpl.h"

namespace OVR {

//-------------------------------------------------------------------------------------
// ***** Oculus Latency Tester specific packet data structures

enum {    
    LatencyTester_VendorId  = Oculus_VendorId,
    LatencyTester_ProductId = 0x0101,
};

// Reported data is little-endian now
static UInt16 DecodeUInt16(const UByte* buffer)
{
    return (UInt16(buffer[1]) << 8) | UInt16(buffer[0]);
}

/* Unreferenced
static SInt16 DecodeSInt16(const UByte* buffer)
{
    return (SInt16(buffer[1]) << 8) | SInt16(buffer[0]);
}*/

static void UnpackSamples(const UByte* buffer, UByte* r, UByte* g, UByte* b)
{
    *r = buffer[0];
    *g = buffer[1];
    *b = buffer[2];
}

// Messages we handle.
enum LatencyTestMessageType
{
    LatencyTestMessage_None                 = 0,
    LatencyTestMessage_Samples              = 1,
    LatencyTestMessage_ColorDetected        = 2,
    LatencyTestMessage_TestStarted          = 3,
    LatencyTestMessage_Button               = 4,
    LatencyTestMessage_Unknown              = 0x100,
    LatencyTestMessage_SizeError            = 0x101,
};

struct LatencyTestSample
{
    UByte Value[3];
};

struct LatencyTestSamples
{
    UByte	SampleCount;
    UInt16	Timestamp;

    LatencyTestSample Samples[20];

    LatencyTestMessageType Decode(const UByte* buffer, int size)
    {
        if (size < 64)
        {
            return LatencyTestMessage_SizeError;
        }

        SampleCount		= buffer[1];
        Timestamp		= DecodeUInt16(buffer + 2);
        
        for (UByte i = 0; i < SampleCount; i++)
        {
            UnpackSamples(buffer + 4 + (3 * i),  &Samples[i].Value[0], &Samples[i].Value[1], &Samples[i].Value[2]);
        }

        return LatencyTestMessage_Samples;
    }
};

struct LatencyTestSamplesMessage
{
    LatencyTestMessageType      Type;
    LatencyTestSamples        Samples;
};

bool DecodeLatencyTestSamplesMessage(LatencyTestSamplesMessage* message, UByte* buffer, int size)
{
    memset(message, 0, sizeof(LatencyTestSamplesMessage));

    if (size < 64)
    {
        message->Type = LatencyTestMessage_SizeError;
        return false;
    }

    switch (buffer[0])
    {
    case LatencyTestMessage_Samples:
        message->Type = message->Samples.Decode(buffer, size);
        break;

    default:
        message->Type = LatencyTestMessage_Unknown;
        break;
    }

    return (message->Type < LatencyTestMessage_Unknown) && (message->Type != LatencyTestMessage_None);
}

struct LatencyTestColorDetected
{
    UInt16	CommandID;
    UInt16	Timestamp;
    UInt16  Elapsed;
    UByte   TriggerValue[3];
    UByte   TargetValue[3];

    LatencyTestMessageType Decode(const UByte* buffer, int size)
    {
        if (size < 13)
            return LatencyTestMessage_SizeError;

        CommandID = DecodeUInt16(buffer + 1);
        Timestamp = DecodeUInt16(buffer + 3);
        Elapsed = DecodeUInt16(buffer + 5);
        memcpy(TriggerValue, buffer + 7, 3);
        memcpy(TargetValue, buffer + 10, 3);

        return LatencyTestMessage_ColorDetected;
    }
};

struct LatencyTestColorDetectedMessage
{
    LatencyTestMessageType    Type;
    LatencyTestColorDetected  ColorDetected;
};

bool DecodeLatencyTestColorDetectedMessage(LatencyTestColorDetectedMessage* message, UByte* buffer, int size)
{
    memset(message, 0, sizeof(LatencyTestColorDetectedMessage));

    if (size < 13)
    {
        message->Type = LatencyTestMessage_SizeError;
        return false;
    }

    switch (buffer[0])
    {
    case LatencyTestMessage_ColorDetected:
        message->Type = message->ColorDetected.Decode(buffer, size);
        break;

    default:
        message->Type = LatencyTestMessage_Unknown;
        break;
    }

    return (message->Type < LatencyTestMessage_Unknown) && (message->Type != LatencyTestMessage_None);
}

struct LatencyTestStarted
{
    UInt16	CommandID;
    UInt16	Timestamp;
    UByte   TargetValue[3];

    LatencyTestMessageType Decode(const UByte* buffer, int size)
    {
        if (size < 8)
            return LatencyTestMessage_SizeError;

        CommandID = DecodeUInt16(buffer + 1);
        Timestamp = DecodeUInt16(buffer + 3);
        memcpy(TargetValue, buffer + 5, 3);

        return LatencyTestMessage_TestStarted;
    }
};

struct LatencyTestStartedMessage
{
    LatencyTestMessageType  Type;
    LatencyTestStarted  TestStarted;
};

bool DecodeLatencyTestStartedMessage(LatencyTestStartedMessage* message, UByte* buffer, int size)
{
    memset(message, 0, sizeof(LatencyTestStartedMessage));

    if (size < 8)
    {
        message->Type = LatencyTestMessage_SizeError;
        return false;
    }

    switch (buffer[0])
    {
    case LatencyTestMessage_TestStarted:
        message->Type = message->TestStarted.Decode(buffer, size);
        break;

    default:
        message->Type = LatencyTestMessage_Unknown;
        break;
    }

    return (message->Type < LatencyTestMessage_Unknown) && (message->Type != LatencyTestMessage_None);
}

struct LatencyTestButton
{
    UInt16	CommandID;
    UInt16	Timestamp;

    LatencyTestMessageType Decode(const UByte* buffer, int size)
    {
        if (size < 5)
            return LatencyTestMessage_SizeError;

        CommandID = DecodeUInt16(buffer + 1);
        Timestamp = DecodeUInt16(buffer + 3);

        return LatencyTestMessage_Button;
    }
};

struct LatencyTestButtonMessage
{
    LatencyTestMessageType    Type;
    LatencyTestButton         Button;
};

bool DecodeLatencyTestButtonMessage(LatencyTestButtonMessage* message, UByte* buffer, int size)
{
    memset(message, 0, sizeof(LatencyTestButtonMessage));

    if (size < 5)
    {
        message->Type = LatencyTestMessage_SizeError;
        return false;
    }

    switch (buffer[0])
    {
    case LatencyTestMessage_Button:
        message->Type = message->Button.Decode(buffer, size);
        break;

    default:
        message->Type = LatencyTestMessage_Unknown;
        break;
    }

    return (message->Type < LatencyTestMessage_Unknown) && (message->Type != LatencyTestMessage_None);
}

struct LatencyTestConfigurationImpl
{
    enum  { PacketSize = 5 };
    UByte   Buffer[PacketSize];

    OVR::LatencyTestConfiguration  Configuration;

    LatencyTestConfigurationImpl(const OVR::LatencyTestConfiguration& configuration)
        : Configuration(configuration)
    {
        Pack();
    }

    void Pack()
    {
        Buffer[0] = 5;
		Buffer[1] = UByte(Configuration.SendSamples);
		Buffer[2] = Configuration.Threshold.R;
        Buffer[3] = Configuration.Threshold.G;
        Buffer[4] = Configuration.Threshold.B;
    }

    void Unpack()
    {
		Configuration.SendSamples = Buffer[1] != 0 ? true : false;
        Configuration.Threshold.R = Buffer[2];
        Configuration.Threshold.G = Buffer[3];
        Configuration.Threshold.B = Buffer[4];
    }
};

struct LatencyTestCalibrateImpl
{
    enum  { PacketSize = 4 };
    UByte   Buffer[PacketSize];

    Color CalibrationColor;
    
    LatencyTestCalibrateImpl(const Color& calibrationColor)
        : CalibrationColor(calibrationColor)
    {
        Pack();
    }

    void Pack()
    {
        Buffer[0] = 7;
		Buffer[1] = CalibrationColor.R;
		Buffer[2] = CalibrationColor.G;
		Buffer[3] = CalibrationColor.B;
    }

    void Unpack()
    {
        CalibrationColor.R = Buffer[1];
        CalibrationColor.G = Buffer[2];
        CalibrationColor.B = Buffer[3];
    }
};

struct LatencyTestStartTestImpl
{
    enum  { PacketSize = 6 };
    UByte   Buffer[PacketSize];

    Color TargetColor;

    LatencyTestStartTestImpl(const Color& targetColor)
        : TargetColor(targetColor)
    {
        Pack();
    }

    void Pack()
    {
        UInt16 commandID = 1;

        Buffer[0] = 8;
		Buffer[1] = UByte(commandID  & 0xFF);
		Buffer[2] = UByte(commandID >> 8);
		Buffer[3] = TargetColor.R;
		Buffer[4] = TargetColor.G;
		Buffer[5] = TargetColor.B;
    }

    void Unpack()
    {
//      UInt16 commandID = Buffer[1] | (UInt16(Buffer[2]) << 8);
        TargetColor.R = Buffer[3];
        TargetColor.G = Buffer[4];
        TargetColor.B = Buffer[5];
    }
};

struct LatencyTestDisplayImpl
{
    enum  { PacketSize = 6 };
    UByte   Buffer[PacketSize];

    OVR::LatencyTestDisplay  Display;

    LatencyTestDisplayImpl(const OVR::LatencyTestDisplay& display)
        : Display(display)
    {
        Pack();
    }

    void Pack()
    {
        Buffer[0] = 9;
        Buffer[1] = Display.Mode;
        Buffer[2] = UByte(Display.Value & 0xFF);
        Buffer[3] = UByte((Display.Value >> 8) & 0xFF);
        Buffer[4] = UByte((Display.Value >> 16) & 0xFF);
        Buffer[5] = UByte((Display.Value >> 24) & 0xFF);
    }

    void Unpack()
    {
        Display.Mode = Buffer[1];
        Display.Value = UInt32(Buffer[2]) |
            (UInt32(Buffer[3]) << 8) |
            (UInt32(Buffer[4]) << 16) |
            (UInt32(Buffer[5]) << 24);
    }
};

//-------------------------------------------------------------------------------------
// ***** LatencyTestDeviceFactory

LatencyTestDeviceFactory LatencyTestDeviceFactory::Instance;

void LatencyTestDeviceFactory::EnumerateDevices(EnumerateVisitor& visitor)
{

    class LatencyTestEnumerator : public HIDEnumerateVisitor
    {
        // Assign not supported; suppress MSVC warning.
        void operator = (const LatencyTestEnumerator&) { }

        DeviceFactory*     pFactory;
        EnumerateVisitor&  ExternalVisitor;   
    public:
        LatencyTestEnumerator(DeviceFactory* factory, EnumerateVisitor& externalVisitor)
            : pFactory(factory), ExternalVisitor(externalVisitor) { }

        virtual bool MatchVendorProduct(UInt16 vendorId, UInt16 productId)
        {
            return pFactory->MatchVendorProduct(vendorId, productId);
        }

        virtual void Visit(HIDDevice& device, const HIDDeviceDesc& desc)
        {
            OVR_UNUSED(device);

            LatencyTestDeviceCreateDesc createDesc(pFactory, desc);
            ExternalVisitor.Visit(createDesc);
        }
    };

    LatencyTestEnumerator latencyTestEnumerator(this, visitor);
    GetManagerImpl()->GetHIDDeviceManager()->Enumerate(&latencyTestEnumerator);
}

bool LatencyTestDeviceFactory::MatchVendorProduct(UInt16 vendorId, UInt16 productId) const
{
    return ((vendorId == LatencyTester_VendorId) && (productId == LatencyTester_ProductId));                
}

bool LatencyTestDeviceFactory::DetectHIDDevice(DeviceManager* pdevMgr, 
                                               const HIDDeviceDesc& desc)
{
    if (MatchVendorProduct(desc.VendorId, desc.ProductId))
    {
        LatencyTestDeviceCreateDesc createDesc(this, desc);
        return pdevMgr->AddDevice_NeedsLock(createDesc).GetPtr() != NULL;
    }
    return false;
}

//-------------------------------------------------------------------------------------
// ***** LatencyTestDeviceCreateDesc

DeviceBase* LatencyTestDeviceCreateDesc::NewDeviceInstance()
{
    return new LatencyTestDeviceImpl(this);
}

bool LatencyTestDeviceCreateDesc::GetDeviceInfo(DeviceInfo* info) const
{
    if ((info->InfoClassType != Device_LatencyTester) &&
        (info->InfoClassType != Device_None))
        return false;

    OVR_strcpy(info->ProductName,  DeviceInfo::MaxNameLength, HIDDesc.Product.ToCStr());
    OVR_strcpy(info->Manufacturer, DeviceInfo::MaxNameLength, HIDDesc.Manufacturer.ToCStr());
    info->Type    = Device_LatencyTester;
    info->Version = 0;

    if (info->InfoClassType == Device_LatencyTester)
    {
        SensorInfo* sinfo = (SensorInfo*)info;
        sinfo->VendorId  = HIDDesc.VendorId;
        sinfo->ProductId = HIDDesc.ProductId;
        OVR_strcpy(sinfo->SerialNumber, sizeof(sinfo->SerialNumber),HIDDesc.SerialNumber.ToCStr());
    }
    return true;
}

//-------------------------------------------------------------------------------------
// ***** LatencyTestDevice

LatencyTestDeviceImpl::LatencyTestDeviceImpl(LatencyTestDeviceCreateDesc* createDesc)
    : OVR::HIDDeviceImpl<OVR::LatencyTestDevice>(createDesc, 0)
{
}

LatencyTestDeviceImpl::~LatencyTestDeviceImpl()
{
    // Check that Shutdown() was called.
    OVR_ASSERT(!pCreateDesc->pDevice);    
}

// Internal creation APIs.
bool LatencyTestDeviceImpl::Initialize(DeviceBase* parent)
{
    if (HIDDeviceImpl<OVR::LatencyTestDevice>::Initialize(parent))
    {
        LogText("OVR::LatencyTestDevice initialized.\n");
        return true;
    }

    return false;
}

void LatencyTestDeviceImpl::Shutdown()
{   
    HIDDeviceImpl<OVR::LatencyTestDevice>::Shutdown();

    LogText("OVR::LatencyTestDevice - Closed '%s'\n", getHIDDesc()->Path.ToCStr());
}

void LatencyTestDeviceImpl::OnInputReport(UByte* pData, UInt32 length)
{
    
    bool processed = false;
    if (!processed)
    {
        LatencyTestSamplesMessage message; 
        if (DecodeLatencyTestSamplesMessage(&message, pData, length))     
        {
            processed = true;
            onLatencyTestSamplesMessage(&message);
        }
    }

    if (!processed)
    {
        LatencyTestColorDetectedMessage message; 
        if (DecodeLatencyTestColorDetectedMessage(&message, pData, length))     
        {
            processed = true;
            onLatencyTestColorDetectedMessage(&message);
        }
    }

    if (!processed)
    {
        LatencyTestStartedMessage message; 
        if (DecodeLatencyTestStartedMessage(&message, pData, length))     
        {
            processed = true;
            onLatencyTestStartedMessage(&message);
        }
    }

    if (!processed)
    {
        LatencyTestButtonMessage message; 
        if (DecodeLatencyTestButtonMessage(&message, pData, length))     
        {
            processed = true;
            onLatencyTestButtonMessage(&message);
        }
    }
}

bool LatencyTestDeviceImpl::SetConfiguration(const OVR::LatencyTestConfiguration& configuration, bool waitFlag)
{  
    bool                result = false;
    ThreadCommandQueue* queue = GetManagerImpl()->GetThreadQueue();

    if (GetManagerImpl()->GetThreadId() != OVR::GetCurrentThreadId())
    {
        if (!waitFlag)
        {
            return queue->PushCall(this, &LatencyTestDeviceImpl::setConfiguration, configuration);
        }

        if (!queue->PushCallAndWaitResult(  this, 
            &LatencyTestDeviceImpl::setConfiguration,
            &result, 
            configuration))
        {
            return false;
        }
    }
    else
        return setConfiguration(configuration);

    return result;
}

bool LatencyTestDeviceImpl::setConfiguration(const OVR::LatencyTestConfiguration& configuration)
{
    LatencyTestConfigurationImpl ltc(configuration);
    return GetInternalDevice()->SetFeatureReport(ltc.Buffer, LatencyTestConfigurationImpl::PacketSize);
}

bool LatencyTestDeviceImpl::GetConfiguration(OVR::LatencyTestConfiguration* configuration)
{  
    bool result = false;

	ThreadCommandQueue* pQueue = this->GetManagerImpl()->GetThreadQueue();
    if (!pQueue->PushCallAndWaitResult(this, &LatencyTestDeviceImpl::getConfiguration, &result, configuration))
        return false;

    return result;
}

bool LatencyTestDeviceImpl::getConfiguration(OVR::LatencyTestConfiguration* configuration)
{
    LatencyTestConfigurationImpl ltc(*configuration);
    if (GetInternalDevice()->GetFeatureReport(ltc.Buffer, LatencyTestConfigurationImpl::PacketSize))
    {
        ltc.Unpack();
        *configuration = ltc.Configuration;
        return true;
    }

    return false;
}

bool LatencyTestDeviceImpl::SetCalibrate(const Color& calibrationColor, bool waitFlag)
{
    bool                result = false;
    ThreadCommandQueue* queue = GetManagerImpl()->GetThreadQueue();

    if (!waitFlag)
    {
        return queue->PushCall(this, &LatencyTestDeviceImpl::setCalibrate, calibrationColor);
    }

    if (!queue->PushCallAndWaitResult(  this, 
                                        &LatencyTestDeviceImpl::setCalibrate,
                                        &result, 
                                        calibrationColor))
    {
        return false;
    }

    return result;
}

bool LatencyTestDeviceImpl::setCalibrate(const Color& calibrationColor)
{
    LatencyTestCalibrateImpl ltc(calibrationColor);
    return GetInternalDevice()->SetFeatureReport(ltc.Buffer, LatencyTestCalibrateImpl::PacketSize);
}

bool LatencyTestDeviceImpl::SetStartTest(const Color& targetColor, bool waitFlag)
{
    bool                result = false;
    ThreadCommandQueue* queue = GetManagerImpl()->GetThreadQueue();

    if (!waitFlag)
    {
        return queue->PushCall(this, &LatencyTestDeviceImpl::setStartTest, targetColor);
    }

    if (!queue->PushCallAndWaitResult(  this, 
                                        &LatencyTestDeviceImpl::setStartTest,
                                        &result, 
                                        targetColor))
    {
        return false;
    }

    return result;
}

bool LatencyTestDeviceImpl::setStartTest(const Color& targetColor)
{
    LatencyTestStartTestImpl ltst(targetColor);
    return GetInternalDevice()->SetFeatureReport(ltst.Buffer, LatencyTestStartTestImpl::PacketSize);
}

bool LatencyTestDeviceImpl::SetDisplay(const OVR::LatencyTestDisplay& display, bool waitFlag)
{
    bool                 result = false;
    ThreadCommandQueue * queue = GetManagerImpl()->GetThreadQueue();

    if (!waitFlag)
    {
        return queue->PushCall(this, &LatencyTestDeviceImpl::setDisplay, display);
    }

    if (!queue->PushCallAndWaitResult(  this, 
                                        &LatencyTestDeviceImpl::setDisplay,
                                        &result, 
                                        display))
    {
        return false;
    }

    return result;
}

bool LatencyTestDeviceImpl::setDisplay(const OVR::LatencyTestDisplay& display)
{
    LatencyTestDisplayImpl ltd(display);
    return GetInternalDevice()->SetFeatureReport(ltd.Buffer, LatencyTestDisplayImpl::PacketSize);
}

void LatencyTestDeviceImpl::onLatencyTestSamplesMessage(LatencyTestSamplesMessage* message)
{

    if (message->Type != LatencyTestMessage_Samples)
        return;

    LatencyTestSamples& s = message->Samples;

    // Call OnMessage() within a lock to avoid conflicts with handlers.
    Lock::Locker scopeLock(HandlerRef.GetLock());
  
    if (HandlerRef.GetHandler())
    {
        MessageLatencyTestSamples samples(this);
        for (UByte i = 0; i < s.SampleCount; i++)
        {            
            samples.Samples.PushBack(Color(s.Samples[i].Value[0], s.Samples[i].Value[1], s.Samples[i].Value[2]));
        }

        HandlerRef.GetHandler()->OnMessage(samples);
    }
}

void LatencyTestDeviceImpl::onLatencyTestColorDetectedMessage(LatencyTestColorDetectedMessage* message)
{
    if (message->Type != LatencyTestMessage_ColorDetected)
        return;

    LatencyTestColorDetected& s = message->ColorDetected;

    // Call OnMessage() within a lock to avoid conflicts with handlers.
    Lock::Locker scopeLock(HandlerRef.GetLock());

    if (HandlerRef.GetHandler())
    {
        MessageLatencyTestColorDetected detected(this);
        detected.Elapsed = s.Elapsed;
        detected.DetectedValue = Color(s.TriggerValue[0], s.TriggerValue[1], s.TriggerValue[2]);
        detected.TargetValue = Color(s.TargetValue[0], s.TargetValue[1], s.TargetValue[2]);

        HandlerRef.GetHandler()->OnMessage(detected);
    }
}

void LatencyTestDeviceImpl::onLatencyTestStartedMessage(LatencyTestStartedMessage* message)
{
    if (message->Type != LatencyTestMessage_TestStarted)
        return;

    LatencyTestStarted& ts = message->TestStarted;

    // Call OnMessage() within a lock to avoid conflicts with handlers.
    Lock::Locker scopeLock(HandlerRef.GetLock());

    if (HandlerRef.GetHandler())
    {
        MessageLatencyTestStarted started(this);
        started.TargetValue = Color(ts.TargetValue[0], ts.TargetValue[1], ts.TargetValue[2]);

        HandlerRef.GetHandler()->OnMessage(started);
    }
}

void LatencyTestDeviceImpl::onLatencyTestButtonMessage(LatencyTestButtonMessage* message)
{
    if (message->Type != LatencyTestMessage_Button)
        return;

//  LatencyTestButton& s = message->Button;

    // Call OnMessage() within a lock to avoid conflicts with handlers.
    Lock::Locker scopeLock(HandlerRef.GetLock());

    if (HandlerRef.GetHandler())
    {
        MessageLatencyTestButton button(this);

        HandlerRef.GetHandler()->OnMessage(button);
    }
}

} // namespace OVR
