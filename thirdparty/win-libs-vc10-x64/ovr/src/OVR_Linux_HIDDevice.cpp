/************************************************************************************
Filename    :   OVR_Linux_HIDDevice.cpp
Content     :   Linux HID device implementation.
Created     :   February 26, 2013
Authors     :   Lee Cooper
 
Copyright   :   Copyright 2013 Oculus VR, Inc. All Rights reserved.
 
Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#include "OVR_Linux_HIDDevice.h"

#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/hidraw.h>
#include "OVR_HIDDeviceImpl.h"

namespace OVR { namespace Linux {

static const UInt32 MAX_QUEUED_INPUT_REPORTS = 5;
    
//-------------------------------------------------------------------------------------
// **** Linux::DeviceManager
//-----------------------------------------------------------------------------
HIDDeviceManager::HIDDeviceManager(DeviceManager* manager) : DevManager(manager)
{
    UdevInstance = NULL;
    HIDMonitor = NULL;
    HIDMonHandle = -1;
}

//-----------------------------------------------------------------------------
HIDDeviceManager::~HIDDeviceManager()
{
}

//-----------------------------------------------------------------------------
bool HIDDeviceManager::initializeManager()
{
    if (HIDMonitor)
    {
        return true;
    }

    // Create a udev_monitor handle to watch for device changes (hot-plug detection)
    HIDMonitor = udev_monitor_new_from_netlink(UdevInstance, "udev");
    if (HIDMonitor == NULL)
    {
        return false;
    }

    udev_monitor_filter_add_match_subsystem_devtype(HIDMonitor, "hidraw", NULL);  // filter for hidraw only
	
    int err = udev_monitor_enable_receiving(HIDMonitor);
    if (err)
    {
        udev_monitor_unref(HIDMonitor);
        HIDMonitor = NULL;
        return false;
    }
	
    // Get the file descriptor (fd) for the monitor.  
    HIDMonHandle = udev_monitor_get_fd(HIDMonitor);
    if (HIDMonHandle < 0)
    {
        udev_monitor_unref(HIDMonitor);
        HIDMonitor = NULL;
        return false;
    }

    // This file handle will be polled along-side with the device hid handles for changes
    // Add the handle to the polling list
    if (!DevManager->pThread->AddSelectFd(this, HIDMonHandle))
    {
        close(HIDMonHandle);
        HIDMonHandle = -1;

        udev_monitor_unref(HIDMonitor);
        HIDMonitor = NULL;
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
bool HIDDeviceManager::Initialize()
{
    // Get a udev library handle.  This handle must stay active during the
    // duration the lifetime of device monitoring handles
    UdevInstance = udev_new();
    if (!UdevInstance)
        return false;

    return initializeManager();
}

//-----------------------------------------------------------------------------
void HIDDeviceManager::Shutdown()
{
    OVR_ASSERT_LOG((UdevInstance), ("Should have called 'Initialize' before 'Shutdown'."));

    if (HIDMonitor)
    {
        DevManager->pThread->RemoveSelectFd(this, HIDMonHandle);
        close(HIDMonHandle);
        HIDMonHandle = -1;

        udev_monitor_unref(HIDMonitor);
        HIDMonitor = NULL;
    }

    udev_unref(UdevInstance);  // release the library
    
    LogText("OVR::Linux::HIDDeviceManager - shutting down.\n");
}

//-------------------------------------------------------------------------------
bool HIDDeviceManager::AddNotificationDevice(HIDDevice* device)
{
    NotificationDevices.PushBack(device);
    return true;
}

//-------------------------------------------------------------------------------
bool HIDDeviceManager::RemoveNotificationDevice(HIDDevice* device)
{
    for (UPInt i = 0; i < NotificationDevices.GetSize(); i++)
    {
        if (NotificationDevices[i] == device)
        {
            NotificationDevices.RemoveAt(i);
            return true;
        }
    }
    return false;
}

//-----------------------------------------------------------------------------
bool HIDDeviceManager::getIntProperty(udev_device* device,
                                      const char* propertyName,
                                      SInt32* pResult)
{
    const char* str = udev_device_get_sysattr_value(device, propertyName);
	if (str)
    {
        *pResult = strtol(str, NULL, 16);
        return true;
    }
    else
    {
        *pResult = 0;
        return true;
    }
}

//-----------------------------------------------------------------------------
bool HIDDeviceManager::initVendorProductVersion(udev_device* device, HIDDeviceDesc* pDevDesc)
{
    SInt32 result;
    if (getIntProperty(device, "idVendor", &result))
        pDevDesc->VendorId = result;
    else
        return false;

    if (getIntProperty(device, "idProduct", &result))
        pDevDesc->ProductId = result;
    else
        return false;

    return true;
}

//-----------------------------------------------------------------------------
bool HIDDeviceManager::getStringProperty(udev_device* device,
                                         const char* propertyName,
                                         OVR::String* pResult)
{
    // Get the attribute in UTF8
    const char* str = udev_device_get_sysattr_value(device, propertyName);
	if (str)
    {   // Copy the string into the return value
		*pResult = String(str);
        return true;
	}
    else
    {
        return false;
    }
}

//-----------------------------------------------------------------------------
bool HIDDeviceManager::Enumerate(HIDEnumerateVisitor* enumVisitor)
{
    
    if (!initializeManager())
    {
        return false;
    }

	// Get a list of hid devices
    udev_enumerate* devices = udev_enumerate_new(UdevInstance);
    udev_enumerate_add_match_subsystem(devices, "hidraw");
    udev_enumerate_scan_devices(devices);

    udev_list_entry* entry = udev_enumerate_get_list_entry(devices);

    // Search each device for the matching vid/pid
    while (entry != NULL)
    {
        // Get the device file name
        const char* sysfs_path = udev_list_entry_get_name(entry);
        udev_device* hid;  // The device's HID udev node.
        hid = udev_device_new_from_syspath(UdevInstance, sysfs_path);
        const char* dev_path = udev_device_get_devnode(hid);

        // Get the USB device
        hid = udev_device_get_parent_with_subsystem_devtype(hid, "usb", "usb_device");
        if (hid)
        {
            HIDDeviceDesc devDesc;

            // Check the VID/PID for a match
            if (dev_path &&
                initVendorProductVersion(hid, &devDesc) &&
                enumVisitor->MatchVendorProduct(devDesc.VendorId, devDesc.ProductId))
            {
                devDesc.Path = dev_path;
                getFullDesc(hid, &devDesc);

                // Look for the device to check if it is already opened.
                Ptr<DeviceCreateDesc> existingDevice = DevManager->FindHIDDevice(devDesc);
                // if device exists and it is opened then most likely the device open()
                // will fail; therefore, we just set Enumerated to 'true' and continue.
                if (existingDevice && existingDevice->pDevice)
                {
                    existingDevice->Enumerated = true;
                }
                else
                {   // open the device temporarily for startup communication
                    int device_handle = open(dev_path, O_RDWR);
                    if (device_handle >= 0)
                    {
                        // Construct minimal device that the visitor callback can get feature reports from
                        Linux::HIDDevice device(this, device_handle);
                        enumVisitor->Visit(device, devDesc);

                        close(device_handle);  // close the file handle
                    }
                }
            }

            udev_device_unref(hid);
            entry = udev_list_entry_get_next(entry);
        }
    }

	// Free the enumerator and udev objects
    udev_enumerate_unref(devices);

    return true;
}

//-----------------------------------------------------------------------------
OVR::HIDDevice* HIDDeviceManager::Open(const String& path)
{
    Ptr<Linux::HIDDevice> device = *new Linux::HIDDevice(this);

    if (device->HIDInitialize(path))
    {
        device->AddRef();        
        return device;
    }

    return NULL;
}

//-----------------------------------------------------------------------------
bool HIDDeviceManager::getFullDesc(udev_device* device, HIDDeviceDesc* desc)
{
        
    if (!initVendorProductVersion(device, desc))
    {
        return false;
    }
        
    if (!getStringProperty(device, "serial", &(desc->SerialNumber)))
    {
        return false;
    }
    
    getStringProperty(device, "manufacturer", &(desc->Manufacturer));
    getStringProperty(device, "product", &(desc->Product));
        
    return true;
}

//-----------------------------------------------------------------------------
bool HIDDeviceManager::GetDescriptorFromPath(const char* dev_path, HIDDeviceDesc* desc)
{
    if (!initializeManager())
    {
        return false;
    }

    // Search for the udev device from the given pathname so we can
    // have a handle to query device properties

    udev_enumerate* devices = udev_enumerate_new(UdevInstance);
    udev_enumerate_add_match_subsystem(devices, "hidraw");
    udev_enumerate_scan_devices(devices);

    udev_list_entry* entry = udev_enumerate_get_list_entry(devices);

    bool success = false;
    // Search for the device with the matching path
    while (entry != NULL)
    {
        // Get the device file name
        const char* sysfs_path = udev_list_entry_get_name(entry);
        udev_device* hid;  // The device's HID udev node.
        hid = udev_device_new_from_syspath(UdevInstance, sysfs_path);
        const char* path = udev_device_get_devnode(hid);

        if (OVR_strcmp(dev_path, path) == 0)
        {   // Found the device so lets collect the device descriptor

            // Get the USB device
            hid = udev_device_get_parent_with_subsystem_devtype(hid, "usb", "usb_device");
            if (hid)
            {
                desc->Path = dev_path;
                success = getFullDesc(hid, desc);
            }

        }

        udev_device_unref(hid);
        entry = udev_list_entry_get_next(entry);
    }

    // Free the enumerator
    udev_enumerate_unref(devices);

    return success;
}

//-----------------------------------------------------------------------------
void HIDDeviceManager::OnEvent(int i, int fd)
{
    // There is a device status change
    udev_device* hid = udev_monitor_receive_device(HIDMonitor);
    if (hid)
    {
        const char* dev_path = udev_device_get_devnode(hid);
        const char* action = udev_device_get_action(hid);

        HIDDeviceDesc device_info;
        device_info.Path = dev_path;

        MessageType notify_type;
        if (OVR_strcmp(action, "add") == 0)
        {
            notify_type = Message_DeviceAdded;

            // Retrieve the device info.  This can only be done on a connected
            // device and is invalid for a disconnected device

            // Get the USB device
            hid = udev_device_get_parent_with_subsystem_devtype(hid, "usb", "usb_device");
            if (!hid)
            {
                return;
            }

            getFullDesc(hid, &device_info);
        }
        else if (OVR_strcmp(action, "remove") == 0)
        {
            notify_type = Message_DeviceRemoved;
        }
        else
        {
            return;
        }

        bool error = false;
        bool deviceFound = false;
        for (UPInt i = 0; i < NotificationDevices.GetSize(); i++)
        {
            if (NotificationDevices[i] &&
                NotificationDevices[i]->OnDeviceNotification(notify_type, &device_info, &error))
            {
                // The notification was for an existing device
                deviceFound = true;
                break;
            }
        }

        if (notify_type == Message_DeviceAdded && !deviceFound)
        {
            DevManager->DetectHIDDevice(device_info);
        }

        udev_device_unref(hid);
    }
}

//=============================================================================
//                           Linux::HIDDevice
//=============================================================================
HIDDevice::HIDDevice(HIDDeviceManager* manager)
 :  HIDManager(manager), InMinimalMode(false)
{
    DeviceHandle = -1;
}
    
//-----------------------------------------------------------------------------
// This is a minimal constructor used during enumeration for us to pass
// a HIDDevice to the visit function (so that it can query feature reports).
HIDDevice::HIDDevice(HIDDeviceManager* manager, int device_handle)
:   HIDManager(manager), DeviceHandle(device_handle), InMinimalMode(true)
{
}

//-----------------------------------------------------------------------------
HIDDevice::~HIDDevice()
{
    if (!InMinimalMode)
    {
        HIDShutdown();
    }
}

//-----------------------------------------------------------------------------
bool HIDDevice::HIDInitialize(const String& path)
{
    const char* hid_path = path.ToCStr();
    if (!openDevice(hid_path))
    {
        LogText("OVR::Linux::HIDDevice - Failed to open HIDDevice: %s", hid_path);
        return false;
    }
    
    HIDManager->DevManager->pThread->AddTicksNotifier(this);
    HIDManager->AddNotificationDevice(this);

    LogText("OVR::Linux::HIDDevice - Opened '%s'\n"
            "                    Manufacturer:'%s'  Product:'%s'  Serial#:'%s'\n",
            DevDesc.Path.ToCStr(),
            DevDesc.Manufacturer.ToCStr(), DevDesc.Product.ToCStr(),
            DevDesc.SerialNumber.ToCStr());
    
    return true;
}

//-----------------------------------------------------------------------------
bool HIDDevice::initInfo()
{
    // Device must have been successfully opened.
    OVR_ASSERT(DeviceHandle >= 0);

    int desc_size = 0;
    hidraw_report_descriptor rpt_desc;
    memset(&rpt_desc, 0, sizeof(rpt_desc));

    // get report descriptor size
    int r = ioctl(DeviceHandle, HIDIOCGRDESCSIZE, &desc_size);
    if (r < 0)
    {
        OVR_ASSERT_LOG(false, ("Failed to get report descriptor size."));
        return false;
    }

    // Get the report descriptor
    rpt_desc.size = desc_size;
    r = ioctl(DeviceHandle, HIDIOCGRDESC, &rpt_desc);
    if (r < 0)
    {
        OVR_ASSERT_LOG(false, ("Failed to get report descriptor."));
        return false;
    }
    
    /*
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
    */

    // Get report lengths.
