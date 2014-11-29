/************************************************************************************

Filename    :   OVR_DeviceImpl.h
Content     :   Partial back-end independent implementation of Device interfaces
Created     :   October 10, 2012
Authors     :   Michael Antonov

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#include "OVR_DeviceImpl.h"
#include "Kernel/OVR_Atomic.h"
#include "Kernel/OVR_Log.h"
#include "Kernel/OVR_System.h"

#include "OVR_DeviceImpl.h"
#include "OVR_SensorImpl.h"
#include "OVR_Profile.h"

namespace OVR {


//-------------------------------------------------------------------------------------
// ***** SharedLock

// This is a general purpose globally shared Lock implementation that should probably be
// moved to Kernel.
// May in theory busy spin-wait if we hit contention on first lock creation,
// but this shouldn't matter in practice since Lock* should be cached.


enum { LockInitMarker = 0xFFFFFFFF };

Lock* SharedLock::GetLockAddRef()
{
    int oldUseCount;

    do {
        oldUseCount = UseCount;
        if (oldUseCount == LockInitMarker)
            continue;

        if (oldUseCount == 0)
        {
            // Initialize marker
            if (AtomicOps<int>::CompareAndSet_Sync(&UseCount, 0, LockInitMarker))
            {
                Construct<Lock>(Buffer);
                do { }
                while (!AtomicOps<int>::CompareAndSet_Sync(&UseCount, LockInitMarker, 1));
                return toLock();
            }
            continue;
        }

    } while (!AtomicOps<int>::CompareAndSet_NoSync(&UseCount, oldUseCount, oldUseCount + 1));

    return toLock();
}

void SharedLock::ReleaseLock(Lock* plock)
{
    OVR_UNUSED(plock);
    OVR_ASSERT(plock == toLock());

    int oldUseCount;

    do {
        oldUseCount = UseCount;
        OVR_ASSERT(oldUseCount != LockInitMarker);

        if (oldUseCount == 1)
        {
            // Initialize marker
            if (AtomicOps<int>::CompareAndSet_Sync(&UseCount, 1, LockInitMarker))
            {
                Destruct<Lock>(toLock());

                do { }
                while (!AtomicOps<int>::CompareAndSet_Sync(&UseCount, LockInitMarker, 0));

                return;
            }
            continue;
        }

    } while (!AtomicOps<int>::CompareAndSet_NoSync(&UseCount, oldUseCount, oldUseCount - 1));
}



//-------------------------------------------------------------------------------------
// ***** MessageHandler

// Threading notes:
// The OnMessage() handler and SetMessageHandler are currently synchronized
// through a separately stored shared Lock object to avoid calling the handler 
// from background thread while it's being removed.

static SharedLock MessageHandlerSharedLock;


class MessageHandlerImpl
{
public:
    MessageHandlerImpl()
        : pLock(MessageHandlerSharedLock.GetLockAddRef())
    {
    }
    ~MessageHandlerImpl()
    {
        MessageHandlerSharedLock.ReleaseLock(pLock);
        pLock = 0;
    }

    static MessageHandlerImpl* FromHandler(MessageHandler* handler)
    { return (MessageHandlerImpl*)&handler->Internal; }
    static const MessageHandlerImpl* FromHandler(const MessageHandler* handler)
    { return (const MessageHandlerImpl*)&handler->Internal; }

    // This lock is held while calling a handler and when we are applied/
    // removed from a device.
    Lock*                     pLock;
    // List of device we are applied to.
    List<MessageHandlerRef>   UseList;
};


MessageHandlerRef::MessageHandlerRef(DeviceBase* device)
    : pLock(MessageHandlerSharedLock.GetLockAddRef()), pDevice(device), pHandler(0)
{
}

MessageHandlerRef::~MessageHandlerRef()
{
    {
        Lock::Locker lockScope(pLock);
        if (pHandler)
        {
            pHandler = 0;
            RemoveNode();
        }
    }
    MessageHandlerSharedLock.ReleaseLock(pLock);
    pLock = 0;
}

void MessageHandlerRef::SetHandler(MessageHandler* handler)
{    
    OVR_ASSERT(!handler ||
               MessageHandlerImpl::FromHandler(handler)->pLock == pLock);    
    Lock::Locker lockScope(pLock);
    SetHandler_NTS(handler);
}

void MessageHandlerRef::SetHandler_NTS(MessageHandler* handler)
{   
    if (pHandler != handler)
    {
        if (pHandler)
            RemoveNode();
        pHandler = handler;

        if (handler)
        {
            MessageHandlerImpl* handlerImpl = MessageHandlerImpl::FromHandler(handler);
            handlerImpl->UseList.PushBack(this);
        }
        // TBD: Call notifier on device?
    }
}


MessageHandler::MessageHandler()
{    
    OVR_COMPILER_ASSERT(sizeof(Internal) > sizeof(MessageHandlerImpl));
    Construct<MessageHandlerImpl>(Internal);
}

MessageHandler::~MessageHandler()
{
    MessageHandlerImpl* handlerImpl = MessageHandlerImpl::FromHandler(this);
    {
        Lock::Locker lockedScope(handlerImpl->pLock);
        OVR_ASSERT_LOG(handlerImpl->UseList.IsEmpty(),
            ("~MessageHandler %p - Handler still active; call RemoveHandlerFromDevices", this));
    }

    Destruct<MessageHandlerImpl>(handlerImpl);    
}

bool MessageHandler::IsHandlerInstalled() const
{
    const MessageHandlerImpl* handlerImpl = MessageHandlerImpl::FromHandler(this);    
    Lock::Locker lockedScope(handlerImpl->pLock);
    return handlerImpl->UseList.IsEmpty() != true;
}


void MessageHandler::RemoveHandlerFromDevices()
{
    MessageHandlerImpl* handlerImpl = MessageHandlerImpl::FromHandler(this);
    Lock::Locker lockedScope(handlerImpl->pLock);

    while(!handlerImpl->UseList.IsEmpty())
    {
        MessageHandlerRef* use = handlerImpl->UseList.GetFirst();
        use->SetHandler_NTS(0);
    }
}

Lock* MessageHandler::GetHandlerLock() const
{
    const MessageHandlerImpl* handlerImpl = MessageHandlerImpl::FromHandler(this);
    return handlerImpl->pLock;
}


//-------------------------------------------------------------------------------------
// ***** DeviceBase
   

// Delegate relevant implementation to DeviceRectord to avoid re-implementation in
// every derived Device.
void DeviceBase::AddRef()
{
    getDeviceCommon()->DeviceAddRef();
}
void DeviceBase::Release()
{
    getDeviceCommon()->DeviceRelease();
}
DeviceBase* DeviceBase::GetParent() const
{
    return getDeviceCommon()->pParent.GetPtr();
}
DeviceManager* DeviceBase::GetManager() const
{
    return getDeviceCommon()->pCreateDesc->GetManagerImpl();
}

void DeviceBase::SetMessageHandler(MessageHandler* handler)
{
    getDeviceCommon()->HandlerRef.SetHandler(handler);
}
MessageHandler* DeviceBase::GetMessageHandler() const
{
    return getDeviceCommon()->HandlerRef.GetHandler();
}

DeviceType DeviceBase::GetType() const
{
    return getDeviceCommon()->pCreateDesc->Type;
}

bool DeviceBase::GetDeviceInfo(DeviceInfo* info) const
{
    return getDeviceCommon()->pCreateDesc->GetDeviceInfo(info);
    //info->Name[0] = 0;
    //return false;
}

// returns the MessageHandler's lock
Lock* DeviceBase::GetHandlerLock() const
{
    return getDeviceCommon()->HandlerRef.GetLock();
}

// Derive DeviceManagerCreateDesc to provide abstract function implementation.
class DeviceManagerCreateDesc : public DeviceCreateDesc
{
public:
    DeviceManagerCreateDesc(DeviceFactory* factory)
        : DeviceCreateDesc(factory, Device_Manager) { }

    // We don't need there on Manager since it isn't assigned to DeviceHandle.
    virtual DeviceCreateDesc* Clone() const                        { return 0; }
    virtual MatchResult MatchDevice(const DeviceCreateDesc&,
                                    DeviceCreateDesc**) const      { return Match_None; }
    virtual DeviceBase* NewDeviceInstance()                        { return 0; }
    virtual bool        GetDeviceInfo(DeviceInfo*) const           { return false; }
};

//-------------------------------------------------------------------------------------
// ***** DeviceManagerImpl

DeviceManagerImpl::DeviceManagerImpl()
    : DeviceImpl<OVR::DeviceManager>(CreateManagerDesc(), 0)
      //,DeviceCreateDescList(pCreateDesc ? pCreateDesc->pLock : 0)
{
    if (pCreateDesc)
    {
        pCreateDesc->pLock->pManager = this;
    }
}

DeviceManagerImpl::~DeviceManagerImpl()
{
    // Shutdown must've been called.
    OVR_ASSERT(!pCreateDesc->pDevice);

    // Remove all factories
    while(!Factories.IsEmpty())
    {
        DeviceFactory* factory = Factories.GetFirst();
        factory->RemovedFromManager();
        factory->RemoveNode();
    }
}

DeviceCreateDesc* DeviceManagerImpl::CreateManagerDesc()
{
    DeviceCreateDesc* managerDesc = new DeviceManagerCreateDesc(0);
    if (managerDesc)
    {
        managerDesc->pLock = *new DeviceManagerLock;
    }
    return managerDesc;
}

bool DeviceManagerImpl::Initialize(DeviceBase* parent)
{
    OVR_UNUSED(parent);
    if (!pCreateDesc || !pCreateDesc->pLock)
		return false;

    pProfileManager = *ProfileManager::Create();

    return true;
}

void DeviceManagerImpl::Shutdown()
{
    // Remove all device descriptors from list while the lock is held.
    // Some descriptors may survive longer due to handles.    
    while(!Devices.IsEmpty())
    {     
        DeviceCreateDesc* devDesc = Devices.GetFirst();
        OVR_ASSERT(!devDesc->pDevice); // Manager shouldn't be dying while Device exists.
        devDesc->Enumerated = false;
        devDesc->RemoveNode();
        devDesc->pNext = devDesc->pPrev = 0;

        if (devDesc->HandleCount == 0)
        {
            delete devDesc;
        }
    }
    Devices.Clear();

    // These must've been cleared by caller.
    OVR_ASSERT(pCreateDesc->pDevice == 0);
    OVR_ASSERT(pCreateDesc->pLock->pManager == 0);

    pProfileManager.Clear();
}


// Callbacks for DeviceCreation/Release    
DeviceBase* DeviceManagerImpl::CreateDevice_MgrThread(DeviceCreateDesc* createDesc, DeviceBase* parent)
{
    // Calls to DeviceManagerImpl::CreateDevice are enqueued with wait while holding pManager,
    // so 'this' must remain valid.
    OVR_ASSERT(createDesc->pLock->pManager);    

    Lock::Locker devicesLock(GetLock());

    // If device already exists, just AddRef to it.
    if (createDesc->pDevice)
    {
        createDesc->pDevice->AddRef();
        return createDesc->pDevice;
    }

    if (!parent)
        parent = this;

    DeviceBase* device = createDesc->NewDeviceInstance();
    
    if (device)
    {
        if (device->getDeviceCommon()->Initialize(parent))
        {
           createDesc->pDevice = device;
        }
        else
        {
            // Don't go through Release() to avoid PushCall behaviour,
            // as it is not needed here.
            delete device;
            device = 0;
        }
    }
     
    return device;
}

Void DeviceManagerImpl::ReleaseDevice_MgrThread(DeviceBase* device)
{
    // descKeepAlive will keep ManagerLock object alive as well,
    // allowing us to exit gracefully.    
    Ptr<DeviceCreateDesc>  descKeepAlive;
    Lock::Locker           devicesLock(GetLock());
    DeviceCommon*          devCommon = device->getDeviceCommon();

    while(1)
    {
        UInt32 refCount = devCommon->RefCount;

        if (refCount > 1)
        {
            if (devCommon->RefCount.CompareAndSet_NoSync(refCount, refCount-1))
            {
                // We decreented from initial count higher then 1;
                // nothing else to do.
                return 0;
            }        
        }
        else if (devCommon->RefCount.CompareAndSet_NoSync(1, 0))
        {
            // { 1 -> 0 } decrement succeded. Destroy this device.
            break;
        }
    }

    // At this point, may be releasing the device manager itself.
    // This does not matter, however, since shutdown logic is the same
    // in both cases. DeviceManager::Shutdown with begin shutdown process for
    // the internal manager thread, which will eventually destroy itself.
    // TBD: Clean thread shutdown.
    descKeepAlive = devCommon->pCreateDesc;
    descKeepAlive->pDevice = 0;
    devCommon->Shutdown();
    delete device;
    return 0;
}



Void DeviceManagerImpl::EnumerateAllFactoryDevices()
{
    // 1. Mark matching devices as NOT enumerated.
    // 2. Call factory to enumerate all HW devices, adding any device that 
    //    was not matched.
    // 3. Remove non-matching devices.

    Lock::Locker deviceLock(GetLock());

    DeviceCreateDesc* devDesc, *nextdevDesc;

    // 1.
    for(devDesc = Devices.GetFirst();
        !Devices.IsNull(devDesc);  devDesc = devDesc->pNext)
    {
        //if (devDesc->pFactory == factory)
            devDesc->Enumerated = false;
    }
    
    // 2.
    DeviceFactory* factory = Factories.GetFirst();
    while(!Factories.IsNull(factory))
    {
        EnumerateFactoryDevices(factory);
        factory = factory->pNext;
    }

    
    // 3.
    for(devDesc = Devices.GetFirst();
        !Devices.IsNull(devDesc);  devDesc = nextdevDesc)
    {
        // In case 'devDesc' gets removed.
        nextdevDesc = devDesc->pNext; 

        // Note, device might be not enumerated since it is opened and
        // in use! Do NOT notify 'device removed' in this case (!AB)
        if (!devDesc->Enumerated)
        {
            // This deletes the devDesc for HandleCount == 0 due to Release in DeviceHandle.
            CallOnDeviceRemoved(devDesc);

            /*
            if (devDesc->HandleCount == 0)
            {                
                // Device must be dead if it ever existed, since it AddRefs to us.
                // ~DeviceCreateDesc removes its node from list.
                OVR_ASSERT(!devDesc->pDevice);
                delete devDesc;
            }
            */
        }
    }

    return 0;
}

