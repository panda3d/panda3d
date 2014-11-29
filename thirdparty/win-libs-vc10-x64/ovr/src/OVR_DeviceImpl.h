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

#ifndef OVR_DeviceImpl_h
#define OVR_DeviceImpl_h

#include "OVR_Device.h"
#include "Kernel/OVR_Atomic.h"
#include "Kernel/OVR_Log.h"
#include "Kernel/OVR_System.h"

#include "Kernel/OVR_Threads.h"
#include "OVR_ThreadCommandQueue.h"
#include "OVR_HIDDevice.h"

namespace OVR {
    
class DeviceManagerImpl;
class DeviceFactory;

enum
{
    Oculus_VendorId = 0x2833
};

//-------------------------------------------------------------------------------------
// Globally shared Lock implementation used for MessageHandlers.

class SharedLock
{    
public:
    SharedLock() : UseCount(0) {}

    Lock* GetLockAddRef();
    void  ReleaseLock(Lock* plock);
   
private:
    Lock* toLock() { return (Lock*)Buffer; }

    // UseCount and max alignment.
    volatile int    UseCount;
    UInt64          Buffer[(sizeof(Lock)+sizeof(UInt64)-1)/sizeof(UInt64)];
};


// Wrapper for MessageHandler that includes synchronization logic.
// References to MessageHandlers are organized in a list to allow for them to
// easily removed with MessageHandler::RemoveAllHandlers.
class MessageHandlerRef : public ListNode<MessageHandlerRef>
{    
public:
    MessageHandlerRef(DeviceBase* device);
    ~MessageHandlerRef();

    void SetHandler(MessageHandler* hander);

    // Not-thread-safe version
    void SetHandler_NTS(MessageHandler* hander);
    
    void Call(const Message& msg)
    {
        Lock::Locker lockScope(pLock);
        if (pHandler)
            pHandler->OnMessage(msg);
    }

    Lock*           GetLock() const { return pLock; }

    // GetHandler() is not thread safe if used out of order across threads; nothing can be done
    // about that.
    MessageHandler* GetHandler() const { return pHandler; }
    DeviceBase*     GetDevice() const  { return pDevice; }

private:
    Lock*           pLock;   // Cached global handler lock.
    DeviceBase*     pDevice;
    MessageHandler* pHandler;
};



//-------------------------------------------------------------------------------------

// DeviceManagerLock is a synchronization lock used by DeviceManager for Devices
// and is allocated separately for potentially longer lifetime.
// 
// DeviceManagerLock is used for all of the following:
//  - Adding/removing devices
//  - Reporting manager lifetime (pManager != 0) for DeviceHandles
//  - Protecting device creation/shutdown.

class DeviceManagerLock : public RefCountBase<DeviceManagerLock>
{
public:
    Lock                CreateLock;
    DeviceManagerImpl*  pManager;

    DeviceManagerLock() : pManager(0) { }
};


// DeviceCreateDesc provides all of the information needed to create any device, a derived
// instance of this class is created by DeviceFactory during enumeration.
//   - DeviceCreateDesc may or may not be a part of DeviceManager::Devices list (check pNext != 0).
//   - Referenced and kept alive by DeviceHandle.

class DeviceCreateDesc : public ListNode<DeviceCreateDesc>, public NewOverrideBase
{    
    void operator = (const DeviceCreateDesc&) { } // Assign not supported; suppress MSVC warning.
public:
    DeviceCreateDesc(DeviceFactory* factory, DeviceType type)
        : pFactory(factory), Type(type), pLock(0), HandleCount(0), pDevice(0), Enumerated(true)
    {
        pNext = pPrev = 0;
    }

    virtual ~DeviceCreateDesc()
    {
        OVR_ASSERT(!pDevice);
        if (pNext)        
            RemoveNode();
    }

    DeviceManagerImpl* GetManagerImpl() const { return pLock->pManager; }
    Lock*              GetLock() const        { return &pLock->CreateLock; }

    // DeviceCreateDesc reference counting is tied to Devices list management,
    // see comments for HandleCount.
    void AddRef();
    void Release();


    // *** Device creation/matching Interface


    // Cloning copies us to an allocated object when new device is enumerated.
    virtual DeviceCreateDesc* Clone() const = 0;
    // Creates a new device instance without Initializing it; the
    // later is done my Initialize()/Shutdown() methods of the device itself.
    virtual DeviceBase*       NewDeviceInstance() = 0;
    // Override to return device-specific info.
    virtual bool              GetDeviceInfo(DeviceInfo* info) const = 0;


    enum MatchResult
    {
        Match_None,
        Match_Found,
        Match_Candidate
    };

    // Override to return Match_Found if descriptor matches our device.
    // Match_Candidate can be returned, with pcandicate update, if this may be a match
    // but more searching is necessary. If this is the case UpdateMatchedCandidate will be called.
    virtual MatchResult       MatchDevice(const DeviceCreateDesc& other,
                                          DeviceCreateDesc** pcandidate) const = 0;
    
