/************************************************************************************

Filename    :   OVR_Win32_HIDDevice.cpp
Content     :   Win32 HID device implementation.
Created     :   February 22, 2013
Authors     :   Lee Cooper

Copyright   :   Copyright 2013 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#include "OVR_Win32_HIDDevice.h"
#include "OVR_Win32_DeviceManager.h"

#include "Kernel/OVR_System.h"
#include "Kernel/OVR_Log.h"

namespace OVR { namespace Win32 {

//-------------------------------------------------------------------------------------
// HIDDevicePathWrapper is a simple class used to extract HID device file path
// through SetupDiGetDeviceInterfaceDetail. We use a class since this is a bit messy.
class HIDDevicePathWrapper
{
    SP_INTERFACE_DEVICE_DETAIL_DATA_A* pData;
public:
    HIDDevicePathWrapper() : pData(0) { }
    ~HIDDevicePathWrapper() { if (pData) OVR_FREE(pData); }

    const char* GetPath() const { return pData ? pData->DevicePath : 0; }

    bool InitPathFromInterfaceData(HDEVINFO hdevInfoSet, SP_DEVICE_INTERFACE_DATA* pidata);
};

bool HIDDevicePathWrapper::InitPathFromInterfaceData(HDEVINFO hdevInfoSet, SP_DEVICE_INTERFACE_DATA* pidata)
{
    DWORD detailSize = 0;
    // SetupDiGetDeviceInterfaceDetailA returns "not enough buffer error code"
    // doe size request. Just check valid size.
    SetupDiGetDeviceInterfaceDetailA(hdevInfoSet, pidata, NULL, 0, &detailSize, NULL);
    if (!detailSize ||
        ((pData = (SP_INTERFACE_DEVICE_DETAIL_DATA_A*)OVR_ALLOC(detailSize)) == 0))
        return false;
    pData->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA_A);

    if (!SetupDiGetDeviceInterfaceDetailA(hdevInfoSet, pidata, pData, detailSize, NULL, NULL))
        return false;
    return true;
}


//-------------------------------------------------------------------------------------
// **** Win32::DeviceManager

HIDDeviceManager::HIDDeviceManager(DeviceManager* manager)
 :  Manager(manager)
{
    hHidLib = ::LoadLibraryA("hid.dll");
    OVR_ASSERT_LOG(hHidLib, ("Couldn't load Win32 'hid.dll'."));

    OVR_RESOLVE_HIDFUNC(HidD_GetHidGuid);
    OVR_RESOLVE_HIDFUNC(HidD_SetNumInputBuffers);
    OVR_RESOLVE_HIDFUNC(HidD_GetFeature);
    OVR_RESOLVE_HIDFUNC(HidD_SetFeature);
    OVR_RESOLVE_HIDFUNC(HidD_GetAttributes);
    OVR_RESOLVE_HIDFUNC(HidD_GetManufacturerString);
    OVR_RESOLVE_HIDFUNC(HidD_GetProductString);
    OVR_RESOLVE_HIDFUNC(HidD_GetSerialNumberString);
    OVR_RESOLVE_HIDFUNC(HidD_GetPreparsedData);   
    OVR_RESOLVE_HIDFUNC(HidD_FreePreparsedData);  
    OVR_RESOLVE_HIDFUNC(HidP_GetCaps);    

    if (HidD_GetHidGuid)
        HidD_GetHidGuid(&HidGuid);
}

HIDDeviceManager::~HIDDeviceManager()
{
    ::FreeLibrary(hHidLib);
}

bool HIDDeviceManager::Initialize()
{
    return true;
}

void HIDDeviceManager::Shutdown()
{   
    LogText("OVR::Win32::HIDDeviceManager - shutting down.\n");
}

bool HIDDeviceManager::Enumerate(HIDEnumerateVisitor* enumVisitor)
{
    HDEVINFO                 hdevInfoSet;
    SP_DEVICE_INTERFACE_DATA interfaceData;
    interfaceData.cbSize = sizeof(interfaceData);

    // Get handle to info data set describing all available HIDs.
    hdevInfoSet = SetupDiGetClassDevsA(&HidGuid, NULL, NULL, DIGCF_INTERFACEDEVICE | DIGCF_PRESENT);
    if (hdevInfoSet == INVALID_HANDLE_VALUE)
        return false;

    for(int deviceIndex = 0;
        SetupDiEnumDeviceInterfaces(hdevInfoSet, NULL, &HidGuid, deviceIndex, &interfaceData);
        deviceIndex++)
    {
        // For each device, we extract its file path and open it to get attributes,
        // such as vendor and product id. If anything goes wrong, we move onto next device.
        HIDDevicePathWrapper pathWrapper;
        if (!pathWrapper.InitPathFromInterfaceData(hdevInfoSet, &interfaceData))
            continue;

        // Look for the device to check if it is already opened.
        Ptr<DeviceCreateDesc> existingDevice = Manager->FindDevice(pathWrapper.GetPath());
        // if device exists and it is opened then most likely the CreateHIDFile
        // will fail; therefore, we just set Enumerated to 'true' and continue.
        if (existingDevice && existingDevice->pDevice)
        {
            existingDevice->Enumerated = true;
            continue;
        }

        // open device in non-exclusive mode for detection...
        HANDLE hidDev = CreateHIDFile(pathWrapper.GetPath(), false);
        if (hidDev == INVALID_HANDLE_VALUE)
            continue;

        HIDDeviceDesc devDesc;
        devDesc.Path = pathWrapper.GetPath();
        if (initVendorProductVersion(hidDev, &devDesc) &&
            enumVisitor->MatchVendorProduct(devDesc.VendorId, devDesc.ProductId) &&
            initUsage(hidDev, &devDesc))
        {
            initStrings(hidDev, &devDesc);

            // Construct minimal device that the visitor callback can get feature reports from.
            Win32::HIDDevice device(this, hidDev);
            enumVisitor->Visit(device, devDesc);
        }

        ::CloseHandle(hidDev);
    }

    SetupDiDestroyDeviceInfoList(hdevInfoSet);
    return true;
}

bool HIDDeviceManager::GetHIDDeviceDesc(const String& path, HIDDeviceDesc* pdevDesc) const
{
    // open device in non-exclusive mode for detection...
    HANDLE hidDev = CreateHIDFile(path, false);
    if (hidDev == INVALID_HANDLE_VALUE)
        return false;

    pdevDesc->Path = path;
    getFullDesc(hidDev, pdevDesc);

    ::CloseHandle(hidDev);
    return true;
}

OVR::HIDDevice* HIDDeviceManager::Open(const String& path)
{

    Ptr<Win32::HIDDevice> device = *new Win32::HIDDevice(this);

    if (device->HIDInitialize(path))
    {
        device->AddRef();        
        return device;
    }

    return NULL;
}

bool HIDDeviceManager::getFullDesc(HANDLE hidDev, HIDDeviceDesc* desc) const
{

    if (!initVendorProductVersion(hidDev, desc))
    {
        return false;
    }

    if (!initUsage(hidDev, desc))
    {
        return false;
    }

    initStrings(hidDev, desc);

    return true;
}

bool HIDDeviceManager::initVendorProductVersion(HANDLE hidDev, HIDDeviceDesc* desc) const
{
    HIDD_ATTRIBUTES attr;
    attr.Size = sizeof(attr);
    if (!HidD_GetAttributes(hidDev, &attr))
        return false;
    desc->VendorId      = attr.VendorID;
    desc->ProductId     = attr.ProductID;
    desc->VersionNumber = attr.VersionNumber;
    return true;
}

bool HIDDeviceManager::initUsage(HANDLE hidDev, HIDDeviceDesc* desc) const
{
    bool                 result = false;
    HIDP_CAPS            caps;
    HIDP_PREPARSED_DATA* preparsedData = 0;

    if (!HidD_GetPreparsedData(hidDev, &preparsedData))
        return false;

    if (HidP_GetCaps(preparsedData, &caps) == HIDP_STATUS_SUCCESS)
    {
        desc->Usage                  = caps.Usage;
        desc->UsagePage              = caps.UsagePage;
        result = true;
    }
    HidD_FreePreparsedData(preparsedData);
    return result;
}

void HIDDeviceManager::initStrings(HANDLE hidDev, HIDDeviceDesc* desc) const
{
    // Documentation mentions 126 as being the max for USB.
    wchar_t strBuffer[196];

    // HidD_Get*String functions return nothing in buffer on failure,
    // so it's ok to do this without further error checking.
    strBuffer[0] = 0;
    HidD_GetManufacturerString(hidDev, strBuffer, sizeof(strBuffer));
    desc->Manufacturer = strBuffer;

    strBuffer[0] = 0;
    HidD_GetProductString(hidDev, strBuffer, sizeof(strBuffer));
    desc->Product = strBuffer;

    strBuffer[0] = 0;
    HidD_GetSerialNumberString(hidDev, strBuffer, sizeof(strBuffer));
    desc->SerialNumber = strBuffer;
}

//-------------------------------------------------------------------------------------
// **** Win32::HIDDevice

HIDDevice::HIDDevice(HIDDeviceManager* manager)
 : HIDManager(manager), inMinimalMode(false), Device(0), ReadRequested(false)
{
    memset(&ReadOverlapped, 0, sizeof(OVERLAPPED));
}

// This is a minimal constructor used during enumeration for us to pass
// a HIDDevice to the visit function (so that it can query feature reports). 
HIDDevice::HIDDevice(HIDDeviceManager* manager, HANDLE device)
 : HIDManager(manager), inMinimalMode(true), Device(device), ReadRequested(true)
{
    memset(&ReadOverlapped, 0, sizeof(OVERLAPPED));
}

HIDDevice::~HIDDevice()
{
    if (!inMinimalMode)
    {
        HIDShutdown();
    }
}

bool HIDDevice::HIDInitialize(const String& path)
{

    DevDesc.Path = path;

    if (!openDevice())
    {
        LogText("OVR::Win32::HIDDevice - Failed to open HIDDevice: ", path);
        return false;
    }


    HIDManager->Manager->pThread->AddTicksNotifier(this);
    HIDManager->Manager->pThread->AddMessageNotifier(this);

    LogText("OVR::Win32::HIDDevice - Opened '%s'\n"
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
    HIDP_PREPARSED_DATA* preparsedData = 0;
    if (!HIDManager->HidD_GetPreparsedData(Device, &preparsedData))
    {
        return false;
    }

    HIDP_CAPS caps;
    if (HIDManager->HidP_GetCaps(preparsedData, &caps) != HIDP_STATUS_SUCCESS)
    {
        HIDManager->HidD_FreePreparsedData(preparsedData);
        return false;
    }

    InputReportBufferLength  = caps.InputReportByteLength;
    OutputReportBufferLength = caps.OutputReportByteLength;
    FeatureReportBufferLength= caps.FeatureReportByteLength;
    HIDManager->HidD_FreePreparsedData(preparsedData);

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

bool HIDDevice::openDevice()
{
    memset(&ReadOverlapped, 0, sizeof(OVERLAPPED));

    Device = HIDManager->CreateHIDFile(DevDesc.Path.ToCStr());
    if (Device == INVALID_HANDLE_VALUE)
    {
        OVR_DEBUG_LOG(("Failed 'CreateHIDFile' while opening device, error = 0x%X.", 
			::GetLastError()));
        Device = 0;
        return false;
    }

    if (!HIDManager->HidD_SetNumInputBuffers(Device, 128))
    {
        OVR_ASSERT_LOG(false, ("Failed 'HidD_SetNumInputBuffers' while initializing device."));
        ::CloseHandle(Device);
        Device = 0;
        return false;
    }


    // Create a manual-reset non-signaled event.
    ReadOverlapped.hEvent = ::CreateEvent(0, TRUE, FALSE, 0);

    if (!ReadOverlapped.hEvent)
    {
        OVR_ASSERT_LOG(false, ("Failed to create event."));
        ::CloseHandle(Device);
        Device = 0;
        return false;
    }

    if (!initInfo())
    {
        OVR_ASSERT_LOG(false, ("Failed to get HIDDevice info."));

        ::CloseHandle(ReadOverlapped.hEvent);
        memset(&ReadOverlapped, 0, sizeof(OVERLAPPED));

        ::CloseHandle(Device);
        Device = 0;
        return false;
    }

    if (!initializeRead())
    {
        OVR_ASSERT_LOG(false, ("Failed to get intialize read for HIDDevice."));

        ::CloseHandle(ReadOverlapped.hEvent);
        memset(&ReadOverlapped, 0, sizeof(OVERLAPPED));

        ::CloseHandle(Device);
        Device = 0;
        return false;
    }

    return true;
}

void HIDDevice::HIDShutdown()
{   

    HIDManager->Manager->pThread->RemoveTicksNotifier(this);
    HIDManager->Manager->pThread->RemoveMessageNotifier(this);

    closeDevice();
    LogText("OVR::Win32::HIDDevice - Closed '%s'\n", DevDesc.Path.ToCStr());
}

bool HIDDevice::initializeRead()
{

    if (!ReadRequested)
    {        
        HIDManager->Manager->pThread->AddOverlappedEvent(this, ReadOverlapped.hEvent);
        ReadRequested = true;
    }

    // Read resets the event...
    while(::ReadFile(Device, ReadBuffer, InputReportBufferLength, 0, &ReadOverlapped))
    {
        processReadResult();
    }

    if (GetLastError() != ERROR_IO_PENDING)
    {
        // Some other error (such as unplugged).
        closeDeviceOnIOError();
        return false;
    }

    return true;
}

bool HIDDevice::processReadResult()
{

    OVR_ASSERT(ReadRequested);

    DWORD bytesRead = 0;

    if (GetOverlappedResult(Device, &ReadOverlapped, &bytesRead, FALSE))
    {
        // We've got data.
        if (Handler)
        {
            Handler->OnInputReport(ReadBuffer, bytesRead);
        }

        // TBD: Not needed?
        // Event should be reset by Read call...
        ReadOverlapped.Pointer = 0;
        ReadOverlapped.Internal = 0;
        ReadOverlapped.InternalHigh = 0;
        return true;
    }
    else
    {
        if (GetLastError() != ERROR_IO_PENDING)
        {
            closeDeviceOnIOError();
            return false;
        }
    }

    return false;
}

void HIDDevice::closeDevice()
{
    if (ReadRequested)
    {
        HIDManager->Manager->pThread->RemoveOverlappedEvent(this, ReadOverlapped.hEvent);
        ReadRequested = false;
        // Must call this to avoid Win32 assertion; CloseHandle is not enough.
        ::CancelIo(Device);
    }

    ::CloseHandle(ReadOverlapped.hEvent);
    memset(&ReadOverlapped, 0, sizeof(OVERLAPPED));

    ::CloseHandle(Device);
    Device = 0;
}

void HIDDevice::closeDeviceOnIOError()
{
    LogText("OVR::Win32::HIDDevice - Lost connection to '%s'\n", DevDesc.Path.ToCStr());
    closeDevice();
}

bool HIDDevice::SetFeatureReport(UByte* data, UInt32 length)
{
    if (!ReadRequested)
        return false;

    return HIDManager->HidD_SetFeature(Device, data, (ULONG) length) != FALSE;
}

bool HIDDevice::GetFeatureReport(UByte* data, UInt32 length)
{
    if (!ReadRequested)
        return false;

	return HIDManager->HidD_GetFeature(Device, data, (ULONG) length) != FALSE;
}

void HIDDevice::OnOverlappedEvent(HANDLE hevent)
{
    OVR_UNUSED(hevent);
    OVR_ASSERT(hevent == ReadOverlapped.hEvent);

    if (processReadResult()) 
    {
        // Proceed to read again.
        initializeRead();
    }
}

UInt64 HIDDevice::OnTicks(UInt64 ticksMks)
{
    if (Handler)
    {
        return Handler->OnTicks(ticksMks);
    }

    return DeviceManagerThread::Notifier::OnTicks(ticksMks);
}

bool HIDDevice::OnDeviceMessage(DeviceMessageType messageType, 
								const String& devicePath,
								bool* error)
{

    // Is this the correct device?
    if (DevDesc.Path.CompareNoCase(devicePath) != 0)
    {
        return false;
    }

    if (messageType == DeviceMessage_DeviceAdded && !Device)
    {
        // A closed device has been re-added. Try to reopen.
        if (!openDevice())
        {
            LogError("OVR::Win32::HIDDevice - Failed to reopen a device '%s' that was re-added.\n", devicePath.ToCStr());
			*error = true;
            return true;
        }

        LogText("OVR::Win32::HIDDevice - Reopened device '%s'\n", devicePath.ToCStr());
    }

    HIDHandler::HIDDeviceMessageType handlerMessageType = HIDHandler::HIDDeviceMessage_DeviceAdded;
    if (messageType == DeviceMessage_DeviceAdded)
    {
    }
    else if (messageType == DeviceMessage_DeviceRemoved)
    {
        handlerMessageType = HIDHandler::HIDDeviceMessage_DeviceRemoved;
    }
    else
    {
        OVR_ASSERT(0);		
    }

    if (Handler)
    {
        Handler->OnDeviceMessage(handlerMessageType);
    }

	*error = false;
    return true;
}

HIDDeviceManager* HIDDeviceManager::CreateInternal(Win32::DeviceManager* devManager)
{

    if (!System::IsInitialized())
    {
        // Use custom message, since Log is not yet installed.
        OVR_DEBUG_STATEMENT(Log::GetDefaultLog()->
            LogMessage(Log_Debug, "HIDDeviceManager::Create failed - OVR::System not initialized"); );
        return 0;
    }

    Ptr<Win32::HIDDeviceManager> manager = *new Win32::HIDDeviceManager(devManager);

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

} // namespace Win32

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

    Ptr<Win32::HIDDeviceManager> manager = *new Win32::HIDDeviceManager(NULL);

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