Ptr<DeviceCreateDesc> DeviceManagerImpl::AddDevice_NeedsLock(
    const DeviceCreateDesc& createDesc)
{
    // If found, mark as enumerated and we are done.
    DeviceCreateDesc* descCandidate = 0;

    for(DeviceCreateDesc* devDesc = Devices.GetFirst();
        !Devices.IsNull(devDesc);  devDesc = devDesc->pNext)
    {
        DeviceCreateDesc::MatchResult mr = devDesc->MatchDevice(createDesc, &descCandidate);
        if (mr == DeviceCreateDesc::Match_Found)
        {
            devDesc->Enumerated = true;
            if (!devDesc->pDevice)
                CallOnDeviceAdded(devDesc);
            return devDesc;
        }
    }

    // Update candidate (this may involve writing fields to HMDDevice createDesc).
    if (descCandidate)
    {
        bool newDevice = false;
        if (descCandidate->UpdateMatchedCandidate(createDesc, &newDevice))
        {
            descCandidate->Enumerated = true;
            if (!descCandidate->pDevice || newDevice)
                CallOnDeviceAdded(descCandidate);
            return descCandidate;
        }
    }

    // If not found, add new device.
    //  - This stores a new descriptor with
    //    {pDevice = 0, HandleCount = 1, Enumerated = true}
    DeviceCreateDesc* desc = createDesc.Clone();
    desc->pLock = pCreateDesc->pLock;
    Devices.PushBack(desc);
    desc->Enumerated = true;

    CallOnDeviceAdded(desc);

    return desc;
}

