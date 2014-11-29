/************************************************************************************

Filename    :   OVR_Win32_DeviceManager.cpp
Content     :   Win32 implementation of DeviceManager.
Created     :   September 21, 2012
Authors     :   Michael Antonov

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#include "OVR_Win32_DeviceManager.h"

// Sensor & HMD Factories
#include "OVR_SensorImpl.h"
#include "OVR_LatencyTestImpl.h"
#include "OVR_Win32_HMDDevice.h"
#include "OVR_Win32_DeviceStatus.h"
#include "OVR_Win32_HIDDevice.h"

#include "Kernel/OVR_Timer.h"
#include "Kernel/OVR_Std.h"
#include "Kernel/OVR_Log.h"

DWORD Debug_WaitedObjectCount = 0;

namespace OVR { namespace Win32 {


//-------------------------------------------------------------------------------------
// **** Win32::DeviceManager

DeviceManager::DeviceManager()
{
    HidDeviceManager = *HIDDeviceManager::CreateInternal(this);
}

DeviceManager::~DeviceManager()
{
    // make sure Shutdown was called.
    OVR_ASSERT(!pThread);
}

bool DeviceManager::Initialize(DeviceBase*)
{
    if (!DeviceManagerImpl::Initialize(0))
        return false;

    pThread = *new DeviceManagerThread(this);
    if (!pThread || !pThread->Start())
        return false;
         
    pCreateDesc->pDevice = this;
    LogText("OVR::DeviceManager - initialized.\n");
    return true;
}

void DeviceManager::Shutdown()
{   
    LogText("OVR::DeviceManager - shutting down.\n");

    // Set Manager shutdown marker variable; this prevents
    // any existing DeviceHandle objects from accessing device.
    pCreateDesc->pLock->pManager = 0;

    // Push for thread shutdown *WITH NO WAIT*.
    // This will have the following effect:
    //  - Exit command will get enqueued, which will be executed later on the thread itself.
    //  - Beyond this point, this DeviceManager object may be deleted by our caller.
    //  - Other commands, such as CreateDevice, may execute before ExitCommand, but they will
    //    fail gracefully due to pLock->pManager == 0. Future commands can't be enqued
    //    after pManager is null.
    //  - Once ExitCommand executes, ThreadCommand::Run loop will exit and release the last
    //    reference to the thread object.
    pThread->PushExitCommand(false);
    pThread->DetachDeviceManager();
    pThread.Clear();

    DeviceManagerImpl::Shutdown();
}

ThreadCommandQueue* DeviceManager::GetThreadQueue()
{
    return pThread;
}

bool DeviceManager::GetDeviceInfo(DeviceInfo* info) const
{
    if ((info->InfoClassType != Device_Manager) &&
        (info->InfoClassType != Device_None))
        return false;
    
    info->Type    = Device_Manager;
    info->Version = 0;
    OVR_strcpy(info->ProductName, DeviceInfo::MaxNameLength, "DeviceManager");
    OVR_strcpy(info->Manufacturer,DeviceInfo::MaxNameLength, "Oculus VR, Inc.");        
    return true;
}

DeviceEnumerator<> DeviceManager::EnumerateDevicesEx(const DeviceEnumerationArgs& args)
{
    // TBD: Can this be avoided in the future, once proper device notification is in place?
    if (GetThreadId() != OVR::GetCurrentThreadId())
    {
        pThread->PushCall((DeviceManagerImpl*)this,
            &DeviceManager::EnumerateAllFactoryDevices, true);
    }
    else
        DeviceManager::EnumerateAllFactoryDevices();

    return DeviceManagerImpl::EnumerateDevicesEx(args);
}

ThreadId DeviceManager::GetThreadId() const
{
    return pThread->GetThreadId();
}
    
bool DeviceManager::GetHIDDeviceDesc(const String& path, HIDDeviceDesc* pdevDesc) const
{
    if (GetHIDDeviceManager())
        return static_cast<HIDDeviceManager*>(GetHIDDeviceManager())->GetHIDDeviceDesc(path, pdevDesc);
    return false;
}


//-------------------------------------------------------------------------------------
// ***** DeviceManager Thread 

DeviceManagerThread::DeviceManagerThread(DeviceManager* pdevMgr)
    : Thread(ThreadStackSize), hCommandEvent(0), pDeviceMgr(pdevMgr)
{    
    // Create a non-signaled manual-reset event.
    hCommandEvent = ::CreateEvent(0, TRUE, FALSE, 0);
    if (!hCommandEvent)
        return;

    // Must add event before starting.
    AddOverlappedEvent(0, hCommandEvent);

	// Create device messages object.
	pStatusObject = *new DeviceStatus(this);
}

DeviceManagerThread::~DeviceManagerThread()
{
    // Remove overlapped event [0], after thread service exit.
    if (hCommandEvent)
    {
        RemoveOverlappedEvent(0, hCommandEvent);    
        ::CloseHandle(hCommandEvent);
        hCommandEvent = 0;
    }
}

int DeviceManagerThread::Run()
{
    ThreadCommand::PopBuffer command;

    SetThreadName("OVR::DeviceManagerThread");
    LogText("OVR::DeviceManagerThread - running (ThreadId=0x%X).\n", GetThreadId());
  
	if (!pStatusObject->Initialize())
	{
		LogText("OVR::DeviceManagerThread - failed to initialize MessageObject.\n");
	}

    while(!IsExiting())
    {
        // PopCommand will reset event on empty queue.
        if (PopCommand(&command))
        {
            command.Execute();
        }
        else
        {
            DWORD eventIndex = 0;
            do {
                UPInt numberOfWaitHandles = WaitHandles.GetSize();
				Debug_WaitedObjectCount = (DWORD)numberOfWaitHandles;

                DWORD waitMs = INFINITE;

                // If devices have time-dependent logic registered, get the longest wait
                // allowed based on current ticks.
                if (!TicksNotifiers.IsEmpty())
                {
                    UInt64 ticksMks = Timer::GetTicks();
                    DWORD  waitAllowed;
                    
                    for (UPInt j = 0; j < TicksNotifiers.GetSize(); j++)
                    {
                        waitAllowed = (DWORD)(TicksNotifiers[j]->OnTicks(ticksMks) / Timer::MksPerMs);
                        if (waitAllowed < waitMs)
                            waitMs = waitAllowed;
                    }
                }
          
				// Wait for event signals or window messages.
                eventIndex = MsgWaitForMultipleObjects((DWORD)numberOfWaitHandles, &WaitHandles[0], FALSE, waitMs, QS_ALLINPUT);
				
                if (eventIndex != WAIT_FAILED)
                {
                    if (eventIndex == WAIT_TIMEOUT)
                        continue;

                    // TBD: Does this ever apply?
                    OVR_ASSERT(eventIndex < WAIT_ABANDONED_0);

                    if (eventIndex == WAIT_OBJECT_0)
                    {
                        // Handle [0] services commands.
                        break;
                    }
					else if (eventIndex == WAIT_OBJECT_0 + numberOfWaitHandles)
					{
						// Handle Windows messages.
						pStatusObject->ProcessMessages();
					}
                    else 
                    {
                        // Notify waiting device that its event is signaled.
                        unsigned i = eventIndex - WAIT_OBJECT_0; 
                        OVR_ASSERT(i < numberOfWaitHandles);
                        if (WaitNotifiers[i])                        
                            WaitNotifiers[i]->OnOverlappedEvent(WaitHandles[i]);
                    }
                }

            } while(eventIndex != WAIT_FAILED);
                    
        }
    }

	pStatusObject->ShutDown();

    LogText("OVR::DeviceManagerThread - exiting (ThreadId=0x%X).\n", GetThreadId());
    return 0;
}

bool DeviceManagerThread::AddOverlappedEvent(Notifier* notify, HANDLE hevent)
{
    WaitNotifiers.PushBack(notify);
    WaitHandles.PushBack(hevent);

    OVR_ASSERT(WaitNotifiers.GetSize() <= MAXIMUM_WAIT_OBJECTS);
    return true;
}

bool DeviceManagerThread::RemoveOverlappedEvent(Notifier* notify, HANDLE hevent)
{
    // [0] is reserved for thread commands with notify of null, but we still
    // can use this function to remove it.
    for (UPInt i = 0; i < WaitNotifiers.GetSize(); i++)
    {
        if ((WaitNotifiers[i] == notify) && (WaitHandles[i] == hevent))
        {
            WaitNotifiers.RemoveAt(i);
            WaitHandles.RemoveAt(i);
            return true;
        }
    }
    return false;
}

bool DeviceManagerThread::AddTicksNotifier(Notifier* notify)
{
     TicksNotifiers.PushBack(notify);
     return true;
}

bool DeviceManagerThread::RemoveTicksNotifier(Notifier* notify)
{
    for (UPInt i = 0; i < TicksNotifiers.GetSize(); i++)
    {
        if (TicksNotifiers[i] == notify)
        {
            TicksNotifiers.RemoveAt(i);
            return true;
        }
    }
    return false;
}

bool DeviceManagerThread::AddMessageNotifier(Notifier* notify)
{
	MessageNotifiers.PushBack(notify);
	return true;
}

bool DeviceManagerThread::RemoveMessageNotifier(Notifier* notify)
{
	for (UPInt i = 0; i < MessageNotifiers.GetSize(); i++)
	{
		if (MessageNotifiers[i] == notify)
		{
			MessageNotifiers.RemoveAt(i);
			return true;
		}
	}
	return false;
}

bool DeviceManagerThread::OnMessage(MessageType type, const String& devicePath)
{
	Notifier::DeviceMessageType notifierMessageType = Notifier::DeviceMessage_DeviceAdded;
	if (type == DeviceAdded)
	{
	}
	else if (type == DeviceRemoved)
	{
		notifierMessageType = Notifier::DeviceMessage_DeviceRemoved;
	}
	else
	{
		OVR_ASSERT(false);
	}

	bool error = false;
    bool deviceFound = false;
	for (UPInt i = 0; i < MessageNotifiers.GetSize(); i++)
    {
		if (MessageNotifiers[i] && 
			MessageNotifiers[i]->OnDeviceMessage(notifierMessageType, devicePath, &error))
		{
			// The notifier belonged to a device with the specified device name so we're done.
            deviceFound = true;
			break;
		}
    }
    if (type == DeviceAdded && !deviceFound)
    {
        Lock::Locker devMgrLock(&DevMgrLock);
        // a new device was connected. Go through all device factories and
        // try to detect the device using HIDDeviceDesc.
        HIDDeviceDesc devDesc;
        if (pDeviceMgr->GetHIDDeviceDesc(devicePath, &devDesc))
        {
            Lock::Locker deviceLock(pDeviceMgr->GetLock());
            DeviceFactory* factory = pDeviceMgr->Factories.GetFirst();
            while(!pDeviceMgr->Factories.IsNull(factory))
            {
                if (factory->DetectHIDDevice(pDeviceMgr, devDesc))
                {
                    deviceFound = true;
                    break;
                }
                factory = factory->pNext;
            }
        }
    }

    if (!deviceFound && strstr(devicePath.ToCStr(), "#OVR00"))
    {
        Ptr<DeviceManager> pmgr;
        {
            Lock::Locker devMgrLock(&DevMgrLock);
            pmgr = pDeviceMgr;
        }
        // HMD plugged/unplugged
        // This is not a final solution to enumerate HMD devices and get
        // a first available handle. This won't work with multiple rifts.
        // @TODO (!AB)
        pmgr->EnumerateDevices<HMDDevice>();
    }

	return !error;
}

void DeviceManagerThread::DetachDeviceManager()
{
    Lock::Locker devMgrLock(&DevMgrLock);
    pDeviceMgr = NULL;
}

} // namespace Win32


//-------------------------------------------------------------------------------------
// ***** Creation


// Creates a new DeviceManager and initializes OVR.
DeviceManager* DeviceManager::Create()
{

    if (!System::IsInitialized())
    {
        // Use custom message, since Log is not yet installed.
        OVR_DEBUG_STATEMENT(Log::GetDefaultLog()->
            LogMessage(Log_Debug, "DeviceManager::Create failed - OVR::System not initialized"); );
        return 0;
    }

    Ptr<Win32::DeviceManager> manager = *new Win32::DeviceManager;

    if (manager)
    {
        if (manager->Initialize(0))
        {            
            manager->AddFactory(&SensorDeviceFactory::Instance);
            manager->AddFactory(&LatencyTestDeviceFactory::Instance);
            manager->AddFactory(&Win32::HMDDeviceFactory::Instance);

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

