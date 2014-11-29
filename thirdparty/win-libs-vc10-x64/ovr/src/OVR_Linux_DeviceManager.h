/************************************************************************************

Filename    :   OVR_Linux_DeviceManager.h
Content     :   Linux-specific DeviceManager header.
Created     :   
Authors     :   

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#ifndef OVR_Linux_DeviceManager_h
#define OVR_Linux_DeviceManager_h

#include "OVR_DeviceImpl.h"

#include <unistd.h>
#include <sys/poll.h>


namespace OVR { namespace Linux {

class DeviceManagerThread;

//-------------------------------------------------------------------------------------
// ***** Linux DeviceManager

class DeviceManager : public DeviceManagerImpl
{
public:
    DeviceManager();
    ~DeviceManager();

    // Initialize/Shutdowncreate and shutdown manger thread.
    virtual bool Initialize(DeviceBase* parent);
    virtual void Shutdown();

    virtual ThreadCommandQueue* GetThreadQueue();
    virtual ThreadId GetThreadId() const;

    virtual DeviceEnumerator<> EnumerateDevicesEx(const DeviceEnumerationArgs& args);    

    virtual bool  GetDeviceInfo(DeviceInfo* info) const;

    Ptr<DeviceManagerThread> pThread;
};

//-------------------------------------------------------------------------------------
// ***** Device Manager Background Thread

class DeviceManagerThread : public Thread, public ThreadCommandQueue
{
    friend class DeviceManager;
    enum { ThreadStackSize = 64 * 1024 };
public:
    DeviceManagerThread();
    ~DeviceManagerThread();

    virtual int Run();

    // ThreadCommandQueue notifications for CommandEvent handling.
    virtual void OnPushNonEmpty_Locked() { write(CommandFd[1], this, 1); }
    virtual void OnPopEmpty_Locked()     { }

    class Notifier
    {
    public:
        // Called when I/O is received
        virtual void OnEvent(int i, int fd) = 0;

        // Called when timing ticks are updated.
        // Returns the largest number of microseconds this function can
        // wait till next call.
        virtual UInt64  OnTicks(UInt64 ticksMks)
        {
            OVR_UNUSED1(ticksMks);
            return Timer::MksPerSecond * 1000;
        }
    };

    // Add I/O notifier
    bool AddSelectFd(Notifier* notify, int fd);
    bool RemoveSelectFd(Notifier* notify, int fd);

    // Add notifier that will be called at regular intervals.
    bool AddTicksNotifier(Notifier* notify);
    bool RemoveTicksNotifier(Notifier* notify);

private:
    
    bool threadInitialized() { return CommandFd[0] != 0; }

    // pipe used to signal commands
    int CommandFd[2];

    Array<struct pollfd>    PollFds;
    Array<Notifier*>        FdNotifiers;

    Event                   StartupEvent;

    // Ticks notifiers - used for time-dependent events such as keep-alive.
    Array<Notifier*>        TicksNotifiers;
};

}} // namespace Linux::OVR

#endif // OVR_Linux_DeviceManager_h
