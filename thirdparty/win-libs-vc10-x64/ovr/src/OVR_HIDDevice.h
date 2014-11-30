/************************************************************************************

Filename    :   OVR_HIDDevice.h
Content     :   Cross platform HID device interface.
Created     :   February 22, 2013
Authors     :   Lee Cooper

Copyright   :   Copyright 2013 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#ifndef OVR_HIDDevice_h
#define OVR_HIDDevice_h

#include "OVR_HIDDeviceBase.h"

#include "Kernel/OVR_RefCount.h"
#include "Kernel/OVR_String.h"
#include "Kernel/OVR_Timer.h"

namespace OVR {

class HIDDevice;
class DeviceManager;

// HIDDeviceDesc contains interesting attributes of a HID device, including a Path
// that can be used to create it.
struct HIDDeviceDesc
{
    UInt16  VendorId;
    UInt16  ProductId;
    UInt16  VersionNumber;
    UInt16  Usage;
    UInt16  UsagePage;
    String  Path;           // Platform specific.
    String  Manufacturer;
    String  Product;
    String  SerialNumber;
};

// HIDEnumerateVisitor exposes a Visit interface called for every detected device
// by HIDDeviceManager::Enumerate. 
class HIDEnumerateVisitor
{
public:

    // Should return true if we are interested in supporting
    // this HID VendorId and ProductId pair.
    virtual bool MatchVendorProduct(UInt16 vendorId, UInt16 productId)
    { OVR_UNUSED2(vendorId, productId); return true; }

    // Override to get notified about available device. Will only be called for
    // devices that matched MatchVendorProduct.
    virtual void Visit(HIDDevice&, const HIDDeviceDesc&) { }
};


//-------------------------------------------------------------------------------------
// ***** HIDDeviceManager

// Internal manager for enumerating and opening HID devices.
// If an OVR::DeviceManager is created then an OVR::HIDDeviceManager will automatically be created and can be accessed from the
// DeviceManager by calling 'GetHIDDeviceManager()'. When using HIDDeviceManager in standalone mode, the client must call
// 'Create' below.
class HIDDeviceManager : public RefCountBase<HIDDeviceManager>
{
public:

    // Creates a new HIDDeviceManager. Only one instance of HIDDeviceManager should be created at a time.
    static HIDDeviceManager* Create();

    // Enumerate HID devices using a HIDEnumerateVisitor derived visitor class.
    virtual bool Enumerate(HIDEnumerateVisitor* enumVisitor) = 0;

    // Open a HID device with the specified path.
    virtual HIDDevice* Open(const String& path) = 0;

protected:
    HIDDeviceManager()
    { }
};

//-------------------------------------------------------------------------------------
// ***** HIDDevice

// HID device object. This is designed to be operated in synchronous
// and asynchronous modes. With no handler set, input messages will be
// stored and can be retrieved by calling 'Read' or 'ReadBlocking'.
class HIDDevice : public RefCountBase<HIDDevice>, public HIDDeviceBase
{
public:

    HIDDevice()
     :  Handler(NULL)
    {
    }

    virtual ~HIDDevice() {}

    virtual bool SetFeatureReport(UByte* data, UInt32 length) = 0;
    virtual bool GetFeatureReport(UByte* data, UInt32 length) = 0;

// Not yet implemented.
/*
    virtual bool Write(UByte* data, UInt32 length) = 0;

    virtual bool Read(UByte* pData, UInt32 length, UInt32 timeoutMilliS) = 0;
    virtual bool ReadBlocking(UByte* pData, UInt32 length) = 0;
*/

    class HIDHandler
    {
    public:
        virtual void OnInputReport(UByte* pData, UInt32 length)
        { OVR_UNUSED2(pData, length); }

        virtual UInt64 OnTicks(UInt64 ticksMks)
        { OVR_UNUSED1(ticksMks);  return Timer::MksPerSecond * 1000; ; }

        enum HIDDeviceMessageType
        {
            HIDDeviceMessage_DeviceAdded    = 0,
            HIDDeviceMessage_DeviceRemoved  = 1
        };

        virtual void OnDeviceMessage(HIDDeviceMessageType messageType) 
        { OVR_UNUSED1(messageType); }
    };

    void SetHandler(HIDHandler* handler)
    { Handler = handler; }

protected:
    HIDHandler* Handler;
};

} // namespace OVR

#endif