// TODO: hard-coded for now.  Need to interpret these values from the report descriptor
    InputReportBufferLength = 62;
    OutputReportBufferLength = 0;
    FeatureReportBufferLength = 69;
    
    if (ReadBufferSize < InputReportBufferLength)
    {
        OVR_ASSERT_LOG(false, ("Input report buffer length is bigger than read buffer."));
        return false;
    }
      
    return true;
}

//-----------------------------------------------------------------------------
bool HIDDevice::openDevice(const char* device_path)
{
    // First fill out the device descriptor
    if (!HIDManager->GetDescriptorFromPath(device_path, &DevDesc))
    {
        return false;
    }

    // Now open the device
    DeviceHandle = open(device_path, O_RDWR);
    if (DeviceHandle < 0)
    {
        OVR_DEBUG_LOG(("Failed 'CreateHIDFile' while opening device, error = 0x%X.", errno));
        DeviceHandle = -1;
        return false;
    }

    // fill out some values from the feature report descriptor
    if (!initInfo())
    {
        OVR_ASSERT_LOG(false, ("Failed to get HIDDevice info."));

        close(DeviceHandle);
        DeviceHandle = -1;
        return false;
    }

    // Add the device to the polling list
    if (!HIDManager->DevManager->pThread->AddSelectFd(this, DeviceHandle))
    {
        OVR_ASSERT_LOG(false, ("Failed to initialize polling for HIDDevice."));

        close(DeviceHandle);
        DeviceHandle = -1;
        return false;
    }
    
    return true;
}
    
