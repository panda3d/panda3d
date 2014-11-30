/************************************************************************************

Filename    :   OVR_DeviceHandle.cpp
Content     :   Implementation of device handle class
Created     :   February 5, 2013
Authors     :   Lee Cooper

Copyright   :   Copyright 2013 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#include "OVR_DeviceHandle.h"

#include "OVR_DeviceImpl.h"

namespace OVR {

//-------------------------------------------------------------------------------------
// ***** DeviceHandle

DeviceHandle::DeviceHandle(DeviceCreateDesc* impl) : pImpl(impl)
{
    if (pImpl)
        pImpl->AddRef();
}

DeviceHandle::DeviceHandle(const DeviceHandle& src) : pImpl(src.pImpl)
{
    if (pImpl)
        pImpl->AddRef();
}    

DeviceHandle::~DeviceHandle()
{
    if (pImpl)
        pImpl->Release();
}

void DeviceHandle::operator = (const DeviceHandle& src)
{
    if (src.pImpl)
        src.pImpl->AddRef();
    if (pImpl)
        pImpl->Release();
    pImpl = src.pImpl;
}

DeviceBase* DeviceHandle::GetDevice_AddRef() const
{ 
    if (pImpl && pImpl->pDevice)
    {
        pImpl->pDevice->AddRef();
        return pImpl->pDevice;
    }
    return NULL;
}

// Returns true, if the handle contains the same device ptr
// as specified in the parameter.
bool DeviceHandle::IsDevice(DeviceBase* pdev) const
{
    return (pdev && pImpl && pImpl->pDevice) ? 
        pImpl->pDevice == pdev : false;
}

DeviceType  DeviceHandle::GetType() const
{
    return pImpl ? pImpl->Type : Device_None;
}

bool DeviceHandle::GetDeviceInfo(DeviceInfo* info) const
{
    return pImpl ? pImpl->GetDeviceInfo(info) : false;
}
bool DeviceHandle::IsAvailable() const
{
    // This isn't "atomically safe", but the function only returns the
    // recent state that may change.
    return pImpl ? (pImpl->Enumerated && pImpl->pLock->pManager) : false;
}

bool DeviceHandle::IsCreated() const
{
    return pImpl ? (pImpl->pDevice != 0) : false;
}

DeviceBase* DeviceHandle::CreateDevice()
{       
    if (!pImpl)
        return 0;
    
    DeviceBase*            device = 0;
    Ptr<DeviceManagerImpl> manager= 0;

    // Since both manager and device pointers can only be destroyed during a lock,
    // hold it while checking for availability.
    // AddRef to manager so that it doesn't get released on us.
    {
        Lock::Locker deviceLockScope(pImpl->GetLock());

        if (pImpl->pDevice)
        {
            pImpl->pDevice->AddRef();
            return pImpl->pDevice;
        }
        manager = pImpl->GetManagerImpl();
    }

    if (manager)
    {
        if (manager->GetThreadId() != OVR::GetCurrentThreadId())
        {
            // Queue up a CreateDevice request. This fills in '&device' with AddRefed value,
            // or keep it at null.
            manager->GetThreadQueue()->PushCallAndWaitResult(
                manager.GetPtr(), &DeviceManagerImpl::CreateDevice_MgrThread,
                &device, pImpl, (DeviceBase*)0);
        }
        else
            device = manager->CreateDevice_MgrThread(pImpl, (DeviceBase*)0);
    }
    return device;
}

void DeviceHandle::Clear()
{
    if (pImpl)
    {
        pImpl->Release();
        pImpl = 0;
    }
}

bool DeviceHandle::enumerateNext(const DeviceEnumerationArgs& args)
{
    if (GetType() == Device_None)
        return false;
    
    Ptr<DeviceManagerImpl> managerKeepAlive;
    Lock::Locker           lockScope(pImpl->GetLock());
    
    DeviceCreateDesc* next = pImpl;
    // If manager was destroyed, we get removed from the list.
    if (!pImpl->pNext)
        return false;

    managerKeepAlive = next->GetManagerImpl();
    OVR_ASSERT(managerKeepAlive);
    
    do {
        next = next->pNext;

        if (managerKeepAlive->Devices.IsNull(next))
        {
            pImpl->Release();
            pImpl = 0;
            return false;
        }

    } while(!args.MatchRule(next->Type, next->Enumerated));

    next->AddRef();
    pImpl->Release();
    pImpl = next;

    return true;
}

} // namespace OVR

