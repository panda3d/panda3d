/************************************************************************************
Filename    :   OVR_OSX_HIDDevice.cpp
Content     :   OSX HID device implementation.
Created     :   February 26, 2013
Authors     :   Lee Cooper
 
Copyright   :   Copyright 2013 Oculus VR, Inc. All Rights reserved.
 
Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#include "OVR_OSX_HIDDevice.h"

#include <IOKit/usb/IOUSBLib.h>

namespace OVR { namespace OSX {

static const UInt32 MAX_QUEUED_INPUT_REPORTS = 5;
    
//-------------------------------------------------------------------------------------
// **** OSX::DeviceManager

HIDDeviceManager::HIDDeviceManager(DeviceManager* manager)
 :  DevManager(manager)
{
    HIDManager = NULL;
}

HIDDeviceManager::~HIDDeviceManager()
{
}

CFRunLoopRef HIDDeviceManager::getRunLoop()
{
    if (DevManager != NULL)
    {
        return DevManager->pThread->GetRunLoop();
    }

    return CFRunLoopGetCurrent();
}

bool HIDDeviceManager::initializeManager()
{
    if (HIDManager != NULL)
    {
        return true;
    }
    
	HIDManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    
    if (!HIDManager)
    {
        return false;
    }
    
    // Create a Matching Dictionary
    CFMutableDictionaryRef matchDict =
        CFDictionaryCreateMutable(kCFAllocatorDefault,
                                  2,
                                  &kCFTypeDictionaryKeyCallBacks,
                                  &kCFTypeDictionaryValueCallBacks);
    
    // Specify a device manufacturer in the Matching Dictionary
    UInt32 vendorId = Oculus_VendorId;
    CFNumberRef vendorIdRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &vendorId);
    CFDictionarySetValue(matchDict,
                         CFSTR(kIOHIDVendorIDKey),
                         vendorIdRef);
    // Register the Matching Dictionary to the HID Manager
    IOHIDManagerSetDeviceMatching(HIDManager, matchDict);
    CFRelease(vendorIdRef);
    CFRelease(matchDict);
    
    // Register a callback for USB device detection with the HID Manager
    IOHIDManagerRegisterDeviceMatchingCallback(HIDManager, &staticDeviceMatchingCallback, this);
    
    IOHIDManagerScheduleWithRunLoop(HIDManager, getRunLoop(), kCFRunLoopDefaultMode);

    return true;
}
    
bool HIDDeviceManager::Initialize()
{
    return initializeManager();
}

void HIDDeviceManager::Shutdown()
{
    OVR_ASSERT_LOG(HIDManager, ("Should have called 'Initialize' before 'Shutdown'."));
    CFRelease(HIDManager);
    
    LogText("OVR::OSX::HIDDeviceManager - shutting down.\n");
}
    
bool HIDDeviceManager::getIntProperty(IOHIDDeviceRef device, CFStringRef propertyName, SInt32* pResult)
{
    
    CFTypeRef ref = IOHIDDeviceGetProperty(device, propertyName);

    if (!ref)
    {
        return false;
    }
    
    if (CFGetTypeID(ref) != CFNumberGetTypeID())
    {
        return false;
    }
    
    CFNumberGetValue((CFNumberRef) ref, kCFNumberSInt32Type, pResult);

    return true;
}
    
bool HIDDeviceManager::initVendorProductVersion(IOHIDDeviceRef device, HIDDeviceDesc* pDevDesc)
{
    
    if (!getVendorId(device, &(pDevDesc->VendorId)))
    {
        return false;
    }
    
    if (!getProductId(device, &(pDevDesc->ProductId)))
    {
        return false;
    }
    
    return true;
}

bool HIDDeviceManager::initUsage(IOHIDDeviceRef device, HIDDeviceDesc* pDevDesc)
{
    
    SInt32 result;
    
    if (!getIntProperty(device, CFSTR(kIOHIDPrimaryUsagePageKey), &result))
    {
        return false;
    }
    
    pDevDesc->UsagePage = result;

    
    if (!getIntProperty(device, CFSTR(kIOHIDPrimaryUsageKey), &result))
    {
        return false;
    }
    
    pDevDesc->Usage = result;
    
    return true;
}

bool HIDDeviceManager::initSerialNumber(IOHIDDeviceRef device, HIDDeviceDesc* pDevDesc)
{
    return getSerialNumberString(device, &(pDevDesc->SerialNumber));
}
    
bool HIDDeviceManager::initStrings(IOHIDDeviceRef device, HIDDeviceDesc* pDevDesc)
{

    // Regardless of whether they fail we'll try and get the remaining.
    getStringProperty(device, CFSTR(kIOHIDManufacturerKey), &(pDevDesc->Manufacturer));
    getStringProperty(device, CFSTR(kIOHIDProductKey), &(pDevDesc->Product));
    
    return true;
}

bool HIDDeviceManager::getStringProperty(IOHIDDeviceRef device,
                                         CFStringRef propertyName,
                                         String* pResult)
{
    
    CFStringRef str = (CFStringRef) IOHIDDeviceGetProperty(device, propertyName);
    
    if (!str)
    {
        return false;
    }

    CFIndex length = CFStringGetLength(str);
    CFRange range = CFRangeMake(0, length);
    
    // Test the conversion first to get required buffer size.
    CFIndex bufferLength;
    CFIndex numberOfChars = CFStringGetBytes(str,
                                             range,
                                             kCFStringEncodingUTF8,
                                             (char) '?',
                                             FALSE,
                                             NULL,
                                             0,
                                             &bufferLength);
    
    if (numberOfChars == 0)
    {
        return false;
    }
    
    // Now allocate buffer.
    char* buffer = new char[bufferLength+1];
    
    numberOfChars = CFStringGetBytes(str,
                                     range,
                                     kCFStringEncodingUTF8,
                                     (char) '?',
                                     FALSE,
                                     (UInt8*) buffer,
                                     bufferLength,
                                     NULL);
    OVR_ASSERT_LOG(numberOfChars != 0, ("CFStringGetBytes failed."));

    buffer[bufferLength] = '\0';
    *pResult = String(buffer);
    
    return true;
}
    
bool HIDDeviceManager::getVendorId(IOHIDDeviceRef device, UInt16* pResult)
{
    SInt32 result;
    
    if (!getIntProperty(device, CFSTR(kIOHIDVendorIDKey), &result))
    {
        return false;
    }
    
    *pResult = result;

    return true;
}
    
bool HIDDeviceManager::getProductId(IOHIDDeviceRef device, UInt16* pResult)
{
    SInt32 result;
    
    if (!getIntProperty(device, CFSTR(kIOHIDProductIDKey), &result))
    {
        return false;
    }
    
    *pResult = result;
    
    return true;
}
 
bool HIDDeviceManager::getLocationId(IOHIDDeviceRef device, SInt32* pResult)
{
    SInt32 result;
    
    if (!getIntProperty(device, CFSTR(kIOHIDLocationIDKey), &result))
    {
        return false;
    }
        
    *pResult = result;
        
    return true;
}
    
bool HIDDeviceManager::getSerialNumberString(IOHIDDeviceRef device, String* pResult)
{
 
    if (!getStringProperty(device, CFSTR(kIOHIDSerialNumberKey), pResult))
    {
        return false;
    }

    return true;
}
    
bool HIDDeviceManager::getPath(IOHIDDeviceRef device, String* pPath)
{

    String transport;
    if (!getStringProperty(device, CFSTR(kIOHIDTransportKey), &transport))
    {
        return false;
    }
    
    UInt16 vendorId;
    if (!getVendorId(device, &vendorId))
    {
        return false;
    }

    UInt16 productId;
    if (!getProductId(device, &productId))
    {
        return false;
    }
    
    String serialNumber;
	if (!getSerialNumberString(device, &serialNumber))
    {
        return false;
    }
    

    StringBuffer buffer;
    buffer.AppendFormat("%s:vid=%04hx:pid=%04hx:ser=%s",
                            transport.ToCStr(),
                            vendorId,
                            productId,
                            serialNumber.ToCStr());
    
    *pPath = String(buffer);
    
    return true;
}

bool HIDDeviceManager::Enumerate(HIDEnumerateVisitor* enumVisitor)
{
    if (!initializeManager())
    {
        return false;
    }
    

	CFSetRef deviceSet = IOHIDManagerCopyDevices(HIDManager);
    if (!deviceSet)
        return false;
    
	CFIndex deviceCount = CFSetGetCount(deviceSet);
    
    // Allocate a block of memory and read the set into it.
    IOHIDDeviceRef* devices = (IOHIDDeviceRef*) OVR_ALLOC(sizeof(IOHIDDeviceRef) * deviceCount);
    CFSetGetValues(deviceSet, (const void **) devices);
    

    // Iterate over devices.
    for (CFIndex deviceIndex = 0; deviceIndex < deviceCount; deviceIndex++)
    {
        IOHIDDeviceRef hidDev = devices[deviceIndex];
        
        if (!hidDev)
        {
            continue;
        }
        
        HIDDeviceDesc devDesc;
                
        if (getPath(hidDev, &(devDesc.Path)) &&
            initVendorProductVersion(hidDev, &devDesc) &&
            enumVisitor->MatchVendorProduct(devDesc.VendorId, devDesc.ProductId) &&
            initUsage(hidDev, &devDesc))
        {
            initStrings(hidDev, &devDesc);
            initSerialNumber(hidDev, &devDesc);

            // Look for the device to check if it is already opened.
            Ptr<DeviceCreateDesc> existingDevice = DevManager->FindHIDDevice(devDesc);
            // if device exists and it is opened then most likely the CreateHIDFile
            // will fail; therefore, we just set Enumerated to 'true' and continue.
            if (existingDevice && existingDevice->pDevice)
            {
                existingDevice->Enumerated = true;
                continue;
            }
            
            // Construct minimal device that the visitor callback can get feature reports from.
            OSX::HIDDevice device(this, hidDev);
            
            enumVisitor->Visit(device, devDesc);
        }
    }
    
    OVR_FREE(devices);
    CFRelease(deviceSet);
    
    return true;
}

OVR::HIDDevice* HIDDeviceManager::Open(const String& path)
{

    Ptr<OSX::HIDDevice> device = *new OSX::HIDDevice(this);

    if (!device->HIDInitialize(path))
    {
        return NULL;
    }

    device->AddRef();
    
    return device;
}
    
bool HIDDeviceManager::getFullDesc(IOHIDDeviceRef device, HIDDeviceDesc* desc)
{
        
    if (!initVendorProductVersion(device, desc))
    {
        return false;
    }
        
    if (!initUsage(device, desc))
    {
        return false;
    }
    
    if (!initSerialNumber(device, desc))
    {
        return false;
    }
    
    initStrings(device, desc);
        
    return true;
}

// New USB device specified in the matching dictionary has been added (callback function)
void HIDDeviceManager::staticDeviceMatchingCallback(void *inContext,
                                                    IOReturn inResult,
                                                    void *inSender,
                                                    IOHIDDeviceRef inIOHIDDeviceRef)
{
    HIDDeviceManager* hidMgr = static_cast<HIDDeviceManager*>(inContext);
    HIDDeviceDesc hidDevDesc;
    hidMgr->getPath(inIOHIDDeviceRef, &hidDevDesc.Path);
    hidMgr->getFullDesc(inIOHIDDeviceRef, &hidDevDesc);
    
    hidMgr->DevManager->DetectHIDDevice(hidDevDesc);
}

//-------------------------------------------------------------------------------------
// **** OSX::HIDDevice

HIDDevice::HIDDevice(HIDDeviceManager* manager)
 :  HIDManager(manager), InMinimalMode(false)
{
    Device = NULL;
    RepluggedNotificationPort = 0;
}
    
// This is a minimal constructor used during enumeration for us to pass
// a HIDDevice to the visit function (so that it can query feature reports).
HIDDevice::HIDDevice(HIDDeviceManager* manager, IOHIDDeviceRef device)
:   HIDManager(manager), Device(device), InMinimalMode(true)
{
    RepluggedNotificationPort = 0;
}

HIDDevice::~HIDDevice()
{
    if (!InMinimalMode)
    {
        HIDShutdown();
    }
}

bool HIDDevice::HIDInitialize(const String& path)
{

    DevDesc.Path = path;

    if (!openDevice())
    {
        LogText("OVR::OSX::HIDDevice - Failed to open HIDDevice: %s", path.ToCStr());
        return false;
    }

    // Setup notification for when a device is unplugged and plugged back in.
    if (!setupDevicePluggedInNotification())
    {
        LogText("OVR::OSX::HIDDevice - Failed to setup notification for when device plugged back in.");
        closeDevice(false);
        return false;
    }
    
    HIDManager->DevManager->pThread->AddTicksNotifier(this);

    
    LogText("OVR::OSX::HIDDevice - Opened '%s'\n"
            "                    Manufacturer:'%s'  Product:'%s'  Serial#:'%s'\n",
            DevDesc.Path.ToCStr(),
            DevDesc.Manufacturer.ToCStr(), DevDesc.Product.ToCStr(),
            DevDesc.SerialNumber.ToCStr());
    
    return true;
}

bool HIDDevice::initInfo()
{
    // Device must have been successfully opened.
    OVR_ASSERT(Device);
    
    
    // Get report lengths.
    SInt32 bufferLength;
    bool getResult = HIDManager->getIntProperty(Device, CFSTR(kIOHIDMaxInputReportSizeKey), &bufferLength);
    OVR_ASSERT(getResult);
    InputReportBufferLength = (UInt16) bufferLength;

    getResult = HIDManager->getIntProperty(Device, CFSTR(kIOHIDMaxOutputReportSizeKey), &bufferLength);
    OVR_ASSERT(getResult);
    OutputReportBufferLength = (UInt16) bufferLength;

    getResult = HIDManager->getIntProperty(Device, CFSTR(kIOHIDMaxFeatureReportSizeKey), &bufferLength);
    OVR_ASSERT(getResult);
    FeatureReportBufferLength = (UInt16) bufferLength;
    
    
    if (ReadBufferSize < InputReportBufferLength)
    {
        OVR_ASSERT_LOG(false, ("Input report buffer length is bigger than read buffer."));
        return false;
    }
    
    // Get device desc.
    if (!HIDManager->getFullDesc(Device, &DevDesc))
    {
        OVR_ASSERT_LOG(false, ("Failed to get device desc while initializing device."));
        return false;
    }
    
    return true;
}

void HIDDevice::staticDeviceAddedCallback(void* pContext, io_iterator_t iterator)
{
    HIDDevice* pDevice = (HIDDevice*) pContext;
    pDevice->deviceAddedCallback(iterator);
}

void HIDDevice::deviceAddedCallback(io_iterator_t iterator)
{

    if (Device == NULL)
    {
        if (openDevice())
        {
            LogText("OVR::OSX::HIDDevice - Reopened device : %s", DevDesc.Path.ToCStr());

            Ptr<DeviceCreateDesc> existingHIDDev = HIDManager->DevManager->FindHIDDevice(DevDesc);
            if (existingHIDDev && existingHIDDev->pDevice)
            {
                HIDManager->DevManager->CallOnDeviceAdded(existingHIDDev);
            }
        }
    }

    // Reset callback.
    while (IOIteratorNext(iterator))
        ;
}
    
bool HIDDevice::openDevice()
{
    
    // Have to iterate through devices again to generate paths.
	CFSetRef deviceSet = IOHIDManagerCopyDevices(HIDManager->HIDManager);
	CFIndex deviceCount = CFSetGetCount(deviceSet);
    
    // Allocate a block of memory and read the set into it.
    IOHIDDeviceRef* devices = (IOHIDDeviceRef*) OVR_ALLOC(sizeof(IOHIDDeviceRef) * deviceCount);
    CFSetGetValues(deviceSet, (const void **) devices);
    
    
    // Iterate over devices.
    IOHIDDeviceRef device = NULL;

    for (CFIndex deviceIndex = 0; deviceIndex < deviceCount; deviceIndex++)
    {
        IOHIDDeviceRef tmpDevice = devices[deviceIndex];
        
        if (!tmpDevice)
        {
            continue;
        }
        
        String path;
        if (!HIDManager->getPath(tmpDevice, &path))
        {
            continue;
        }
        
        if (path == DevDesc.Path)
        {
            device = tmpDevice;
            break;
        }
    }
    
    
    OVR_FREE(devices);
    
    if (!device)
    {
        CFRelease(deviceSet);
        return false;
    }
    
    // Attempt to open device.
    if (IOHIDDeviceOpen(device, kIOHIDOptionsTypeSeizeDevice)
        != kIOReturnSuccess)
    {
        CFRelease(deviceSet);
        return false;
    }

    // Retain the device before we release the set.
    CFRetain(device);
    CFRelease(deviceSet);
    
    
    Device = device;

    
    if (!initInfo())
    {
        IOHIDDeviceClose(Device, kIOHIDOptionsTypeSeizeDevice);
        CFRelease(Device);
        Device = NULL;
        return false;
    }
    
    
    // Setup the Run Loop and callbacks.
    IOHIDDeviceScheduleWithRunLoop(Device,
                                   HIDManager->getRunLoop(),
                                   kCFRunLoopDefaultMode);
    
    IOHIDDeviceRegisterInputReportCallback(Device,
                                           ReadBuffer,
                                           ReadBufferSize,
                                           staticHIDReportCallback,
                                           this);

    IOHIDDeviceRegisterRemovalCallback(Device,
                                       staticDeviceRemovedCallback,
                                       this);
    
    return true;
}
    
void HIDDevice::HIDShutdown()
{

    HIDManager->DevManager->pThread->RemoveTicksNotifier(this);
    
    if (Device != NULL) // Device may already have been closed if unplugged.
    {
        closeDevice(false);
    }

    IOObjectRelease(RepluggedNotification);
    if (RepluggedNotificationPort)
        IONotificationPortDestroy(RepluggedNotificationPort);
    
    LogText("OVR::OSX::HIDDevice - HIDShutdown '%s'\n", DevDesc.Path.ToCStr());
}

bool HIDDevice::setupDevicePluggedInNotification()
{
    
    // Setup notification when devices are plugged in.
    RepluggedNotificationPort = IONotificationPortCreate(kIOMasterPortDefault);
    
    CFRunLoopSourceRef notificationRunLoopSource =
        IONotificationPortGetRunLoopSource(RepluggedNotificationPort);
    
    CFRunLoopAddSource(HIDManager->getRunLoop(),
                       notificationRunLoopSource,
                       kCFRunLoopDefaultMode);
    
    CFMutableDictionaryRef matchingDict = IOServiceMatching(kIOUSBDeviceClassName);
    
    // Have to specify vendorId and productId. Doesn't seem to accept additional
    // things like serial number.
    SInt32 vendorId = DevDesc.VendorId;
    CFNumberRef numberRef = CFNumberCreate(kCFAllocatorDefault,
                                           kCFNumberSInt32Type,
                                           &vendorId);
    CFDictionarySetValue(matchingDict, CFSTR(kUSBVendorID), numberRef);
    CFRelease(numberRef);
    
    SInt32 deviceProductId = DevDesc.ProductId;
    numberRef = CFNumberCreate(kCFAllocatorDefault,
                               kCFNumberSInt32Type,
                               &deviceProductId);
    CFDictionarySetValue(matchingDict, CFSTR(kUSBProductID), numberRef);
    CFRelease(numberRef);
    
    kern_return_t result =
            IOServiceAddMatchingNotification(RepluggedNotificationPort,
                                             kIOMatchedNotification,
                                             matchingDict,
                                             staticDeviceAddedCallback,
                                             this,
                                             &RepluggedNotification);
    
    if (result != KERN_SUCCESS)
    {
        CFRelease(RepluggedNotificationPort);
        RepluggedNotificationPort = 0;
        return false;
    }
    
    // Iterate through to arm.
    while (IOIteratorNext(RepluggedNotification))
    {
	}
    
    return true;
}

void HIDDevice::closeDevice(bool wasUnplugged)
{
    OVR_ASSERT(Device != NULL);
    
    if (!wasUnplugged)
    {
        // Clear the registered callbacks.
        IOHIDDeviceRegisterInputReportCallback(Device,
                                               ReadBuffer,
                                               InputReportBufferLength,
                                               NULL,
                                               this);
        
        IOHIDDeviceRegisterRemovalCallback(Device, NULL, this);
        
        IOHIDDeviceUnscheduleFromRunLoop(Device,
                                         HIDManager->getRunLoop(),
                                         kCFRunLoopDefaultMode);
        IOHIDDeviceClose(Device, kIOHIDOptionsTypeNone);
    }
    
	CFRelease(Device);
    Device = NULL;
        
    LogText("OVR::OSX::HIDDevice - HID Device Closed '%s'\n", DevDesc.Path.ToCStr());
}

void HIDDevice::staticHIDReportCallback(void* pContext,
                                        IOReturn result,
                                        void* pSender,
                                        IOHIDReportType reportType,
                                        uint32_t reportId,
                                        uint8_t* pReport,
                                        CFIndex reportLength)
{
    HIDDevice* pDevice = (HIDDevice*) pContext;
    return pDevice->hidReportCallback(pReport, (UInt32)reportLength);
}

void HIDDevice::hidReportCallback(UByte* pData, UInt32 length)
{
    
    // We got data.
    if (Handler)
    {
        Handler->OnInputReport(pData, length);
    }
}
    
void HIDDevice::staticDeviceRemovedCallback(void* pContext, IOReturn result, void* pSender)
{
    HIDDevice* pDevice = (HIDDevice*) pContext;
    pDevice->deviceRemovedCallback();
}
    
void HIDDevice::deviceRemovedCallback()
{
    Ptr<HIDDevice> _this(this); // prevent from release
    
    Ptr<DeviceCreateDesc> existingHIDDev = HIDManager->DevManager->FindHIDDevice(DevDesc);
    if (existingHIDDev && existingHIDDev->pDevice)
    {
        HIDManager->DevManager->CallOnDeviceRemoved(existingHIDDev);
    }
    closeDevice(true);
}

CFStringRef HIDDevice::generateRunLoopModeString(IOHIDDeviceRef device)
{
    const UInt32 safeBuffSize = 256;
    char nameBuff[safeBuffSize];
    OVR_sprintf(nameBuff, safeBuffSize, "%016lX", device);
  
    return CFStringCreateWithCString(NULL, nameBuff, kCFStringEncodingASCII);
}
    
bool HIDDevice::SetFeatureReport(UByte* data, UInt32 length)
{
    
    if (!Device)
        return false;
    
    UByte reportID = data[0];

    if (reportID == 0)
    {
        // Not using reports so remove from data packet.
        data++;
        length--;
    }
    
	IOReturn result = IOHIDDeviceSetReport( Device,
                                            kIOHIDReportTypeFeature,
                                            reportID,
                                            data,
                                            length);
    
    return (result == kIOReturnSuccess);
}

bool HIDDevice::GetFeatureReport(UByte* data, UInt32 length)
{
    if (!Device)
        return false;
    
    CFIndex bufferLength = length;
    
    // Report id is in first byte of the buffer.
	IOReturn result = IOHIDDeviceGetReport(Device, kIOHIDReportTypeFeature, data[0], data, &bufferLength);
	
    return (result == kIOReturnSuccess);
}
   
UInt64 HIDDevice::OnTicks(UInt64 ticksMks)
{
    
    if (Handler)
    {
        return Handler->OnTicks(ticksMks);
    }
    
    return DeviceManagerThread::Notifier::OnTicks(ticksMks);
}

HIDDeviceManager* HIDDeviceManager::CreateInternal(OSX::DeviceManager* devManager)
{
        
    if (!System::IsInitialized())
    {
        // Use custom message, since Log is not yet installed.
        OVR_DEBUG_STATEMENT(Log::GetDefaultLog()->
                            LogMessage(Log_Debug, "HIDDeviceManager::Create failed - OVR::System not initialized"); );
        return 0;
    }

    Ptr<OSX::HIDDeviceManager> manager = *new OSX::HIDDeviceManager(devManager);

    if (manager)
    {
        if (manager->Initialize())
        {
            manager->AddRef();
        }
        else
        {
            manager.Clear();
        }
    }

    return manager.GetPtr();
}
    
} // namespace OSX

//-------------------------------------------------------------------------------------
// ***** Creation

// Creates a new HIDDeviceManager and initializes OVR.
HIDDeviceManager* HIDDeviceManager::Create()
{
    OVR_ASSERT_LOG(false, ("Standalone mode not implemented yet."));
    
    if (!System::IsInitialized())
    {
        // Use custom message, since Log is not yet installed.
        OVR_DEBUG_STATEMENT(Log::GetDefaultLog()->
            LogMessage(Log_Debug, "HIDDeviceManager::Create failed - OVR::System not initialized"); );
        return 0;
    }

    Ptr<OSX::HIDDeviceManager> manager = *new OSX::HIDDeviceManager(NULL);
    
    if (manager)
    {
        if (manager->Initialize())
        {
            manager->AddRef();
        }
        else
        {
            manager.Clear();
        }
    }

    return manager.GetPtr();
}

} // namespace OVR