//-----------------------------------------------------------------------------
void HIDDevice::HIDShutdown()
{

    HIDManager->DevManager->pThread->RemoveTicksNotifier(this);
    HIDManager->RemoveNotificationDevice(this);
    
    if (DeviceHandle >= 0) // Device may already have been closed if unplugged.
    {
        closeDevice(false);
    }
    
    LogText("OVR::Linux::HIDDevice - HIDShutdown '%s'\n", DevDesc.Path.ToCStr());
}

//-----------------------------------------------------------------------------
void HIDDevice::closeDevice(bool wasUnplugged)
{
    OVR_ASSERT(DeviceHandle >= 0);
    

    HIDManager->DevManager->pThread->RemoveSelectFd(this, DeviceHandle);

    close(DeviceHandle);  // close the file handle
    DeviceHandle = -1;
        
    LogText("OVR::Linux::HIDDevice - HID Device Closed '%s'\n", DevDesc.Path.ToCStr());
}

//-----------------------------------------------------------------------------
void HIDDevice::closeDeviceOnIOError()
{
    LogText("OVR::Linux::HIDDevice - Lost connection to '%s'\n", DevDesc.Path.ToCStr());
    closeDevice(false);
}

//-----------------------------------------------------------------------------
bool HIDDevice::SetFeatureReport(UByte* data, UInt32 length)
{
    
    if (DeviceHandle < 0)
        return false;
    
    UByte reportID = data[0];

    if (reportID == 0)
    {
        // Not using reports so remove from data packet.
        data++;
        length--;
    }

    int r = ioctl(DeviceHandle, HIDIOCSFEATURE(length), data);
	return (r >= 0);
}

