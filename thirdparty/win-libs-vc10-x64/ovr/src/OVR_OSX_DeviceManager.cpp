/************************************************************************************

Filename    :   OVR_OSX_DeviceManager.cpp
Content     :   OSX specific DeviceManager implementation.
Created     :   March 14, 2013
Authors     :   Lee Cooper

Copyright   :   Copyright 2013 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#include "OVR_OSX_DeviceManager.h"

// Sensor & HMD Factories
#include "OVR_LatencyTestImpl.h"
#include "OVR_SensorImpl.h"
#include "OVR_OSX_HMDDevice.h"
#include "OVR_OSX_HIDDevice.h"

#include "Kernel/OVR_Timer.h"
#include "Kernel/OVR_Std.h"
#include "Kernel/OVR_Log.h"

#include <IOKit/hid/IOHIDManager.h>
#include <IOKit/hid/IOHIDKeys.h>


namespace OVR { namespace OSX {

//-------------------------------------------------------------------------------------
// **** OSX::DeviceManager

DeviceManager::DeviceManager()
{
}

DeviceManager::~DeviceManager()
{
    OVR_DEBUG_LOG(("OSX::DeviceManager::~DeviceManager was called"));
}

bool DeviceManager::Initialize(DeviceBase*)
{
    if (!DeviceManagerImpl::Initialize(0))
        return false;
    
    // Start the background thread.
    pThread = *new DeviceManagerThread();
    if (!pThread || !pThread->Start())
        return false;

    // Wait for the thread to be fully up and running.
    pThread->StartupEvent.Wait();

    // Do this now that we know the thread's run loop.
    HidDeviceManager = *HIDDeviceManager::CreateInternal(this);

    CGDisplayRegisterReconfigurationCallback(displayReconfigurationCallBack, this);
         
    pCreateDesc->pDevice = this;
    LogText("OVR::DeviceManager - initialized.\n");

    return true;
}

void DeviceManager::Shutdown()
{
    LogText("OVR::DeviceManager - shutting down.\n");

    CGDisplayRemoveReconfigurationCallback(displayReconfigurationCallBack, this);
    
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
    pThread->Shutdown();
    pThread.Clear();

    DeviceManagerImpl::Shutdown();
}

ThreadCommandQueue* DeviceManager::GetThreadQueue()
{
    return pThread;
}

ThreadId DeviceManager::GetThreadId() const
{
    return pThread->GetThreadId();
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
    pThread->PushCall((DeviceManagerImpl*)this,
                      &DeviceManager::EnumerateAllFactoryDevices, true);

    return DeviceManagerImpl::EnumerateDevicesEx(args);
}

void DeviceManager::displayReconfigurationCallBack (CGDirectDisplayID display,
                                                    CGDisplayChangeSummaryFlags flags,
                                                    void *userInfo)
{
    DeviceManager* manager = reinterpret_cast<DeviceManager*>(userInfo);
    OVR_UNUSED(manager);
    
    if (flags & kCGDisplayAddFlag)
    {
        LogText("Display Added, id = %d\n", int(display));
        manager->EnumerateDevices<HMDDevice>();
    }
    else if (flags & kCGDisplayRemoveFlag)
    {
        LogText("Display Removed, id = %d\n", int(display));
        manager->EnumerateDevices<HMDDevice>();
    }
}

//-------------------------------------------------------------------------------------
// ***** DeviceManager Thread 

DeviceManagerThread::DeviceManagerThread()
    : Thread(ThreadStackSize)
{    
}

DeviceManagerThread::~DeviceManagerThread()
{
}

int DeviceManagerThread::Run()
{

    SetThreadName("OVR::DeviceManagerThread");
    LogText("OVR::DeviceManagerThread - running (ThreadId=0x%p).\n", GetThreadId());

    // Store out the run loop ref.
    RunLoop = CFRunLoopGetCurrent();

    // Create a 'source' to enable us to signal the run loop to process the command queue.
    CFRunLoopSourceContext sourceContext;
    memset(&sourceContext, 0, sizeof(sourceContext));
    sourceContext.version = 0;
    sourceContext.info = this;
    sourceContext.perform = &staticCommandQueueSourceCallback;

    CommandQueueSource = CFRunLoopSourceCreate(kCFAllocatorDefault, 0 , &sourceContext);

    CFRunLoopAddSource(RunLoop, CommandQueueSource, kCFRunLoopDefaultMode);


    // Signal to the parent thread that initialization has finished.
    StartupEvent.SetEvent();


    ThreadCommand::PopBuffer command;
   
    while(!IsExiting())
    {
        // PopCommand will reset event on empty queue.
        if (PopCommand(&command))
        {
            command.Execute();
        }
        else
        {
            SInt32 exitReason = 0;
            do {

                UInt32 waitMs = INT_MAX;

                // If devices have time-dependent logic registered, get the longest wait
                // allowed based on current ticks.
                if (!TicksNotifiers.IsEmpty())
                {
                    UInt64 ticksMks = Timer::GetTicks();
                    UInt32  waitAllowed;

                    for (UPInt j = 0; j < TicksNotifiers.GetSize(); j++)
                    {
                        waitAllowed = (UInt32)(TicksNotifiers[j]->OnTicks(ticksMks) / Timer::MksPerMs);
                        if (waitAllowed < waitMs)
                            waitMs = waitAllowed;
                    }
                }
                
                // Enter blocking run loop. We may continue until we timeout in which
                // case it's time to service the ticks. Or if commands arrive in the command
                // queue then the source callback will call 'CFRunLoopStop' causing this
                // to return.
                CFTimeInterval blockInterval = 0.001 * (double) waitMs;
                exitReason = CFRunLoopRunInMode(kCFRunLoopDefaultMode, blockInterval, false);

                if (exitReason == kCFRunLoopRunFinished)
                {
                    // Maybe this will occur during shutdown?
                    break;
                }
                else if (exitReason == kCFRunLoopRunStopped )
                {
                    // Commands need processing or we're being shutdown.
                    break;
                }
                else if (exitReason == kCFRunLoopRunTimedOut)
                {
                    // Timed out so that we can service our ticks callbacks.
                    continue;
                }
                else if (exitReason == kCFRunLoopRunHandledSource)
                {
                    // Should never occur since the last param when we call
                    // 'CFRunLoopRunInMode' is false.
                    OVR_ASSERT(false);
                    break;
                }
                else
                {
                    OVR_ASSERT_LOG(false, ("CFRunLoopRunInMode returned unexpected code"));
                    break;
                }
            }
            while(true);                    
        }
    }

                                   
    CFRunLoopRemoveSource(RunLoop, CommandQueueSource, kCFRunLoopDefaultMode);
    CFRelease(CommandQueueSource);
    
    LogText("OVR::DeviceManagerThread - exiting (ThreadId=0x%p).\n", GetThreadId());

    return 0;
}
    
void DeviceManagerThread::staticCommandQueueSourceCallback(void* pContext)
{
    DeviceManagerThread* pThread = (DeviceManagerThread*) pContext;
    pThread->commandQueueSourceCallback();
}
    
void DeviceManagerThread::commandQueueSourceCallback()
{    
    CFRunLoopStop(RunLoop);
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

void DeviceManagerThread::Shutdown()
{
    // Push for thread shutdown *WITH NO WAIT*.
    // This will have the following effect:
    //  - Exit command will get enqueued, which will be executed later on the thread itself.
    //  - Beyond this point, this DeviceManager object may be deleted by our caller.
    //  - Other commands, such as CreateDevice, may execute before ExitCommand, but they will
    //    fail gracefully due to pLock->pManager == 0. Future commands can't be enqued
    //    after pManager is null.
    //  - Once ExitCommand executes, ThreadCommand::Run loop will exit and release the last
    //    reference to the thread object.
    PushExitCommand(false);

    // make sure CFRunLoopRunInMode is woken up
    CFRunLoopSourceSignal(CommandQueueSource);
    CFRunLoopWakeUp(RunLoop);
}
    
} // namespace OSX


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

    Ptr<OSX::DeviceManager> manager = *new OSX::DeviceManager;

    if (manager)
    {
        if (manager->Initialize(0))
        {
            manager->AddFactory(&LatencyTestDeviceFactory::Instance);
            manager->AddFactory(&SensorDeviceFactory::Instance);
            manager->AddFactory(&OSX::HMDDeviceFactory::Instance);

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