Ptr<DeviceCreateDesc> DeviceManagerImpl::FindDevice(
    const String& path, 
    DeviceType deviceType)
{
    Lock::Locker deviceLock(GetLock());
    DeviceCreateDesc* devDesc;

    for (devDesc = Devices.GetFirst();
        !Devices.IsNull(devDesc);  devDesc = devDesc->pNext)
    {
        if ((deviceType == Device_None || deviceType == devDesc->Type) &&
            devDesc->MatchDevice(path))
            return devDesc;
    }
    return NULL;
}

Ptr<DeviceCreateDesc> DeviceManagerImpl::FindHIDDevice(const HIDDeviceDesc& hidDevDesc)
{
    Lock::Locker deviceLock(GetLock());
    DeviceCreateDesc* devDesc;
    
    for (devDesc = Devices.GetFirst();
        !Devices.IsNull(devDesc);  devDesc = devDesc->pNext)
    {
        if (devDesc->MatchHIDDevice(hidDevDesc))
            return devDesc;
    }
    return NULL;
}
  
void DeviceManagerImpl::DetectHIDDevice(const HIDDeviceDesc& hidDevDesc)
{
    Lock::Locker deviceLock(GetLock());
    DeviceFactory* factory = Factories.GetFirst();
    while(!Factories.IsNull(factory))
    {
        if (factory->DetectHIDDevice(this, hidDevDesc))
            break;
        factory = factory->pNext;
    }
    
}
    
