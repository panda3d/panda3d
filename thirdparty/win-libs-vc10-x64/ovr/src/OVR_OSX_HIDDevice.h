/************************************************************************************
Filename    :   OVR_OSX_HIDDevice.h
Content     :   OSX HID device implementation.
Created     :   February 26, 2013
Authors     :   Lee Cooper

Copyright   :   Copyright 2013 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#ifndef OVR_OSX_HIDDevice_h
#define OVR_OSX_HIDDevice_h

#include "OVR_HIDDevice.h"

#include "OVR_OSX_DeviceManager.h"

#include <IOKit/IOKitLib.h>

namespace OVR { namespace OSX {

class HIDDeviceManager;

//-------------------------------------------------------------------------------------
// ***** OSX HIDDevice

class HIDDevice : public OVR::HIDDevice, public DeviceManagerThread::Notifier
{
private:
    friend class HIDDeviceManager;

public:
    HIDDevice(HIDDeviceManager* manager);

    // This is a minimal constructor used during enumeration for us to pass
    // a HIDDevice to the visit function (so that it can query feature reports).
    HIDDevice(HIDDeviceManager* manager, IOHIDDeviceRef device);
    
    virtual ~HIDDevice();

    bool HIDInitialize(const String& path);
    void HIDShutdown();
    
    virtual bool SetFeatureReport(UByte* data, UInt32 length);
	virtual bool GetFeatureReport(UByte* data, UInt32 length);

    bool Write(UByte* data, UInt32 length);

    bool Read(UByte* pData, UInt32 length, UInt32 timeoutMilliS);
    bool ReadBlocking(UByte* pData, UInt32 length);


    // DeviceManagerThread::Notifier
    UInt64 OnTicks(UInt64 ticksMks);
    
private:
    bool initInfo();
    bool openDevice();
    void closeDevice(bool wasUnplugged);
    bool setupDevicePluggedInNotification();
    CFStringRef generateRunLoopModeString(IOHIDDeviceRef device);
    
    static void staticHIDReportCallback(void* pContext,
                                        IOReturn result,
                                        void* pSender,
                                        IOHIDReportType reportType,
                                        uint32_t reportId,
                                        uint8_t* pReport,
                                        CFIndex reportLength);
    void hidReportCallback(UByte* pData, UInt32 length);

    static void staticDeviceRemovedCallback(void* pContext,
                                            IOReturn result,
                                            void* pSender);
    void deviceRemovedCallback();
    
    static void staticDeviceAddedCallback(void* pContext,
                                          io_iterator_t iterator);
    void deviceAddedCallback(io_iterator_t iterator);
    
    bool                    InMinimalMode;
    HIDDeviceManager*       HIDManager;
    IOHIDDeviceRef          Device;
    HIDDeviceDesc           DevDesc;
    
    enum { ReadBufferSize = 96 };
    UByte                   ReadBuffer[ReadBufferSize];

    UInt16                  InputReportBufferLength;
    UInt16                  OutputReportBufferLength;
    UInt16                  FeatureReportBufferLength;
    
    IONotificationPortRef   RepluggedNotificationPort;
    io_iterator_t           RepluggedNotification;
};


//-------------------------------------------------------------------------------------
// ***** OSX HIDDeviceManager

class HIDDeviceManager : public OVR::HIDDeviceManager
{
	friend class HIDDevice;

public:
    HIDDeviceManager(OSX::DeviceManager* Manager);
    virtual ~HIDDeviceManager();

    virtual bool Initialize();
    virtual void Shutdown();

    virtual bool Enumerate(HIDEnumerateVisitor* enumVisitor);
    virtual OVR::HIDDevice* Open(const String& path);

    static HIDDeviceManager* CreateInternal(DeviceManager* manager);
    
private:
    CFRunLoopRef getRunLoop();
    bool initializeManager();
    bool initVendorProductVersion(IOHIDDeviceRef device, HIDDeviceDesc* pDevDesc);
    bool initUsage(IOHIDDeviceRef device, HIDDeviceDesc* pDevDesc);
    bool initStrings(IOHIDDeviceRef device, HIDDeviceDesc* pDevDesc);
    bool initSerialNumber(IOHIDDeviceRef device, HIDDeviceDesc* pDevDesc);
    bool getVendorId(IOHIDDeviceRef device, UInt16* pResult);
    bool getProductId(IOHIDDeviceRef device, UInt16* pResult);
    bool getLocationId(IOHIDDeviceRef device, SInt32* pResult);
    bool getSerialNumberString(IOHIDDeviceRef device, String* pResult);
    bool getPath(IOHIDDeviceRef device, String* pPath);
    bool getIntProperty(IOHIDDeviceRef device, CFStringRef key, int32_t* pResult);
    bool getStringProperty(IOHIDDeviceRef device, CFStringRef propertyName, String* pResult);
    bool getFullDesc(IOHIDDeviceRef device, HIDDeviceDesc* desc);
    
    static void staticDeviceMatchingCallback(void *inContext,
                                             IOReturn inResult,
                                             void *inSender,
                                             IOHIDDeviceRef inIOHIDDeviceRef);
    
    DeviceManager* DevManager;

    IOHIDManagerRef HIDManager;
};

}} // namespace OVR::OSX

#endif // OVR_OSX_HIDDevice_h
