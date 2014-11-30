/************************************************************************************

Filename    :   OVR_Linux_DeviceManager.h
Content     :   Linux implementation of DeviceManager.
Created     :   
Authors     :   

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#include "OVR_Linux_DeviceManager.h"

// Sensor & HMD Factories
#include "OVR_LatencyTestImpl.h"
#include "OVR_SensorImpl.h"
#include "OVR_Linux_HIDDevice.h"
#include "OVR_Linux_HMDDevice.h"

#include "Kernel/OVR_Timer.h"
#include "Kernel/OVR_Std.h"
#include "Kernel/OVR_Log.h"

namespace OVR { namespace Linux {


//-------------------------------------------------------------------------------------
// **** Linux::DeviceManager

DeviceManager::DeviceManager()
{
}

DeviceManager::~DeviceManager()
{    
}

bool DeviceManager::Initialize(DeviceBase*)
{
    if (!DeviceManagerImpl::Initialize(0))
        return false;

    pThread = *new DeviceManagerThread();
    if (!pThread || !pThread->Start())
        return false;

    // Wait for the thread to be fully up and running.
    pThread->StartupEvent.Wait();

    // Do this now that we know the thread's run loop.
    HidDeviceManager = *HIDDeviceManager::CreateInternal(this);
         
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


//-------------------------------------------------------------------------------------
// ***** DeviceManager Thread 

DeviceManagerThread::DeviceManagerThread()
    : Thread(ThreadStackSize)
{
    int result = pipe(CommandFd);
    OVR_ASSERT(!result);

    AddSelectFd(NULL, CommandFd[0]);
}

DeviceManagerThread::~DeviceManagerThread()
{
    if (CommandFd[0])
    {
        RemoveSelectFd(NULL, CommandFd[0]);
        close(CommandFd[0]);
        close(CommandFd[1]);
    }
}

bool DeviceManagerThread::AddSelectFd(Notifier* notify, int fd)
{
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN|POLLHUP|POLLERR;
    pfd.revents = 0;

    FdNotifiers.PushBack(notify);
    PollFds.PushBack(pfd);

    OVR_ASSERT(FdNotifiers.GetSize() == PollFds.GetSize());
    return true;
}

bool DeviceManagerThread::RemoveSelectFd(Notifier* notify, int fd)
{
    // [0] is reserved for thread commands with notify of null, but we still
    // can use this function to remove it.
    for (UPInt i = 0; i < FdNotifiers.GetSize(); i++)
    {
        if ((FdNotifiers[i] == notify) && (PollFds[i].fd == fd))
        {
            FdNotifiers.RemoveAt(i);
            PollFds.RemoveAt(i);
            return true;
        }
    }
    return false;
}



int DeviceManagerThread::Run()
{
    ThreadCommand::PopBuffer command;

    SetThreadName("OVR::DeviceManagerThread");
    LogText("OVR::DeviceManagerThread - running (ThreadId=%p).\n", GetThreadId());
    
    // Signal to the parent thread that initialization has finished.
    StartupEvent.SetEvent();

    while(!IsExiting())
    {
        // PopCommand will reset event on empty queue.
        if (PopCommand(&command))
        {
            command.Execute();
        }
        else
        {
            bool commands = 0;
            do
            {
                int waitMs = -1;

                // If devices have time-dependent logic registered, get the longest wait
                // allowed based on current ticks.
                if (!TicksNotifiers.IsEmpty())
                {
                    UInt64 ticksMks = Timer::GetTicks();
                    int  waitAllowed;

                    for (UPInt j = 0; j < TicksNotifiers.GetSize(); j++)
                    {
                        waitAllowed = (int)(TicksNotifiers[j]->OnTicks(ticksMks) / Timer::MksPerMs);
                        if (waitAllowed < waitMs)
                            waitMs = waitAllowed;
                    }
                }

                // wait until there is data available on one of the devices or the timeout expires
                int n = poll(&PollFds[0], PollFds.GetSize(), waitMs);

                if (n > 0)
                {
                    // Iterate backwards through the list so the ordering will not be
                    // affected if the called object gets removed during the callback
                    // Also, the HID data streams are located toward the back of the list
                    // and servicing them first will allow a disconnect to be handled
                    // and cleaned directly at the device first instead of the general HID monitor
                    for (int i=PollFds.GetSize()-1; i>=0; i--)
                    {
                        if (PollFds[i].revents & POLLERR)
                        {
                            OVR_DEBUG_LOG(("poll: error on [%d]: %d", i, PollFds[i].fd));
                        }
                        else if (PollFds[i].revents & POLLIN)
                        {
                            if (FdNotifiers[i])
                                FdNotifiers[i]->OnEvent(i, PollFds[i].fd);
                            else if (i == 0) // command
                            {
                                char dummy[128];
                                read(PollFds[i].fd, dummy, 128);
                                commands = 1;
                            }
                        }

                        if (PollFds[i].revents & POLLHUP)
                            PollFds[i].events = 0;

                        if (PollFds[i].revents != 0)
                        {
                            n--;
                            if (n == 0)
                                break;
                        }
                    }
                }
            } while (PollFds.GetSize() > 0 && !commands);
        }
    }

    LogText("OVR::DeviceManagerThread - exiting (ThreadId=%p).\n", GetThreadId());
    return 0;
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

} // namespace Linux


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

    Ptr<Linux::DeviceManager> manager = *new Linux::DeviceManager;

    if (manager)
    {
        if (manager->Initialize(0))
        {            
            manager->AddFactory(&LatencyTestDeviceFactory::Instance);
            manager->AddFactory(&SensorDeviceFactory::Instance);
            manager->AddFactory(&Linux::HMDDeviceFactory::Instance);

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