// Enumerates devices for a particular factory.
Void DeviceManagerImpl::EnumerateFactoryDevices(DeviceFactory* factory)
{
       
    class FactoryEnumerateVisitor : public DeviceFactory::EnumerateVisitor
    {        
        DeviceManagerImpl* pManager;
        DeviceFactory*     pFactory;
    public:
        FactoryEnumerateVisitor(DeviceManagerImpl* manager, DeviceFactory* factory)
            : pManager(manager), pFactory(factory) { }

        virtual void Visit(const DeviceCreateDesc& createDesc)
        {
            pManager->AddDevice_NeedsLock(createDesc);
        }
    };

    FactoryEnumerateVisitor newDeviceVisitor(this, factory);
    factory->EnumerateDevices(newDeviceVisitor);


    return 0;
}


DeviceEnumerator<> DeviceManagerImpl::EnumerateDevicesEx(const DeviceEnumerationArgs& args)
{
    Lock::Locker deviceLock(GetLock());
    
    if (Devices.IsEmpty())
        return DeviceEnumerator<>();

    DeviceCreateDesc*  firstDeviceDesc = Devices.GetFirst();
    DeviceEnumerator<> e = enumeratorFromHandle(DeviceHandle(firstDeviceDesc), args);

    if (!args.MatchRule(firstDeviceDesc->Type, firstDeviceDesc->Enumerated))
    {
        e.Next();
    }
    
    return e;
}