    // Called for matched candidate after all potential matches are iterated.
    // Used to update HMDevice creation arguments from Sensor.
    // Optional return param 'newDeviceFlag' will be set to true if the 
    // 'desc' refers to a new device; false, otherwise.
    // Return 'false' to create new object, 'true' if done with this argument.
    virtual bool              UpdateMatchedCandidate(
        const DeviceCreateDesc& desc, bool* newDeviceFlag = NULL) 
    { OVR_UNUSED2(desc, newDeviceFlag); return false; }

    // Matches HID device to the descriptor.
    virtual bool              MatchHIDDevice(const HIDDeviceDesc&) const { return false; }

    // Matches device by path.
    virtual bool              MatchDevice(const String& /*path*/) { return false; }
//protected:
    DeviceFactory* const        pFactory;
    const DeviceType            Type;

    // List in which this descriptor lives. pList->CreateLock required if added/removed.
    Ptr<DeviceManagerLock>      pLock;    

    // Strong references to us: Incremented by Device, DeviceHandles & Enumerators.
    // May be 0 if device not created and there are no handles.
    // Following transitions require pList->CreateLock:
    //  {1 -> 0}: May delete & remove handle if no longer available.
    //  {0 -> 1}: Device creation is only possible if manager is still alive.
    AtomicInt<UInt32>           HandleCount;
    // If not null, points to our created device instance. Modified during lock only.
    DeviceBase*                 pDevice;
    // True if device is marked as available during enumeration.
    bool                        Enumerated;
};



// Common data present in the implementation of every DeviceBase.
// Injected by DeviceImpl.
class DeviceCommon
{
public:
    AtomicInt<UInt32>      RefCount;
    Ptr<DeviceCreateDesc>  pCreateDesc;
    Ptr<DeviceBase>        pParent;
    MessageHandlerRef      HandlerRef;

    DeviceCommon(DeviceCreateDesc* createDesc, DeviceBase* device, DeviceBase* parent)
        : RefCount(1), pCreateDesc(createDesc), pParent(parent), HandlerRef(device)
    {
    }

    // Device reference counting delegates to Manager thread to actually kill devices.
    void DeviceAddRef();
    void DeviceRelease();

    Lock* GetLock() const { return pCreateDesc->GetLock(); }

    virtual bool Initialize(DeviceBase* parent) = 0;
    virtual void Shutdown() = 0;
};


//-------------------------------------------------------------------------------------
// DeviceImpl address DeviceRecord implementation to a device base class B.
// B must be derived form DeviceBase.

template<class B>
class DeviceImpl : public B, public DeviceCommon
{
public:
    DeviceImpl(DeviceCreateDesc* createDesc, DeviceBase* parent)
        : DeviceCommon(createDesc, getThis(), parent)        
    {
    }

	// Convenience method to avoid manager access typecasts.
    DeviceManagerImpl*  GetManagerImpl() const      { return pCreateDesc->pLock->pManager; }

    // Inline to avoid warnings.
    DeviceImpl*         getThis()                   { return this; }

    // Common implementation delegate to avoid virtual inheritance and dynamic casts.
    virtual DeviceCommon* getDeviceCommon() const   { return (DeviceCommon*)this; }

    /*
    virtual void            AddRef()                                   { pCreateDesc->DeviceAddRef(); }
    virtual void            Release()                                  { pCreateDesc->DeviceRelease(); }
    virtual DeviceBase*     GetParent() const                          { return pParent.GetPtr(); } 
    virtual DeviceManager*  GetManager() const                         { return pCreateDesc->pLock->pManager;}
    virtual void            SetMessageHandler(MessageHandler* handler) { HanderRef.SetHandler(handler); }
    virtual MessageHandler* GetMessageHandler() const                  { return HanderRef.GetHandler(); }
    virtual DeviceType      GetType() const                            { return pCreateDesc->Type; }
    virtual DeviceType      GetType() const                            { return pCreateDesc->Type; }
    */
};


//-------------------------------------------------------------------------------------
// ***** DeviceFactory

// DeviceFactory is maintained in DeviceManager for each separately-enumerable
// device type; factories allow separation of unrelated enumeration code.

class DeviceFactory : public ListNode<DeviceFactory>, public NewOverrideBase
{    
public:

    DeviceFactory() : pManager(0)
    {
        pNext = pPrev = 0;
    }
    virtual ~DeviceFactory() { }

    DeviceManagerImpl* GetManagerImpl() { return pManager; }

    // Notifiers called when we are added to/removed from a device.
    virtual bool AddedToManager(DeviceManagerImpl* manager)
    {
        OVR_ASSERT(pManager == 0);
        pManager = manager;
        return true;
    }

    virtual void RemovedFromManager()
    {
        pManager = 0;
    }