//-----------------------------------------------------------------------------
bool HIDDevice::GetFeatureReport(UByte* data, UInt32 length)
{
    if (DeviceHandle < 0)
        return false;

	int r = ioctl(DeviceHandle, HIDIOCGFEATURE(length), data);
    return (r >= 0);
}

//-----------------------------------------------------------------------------
UInt64 HIDDevice::OnTicks(UInt64 ticksMks)
{
    if (Handler)
    {
        return Handler->OnTicks(ticksMks);
    }
    
    return DeviceManagerThread::Notifier::OnTicks(ticksMks);
}

//-----------------------------------------------------------------------------
void HIDDevice::OnEvent(int i, int fd)
{
    // We have data to read from the device
    int bytes = read(fd, ReadBuffer, ReadBufferSize);
    if (bytes >= 0)
    {
// TODO: I need to handle partial messages and package reconstruction
        if (Handler)
        {
            Handler->OnInputReport(ReadBuffer, bytes);
        }
    }
    else
    {   // Close the device on read error.
        closeDeviceOnIOError();
    }
}

//-----------------------------------------------------------------------------
bool HIDDevice::OnDeviceNotification(MessageType messageType,
                                     HIDDeviceDesc* device_info,
                                     bool* error)
{
    const char* device_path = device_info->Path.ToCStr();

    if (messageType == Message_DeviceAdded && DeviceHandle < 0)
    {
        // Is this the correct device?
        if (!(device_info->VendorId == DevDesc.VendorId
            && device_info->ProductId == DevDesc.ProductId
            && device_info->SerialNumber == DevDesc.SerialNumber))
        {
            return false;
        }

        // A closed device has been re-added. Try to reopen.
        if (!openDevice(device_path))
        {
            LogError("OVR::Linux::HIDDevice - Failed to reopen a device '%s' that was re-added.\n", 
                     device_path);
            *error = true;
            return true;
        }

        LogText("OVR::Linux::HIDDevice - Reopened device '%s'\n", device_path);

        if (Handler)
        {
            Handler->OnDeviceMessage(HIDHandler::HIDDeviceMessage_DeviceAdded);
        }
    }
    else if (messageType == Message_DeviceRemoved)
    {
        // Is this the correct device?
        // For disconnected device, the device description will be invalid so
        // checking the path is the only way to match them
        if (DevDesc.Path.CompareNoCase(device_path) != 0)
        {
            return false;
        }

        if (DeviceHandle >= 0)
        {
            closeDevice(true);
        }

        if (Handler)
        {
            Handler->OnDeviceMessage(HIDHandler::HIDDeviceMessage_DeviceRemoved);
        }
    }
    else
    {
        OVR_ASSERT(0);
    }

    *error = false;
    return true;
}

//-----------------------------------------------------------------------------
HIDDeviceManager* HIDDeviceManager::CreateInternal(Linux::DeviceManager* devManager)
{
        
    if (!System::IsInitialized())
    {
        // Use custom message, since Log is not yet installed.
        OVR_DEBUG_STATEMENT(Log::GetDefaultLog()->
                            LogMessage(Log_Debug, "HIDDeviceManager::Create failed - OVR::System not initialized"); );
        return 0;
    }

    Ptr<Linux::HIDDeviceManager> manager = *new Linux::HIDDeviceManager(devManager);

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
    
} // namespace Linux

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

    Ptr<Linux::HIDDeviceManager> manager = *new Linux::HIDDeviceManager(NULL);

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