//-------------------------------------------------------------------------------------
// ***** DeviceCommon

void DeviceCommon::DeviceAddRef()
{
    RefCount++;
}

void DeviceCommon::DeviceRelease()
{
    while(1)
    {
        UInt32 refCount = RefCount;
        OVR_ASSERT(refCount > 0);
        
        if (refCount == 1)
        {
            DeviceManagerImpl*  manager = pCreateDesc->GetManagerImpl();
            ThreadCommandQueue* queue   = manager->GetThreadQueue();

            // Enqueue ReleaseDevice for {1 -> 0} transition with no wait.
            // We pass our reference ownership into the queue to destroy.
            // It's in theory possible for another thread to re-steal our device reference,
            // but that is checked for atomically in DeviceManagerImpl::ReleaseDevice.
            if (!queue->PushCall(manager, &DeviceManagerImpl::ReleaseDevice_MgrThread,
                                          pCreateDesc->pDevice))
            {
                // PushCall shouldn't fail because background thread runs while manager is
                // alive and we are holding Manager alive through pParent chain.
                OVR_ASSERT(false);
            }

            // Warning! At his point everything, including manager, may be dead.
            break;
        }
        else if (RefCount.CompareAndSet_NoSync(refCount, refCount-1))
        {
            break;
        }
    }
}