    // *** Device Enumeration/Creation Support
    
    // Passed to EnumerateDevices to be informed of every device detected.
    class EnumerateVisitor
    {
    public:        
        virtual void Visit(const DeviceCreateDesc& createDesc) = 0;
    };

    // Enumerates factory devices by notifying EnumerateVisitor about every
    // device that is present.
    virtual void EnumerateDevices(EnumerateVisitor& visitor) = 0;

    // Matches vendorId/productId pair with the factory; returns 'true'
    // if the factory can handle the device.
    virtual bool MatchVendorProduct(UInt16 vendorId, UInt16 productId) const
    {
        OVR_UNUSED2(vendorId, productId);
        return false;
    }

    // Detects the HID device and adds the DeviceCreateDesc into Devices list, if
    // the device belongs to this factory. Returns 'false', if not.
    virtual bool DetectHIDDevice(DeviceManager* pdevMgr, const HIDDeviceDesc& desc)
    {
        OVR_UNUSED2(pdevMgr, desc);
        return false;
    }
    
protected:
    DeviceManagerImpl* pManager;
};


//-------------------------------------------------------------------------------------
// ***** DeviceManagerImpl

// DeviceManagerImpl is a partial default DeviceManager implementation that
// maintains a list of devices and supports their enumeration.

class DeviceManagerImpl : public DeviceImpl<OVR::DeviceManager>, public ThreadCommandQueue
{
public:
    DeviceManagerImpl();
    ~DeviceManagerImpl();

    // Constructor helper function to create Descriptor and manager lock during initialization.
    static DeviceCreateDesc* CreateManagerDesc();

    // DeviceManagerImpl provides partial implementation of Initialize/Shutdown that must
    // be called by the platform-specific derived class.
    virtual bool Initialize(DeviceBase* parent);
    virtual void Shutdown();


    // Every DeviceManager has an associated profile manager, which is used to store
    // user settings that may affect device behavior. 
    virtual ProfileManager* GetProfileManager() const { return pProfileManager.GetPtr(); }

    // Override to return ThreadCommandQueue implementation used to post commands
    // to the background device manager thread (that must be created by Initialize).
    virtual ThreadCommandQueue* GetThreadQueue() = 0;

    // Returns the thread id of the DeviceManager.
    virtual ThreadId GetThreadId() const = 0;

    virtual DeviceEnumerator<> EnumerateDevicesEx(const DeviceEnumerationArgs& args);


    // 
    void AddFactory(DeviceFactory* factory)
    {
        // This lock is only needed if we call AddFactory after manager thread creation.
        Lock::Locker scopeLock(GetLock());
        Factories.PushBack(factory);
        factory->AddedToManager(this);        
    }

    void CallOnDeviceAdded(DeviceCreateDesc* desc)
    {
        HandlerRef.Call(MessageDeviceStatus(Message_DeviceAdded, this, DeviceHandle(desc)));
    }
    void CallOnDeviceRemoved(DeviceCreateDesc* desc)
    {
        HandlerRef.Call(MessageDeviceStatus(Message_DeviceRemoved, this, DeviceHandle(desc)));
    }

    // Helper to access Common data for a device.
    static DeviceCommon* GetDeviceCommon(DeviceBase* device)
    {
        return device->getDeviceCommon();
    }


    // Background-thread callbacks for DeviceCreation/Release. These
    DeviceBase* CreateDevice_MgrThread(DeviceCreateDesc* createDesc, DeviceBase* parent = 0);
    Void        ReleaseDevice_MgrThread(DeviceBase* device);

   
    // Calls EnumerateDevices() on all factories
    virtual Void EnumerateAllFactoryDevices();
    // Enumerates devices for a particular factory.
    virtual Void EnumerateFactoryDevices(DeviceFactory* factory);

    virtual HIDDeviceManager* GetHIDDeviceManager() const
    {
        return HidDeviceManager;
    }

    // Adds device (DeviceCreateDesc*) into Devices. Returns NULL, 
    // if unsuccessful or device is already in the list.
    virtual Ptr<DeviceCreateDesc> AddDevice_NeedsLock(const DeviceCreateDesc& createDesc);
    
    // Finds a device descriptor by path and optional type.
    Ptr<DeviceCreateDesc> FindDevice(const String& path, DeviceType = Device_None);

    // Finds HID device by HIDDeviceDesc.
    Ptr<DeviceCreateDesc> FindHIDDevice(const HIDDeviceDesc&);
    void DetectHIDDevice(const HIDDeviceDesc&);

    // Manager Lock-protected list of devices.
    List<DeviceCreateDesc>  Devices;    

    // Factories used to detect and manage devices.
    List<DeviceFactory>     Factories;

protected:
    Ptr<HIDDeviceManager>   HidDeviceManager;
    Ptr<ProfileManager>     pProfileManager;
};


} // namespace OVR

#endif