//-------------------------------------------------------------------------------------
// ***** DeviceCreateDesc


void DeviceCreateDesc::AddRef()
{
    // Technically, HandleCount { 0 -> 1 } transition can only happen during Lock,
    // but we leave this to caller to worry about (happens during enumeration).
    HandleCount++;
}

void DeviceCreateDesc::Release()
{
    while(1)
    {
        UInt32 handleCount = HandleCount;
        // HandleCount must obviously be >= 1, since we are releasing it.
        OVR_ASSERT(handleCount > 0);

        // {1 -> 0} transition may cause us to be destroyed, so require a lock.
        if (handleCount == 1)
        {       
            Ptr<DeviceManagerLock>  lockKeepAlive;
            Lock::Locker            deviceLockScope(GetLock());

            if (!HandleCount.CompareAndSet_NoSync(handleCount, 0))
                continue;
            
            OVR_ASSERT(pDevice == 0);

            // Destroy *this if the manager was destroyed already, or Enumerated
            // is false (device no longer available).           
            if (!GetManagerImpl() || !Enumerated)
            {
                lockKeepAlive = pLock;

                // Remove from manager list (only matters for !Enumerated).
                if (pNext)
                {
                    RemoveNode();
                    pNext = pPrev = 0;
                }

                delete this;
            }

            // Available DeviceCreateDesc may survive with { HandleCount == 0 },
            // in case it might be enumerated again later.
            break;
        }
        else if (HandleCount.CompareAndSet_NoSync(handleCount, handleCount-1))
        {
            break;
        }
    }
}

HMDDevice* HMDDevice::Disconnect(SensorDevice* psensor)
{
    if (!psensor)
        return NULL;

    OVR::DeviceManager* manager = GetManager();
    if (manager)
    {
        //DeviceManagerImpl* mgrImpl = static_cast<DeviceManagerImpl*>(manager);
        Ptr<DeviceCreateDesc> desc = getDeviceCommon()->pCreateDesc;
        if (desc)
        {
            class Visitor : public DeviceFactory::EnumerateVisitor
            {
                Ptr<DeviceCreateDesc> Desc;
            public:
                Visitor(DeviceCreateDesc* desc) : Desc(desc) {}
                virtual void Visit(const DeviceCreateDesc& createDesc) 
                {
                    Lock::Locker lock(Desc->GetLock());
                    Desc->UpdateMatchedCandidate(createDesc);
                }
            } visitor(desc);
            //SensorDeviceImpl* sImpl = static_cast<SensorDeviceImpl*>(psensor);

            SensorDisplayInfoImpl displayInfo;

            if (psensor->GetFeatureReport(displayInfo.Buffer, SensorDisplayInfoImpl::PacketSize))
            {
                displayInfo.Unpack();

                // If we got display info, try to match / create HMDDevice as well
                // so that sensor settings give preference.
                if (displayInfo.DistortionType & SensorDisplayInfoImpl::Mask_BaseFmt)
                {
                    SensorDeviceImpl::EnumerateHMDFromSensorDisplayInfo(displayInfo, visitor);
                }
            }
        }
    }
    return this;
}

bool  HMDDevice::IsDisconnected() const
{
    OVR::HMDInfo info;
    GetDeviceInfo(&info);
    // if strlen(info.DisplayDeviceName) == 0 then
    // this HMD is 'fake' (created using sensor).
    return (strlen(info.DisplayDeviceName) == 0);
}


} // namespace OVR

