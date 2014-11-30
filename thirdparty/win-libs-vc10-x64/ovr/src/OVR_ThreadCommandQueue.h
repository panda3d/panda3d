/************************************************************************************

PublicHeader:   None
Filename    :   OVR_ThreadCommandQueue.h
Content     :   Command queue for operations executed on a thread
Created     :   October 29, 2012
Author      :   Michael Antonov

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

************************************************************************************/

#ifndef OVR_ThreadCommandQueue_h
#define OVR_ThreadCommandQueue_h

#include "Kernel/OVR_Types.h"
#include "Kernel/OVR_List.h"
#include "Kernel/OVR_Atomic.h"
#include "Kernel/OVR_Threads.h"

namespace OVR {

class ThreadCommand;
class ThreadCommandQueue;


//-------------------------------------------------------------------------------------
// ***** ThreadCommand

// ThreadCommand is a base class implementation for commands stored in ThreadCommandQueue.
class ThreadCommand
{
public:    

    // NotifyEvent is used by ThreadCommandQueue::PushCallAndWait to notify the
    // calling (producer)  thread when command is completed or queue slot is available.
    class NotifyEvent : public ListNode<NotifyEvent>, public NewOverrideBase
    {
        Event E;
    public:   
        NotifyEvent() { }

        void Wait()        { E.Wait(); }
        void PulseEvent()  { E.PulseEvent(); }
    };

    // ThreadCommand::PopBuffer is temporary storage for a command popped off
    // by ThreadCommandQueue::PopCommand. 
    class PopBuffer
    {
        enum { MaxSize = 256 };

        UPInt Size;
        union {            
            UByte Buffer[MaxSize];
            UPInt Align;
        };

        ThreadCommand* toCommand() const { return (ThreadCommand*)Buffer; }

    public:
        PopBuffer() : Size(0) { }
        ~PopBuffer();

        void        InitFromBuffer(void* data);

        bool        HasCommand() const  { return Size != 0; }
        UPInt       GetSize() const     { return Size; }
        bool        NeedsWait() const   { return toCommand()->NeedsWait(); }
        NotifyEvent* GetEvent() const   { return toCommand()->pEvent; }

        // Execute the command and also notifies caller to finish waiting,
        // if necessary.
        void        Execute();
    };
    
    UInt16       Size;
    bool         WaitFlag; 
    bool         ExitFlag; // Marks the last exit command. 
    NotifyEvent* pEvent;

    ThreadCommand(UPInt size, bool waitFlag, bool exitFlag = false)
        : Size((UInt16)size), WaitFlag(waitFlag), ExitFlag(exitFlag), pEvent(0) { }
    virtual ~ThreadCommand() { }

    bool          NeedsWait() const { return WaitFlag; }
    UPInt         GetSize() const   { return Size; }

    virtual void            Execute() const = 0;
    // Copy constructor used for serializing this to memory buffer.
    virtual ThreadCommand*  CopyConstruct(void* p) const = 0;
};


//-------------------------------------------------------------------------------------

// CleanType is a template that strips 'const' and '&' modifiers from the argument type;
// for example, typename CleanType<A&>::Type is equivalent to A.
template<class T> struct CleanType           { typedef T Type; };
template<class T> struct CleanType<T&>       { typedef T Type; };
template<class T> struct CleanType<const T>  { typedef T Type; };
template<class T> struct CleanType<const T&> { typedef T Type; };

// SelfType is a template that yields the argument type. This helps avoid conflicts with
// automatic template argument deduction for function calls when identical argument
// is already defined.
template<class T> struct SelfType { typedef T Type; };



//-------------------------------------------------------------------------------------
// ThreadCommand specializations for member functions with different number of
// arguments and argument types.

// Used to return nothing from a ThreadCommand, to avoid problems with 'void'.
struct Void
{
    Void() {}
    Void(int) {}
};

// ThreadCommand for member function with 0 arguments.
template<class C, class R>
class ThreadCommandMF0 : public ThreadCommand
{   
    typedef R (C::*FnPtr)();
    C*      pClass;
    FnPtr   pFn;
    R*      pRet;

    void executeImpl() const
    {
        pRet ? (void)(*pRet = (pClass->*pFn)()) :
	           (void)(pClass->*pFn)();
    }

public:    
    ThreadCommandMF0(C* pclass, FnPtr fn, R* ret, bool needsWait)
        : ThreadCommand(sizeof(ThreadCommandMF0), needsWait),
          pClass(pclass), pFn(fn), pRet(ret) { }

    virtual void           Execute() const { executeImpl(); }
    virtual ThreadCommand* CopyConstruct(void* p) const
    { return Construct<ThreadCommandMF0>(p, *this); }
};


// ThreadCommand for member function with 1 argument.
template<class C, class R, class A0>
class ThreadCommandMF1 : public ThreadCommand
{   
    typedef R (C::*FnPtr)(A0);
    C*                           pClass;
    FnPtr                        pFn;
    R*                           pRet;
    typename CleanType<A0>::Type AVal0;

    void executeImpl() const
    {
      pRet ? (void)(*pRet = (pClass->*pFn)(AVal0)) :
	         (void)(pClass->*pFn)(AVal0);
    }

public:    
    ThreadCommandMF1(C* pclass, FnPtr fn, R* ret, A0 a0, bool needsWait)
        : ThreadCommand(sizeof(ThreadCommandMF1), needsWait),
          pClass(pclass), pFn(fn), pRet(ret), AVal0(a0) { }

    virtual void           Execute() const { executeImpl(); }
    virtual ThreadCommand* CopyConstruct(void* p) const
    { return Construct<ThreadCommandMF1>(p, *this); }
};

// ThreadCommand for member function with 2 arguments.
template<class C, class R, class A0, class A1>
class ThreadCommandMF2 : public ThreadCommand
{   
    typedef R (C::*FnPtr)(A0, A1);
    C*                            pClass;
    FnPtr                         pFn;
    R*                            pRet;
    typename CleanType<A0>::Type  AVal0;
    typename CleanType<A1>::Type  AVal1;

    void executeImpl() const
    {
        pRet ? (void)(*pRet = (pClass->*pFn)(AVal0, AVal1)) :
	           (void)(pClass->*pFn)(AVal0, AVal1);
    }

public:    
    ThreadCommandMF2(C* pclass, FnPtr fn, R* ret, A0 a0, A1 a1, bool needsWait)
        : ThreadCommand(sizeof(ThreadCommandMF2), needsWait),
          pClass(pclass), pFn(fn), pRet(ret), AVal0(a0), AVal1(a1) { }
    
    virtual void           Execute() const { executeImpl(); }
    virtual ThreadCommand* CopyConstruct(void* p) const 
    { return Construct<ThreadCommandMF2>(p, *this); }
};


//-------------------------------------------------------------------------------------
// ***** ThreadCommandQueue

// ThreadCommandQueue is a queue of executable function-call commands intended to be
// serviced by a single consumer thread. Commands are added to the queue with PushCall
// and removed with PopCall; they are processed in FIFO order. Multiple producer threads
// are supported and will be blocked if internal data buffer is full.

class ThreadCommandQueue
{
public:

    ThreadCommandQueue();
    virtual ~ThreadCommandQueue();


    // Pops the next command from the thread queue, if any is available.
    // The command should be executed by calling popBuffer->Execute().
    // Returns 'false' if no command is available at the time of the call.
    bool PopCommand(ThreadCommand::PopBuffer* popBuffer);

    // Generic implementaion of PushCommand; enqueues a command for execution.
    // Returns 'false' if push failed, usually indicating thread shutdown.
    bool PushCommand(const ThreadCommand& command);

    // 
    void PushExitCommand(bool wait);

    // Returns 'true' once ExitCommand has been processed, so the thread can shut down.
    bool IsExiting() const;


    // These two virtual functions serve as notifications for derived
    // thread waiting.    
    virtual void OnPushNonEmpty_Locked() { }
    virtual void OnPopEmpty_Locked()     { }


    // *** PushCall with no result
    
    // Enqueue a member function of 'this' class to be called on consumer thread.
    // By default the function returns immediately; set 'wait' argument to 'true' to
    // wait for completion.
    template<class C, class R>
    bool PushCall(R (C::*fn)(), bool wait = false)
    { return PushCommand(ThreadCommandMF0<C,R>(static_cast<C*>(this), fn, 0, wait)); }       
    template<class C, class R, class A0>
    bool PushCall(R (C::*fn)(A0), typename SelfType<A0>::Type a0, bool wait = false)
    { return PushCommand(ThreadCommandMF1<C,R,A0>(static_cast<C*>(this), fn, 0, a0, wait)); }
    template<class C, class R, class A0, class A1>
    bool PushCall(R (C::*fn)(A0, A1),
                  typename SelfType<A0>::Type a0, typename SelfType<A1>::Type a1, bool wait = false)
    { return PushCommand(ThreadCommandMF2<C,R,A0,A1>(static_cast<C*>(this), fn, 0, a0, a1, wait)); }
    // Enqueue a specified member function call of class C.
    // By default the function returns immediately; set 'wait' argument to 'true' to
    // wait for completion.
    template<class C, class R>
    bool PushCall(C* p, R (C::*fn)(), bool wait = false)
    { return PushCommand(ThreadCommandMF0<C,R>(p, fn, 0, wait)); }
    template<class C, class R, class A0>
    bool PushCall(C* p, R (C::*fn)(A0), typename SelfType<A0>::Type a0, bool wait = false)
    { return PushCommand(ThreadCommandMF1<C,R,A0>(p, fn, 0, a0, wait)); }
    template<class C, class R, class A0, class A1>
    bool PushCall(C* p, R (C::*fn)(A0, A1),
                  typename SelfType<A0>::Type a0, typename SelfType<A1>::Type a1, bool wait = false)
    { return PushCommand(ThreadCommandMF2<C,R,A0,A1>(p, fn, 0, a0, a1, wait)); }
    
    
    // *** PushCall with Result

    // Enqueue a member function of 'this' class call and wait for call to complete
    // on consumer thread before returning.
    template<class C, class R>
    bool PushCallAndWaitResult(R (C::*fn)(), R* ret)
    { return PushCommand(ThreadCommandMF0<C,R>(static_cast<C*>(this), fn, ret, true)); }       
    template<class C, class R, class A0>
    bool PushCallAndWaitResult(R (C::*fn)(A0), R* ret, typename SelfType<A0>::Type a0)
    { return PushCommand(ThreadCommandMF1<C,R,A0>(static_cast<C*>(this), fn, ret, a0, true)); }
    template<class C, class R, class A0, class A1>
    bool PushCallAndWaitResult(R (C::*fn)(A0, A1), R* ret,
                               typename SelfType<A0>::Type a0, typename SelfType<A1>::Type a1)
    { return PushCommand(ThreadCommandMF2<C,R,A0,A1>(static_cast<C*>(this), fn, ret, a0, a1, true)); }
    // Enqueue a member function call for class C and wait for the call to complete
    // on consumer thread before returning.
    template<class C, class R>
    bool PushCallAndWaitResult(C* p, R (C::*fn)(), R* ret)
    { return PushCommand(ThreadCommandMF0<C,R>(p, fn, ret, true)); }
    template<class C, class R, class A0>
    bool PushCallAndWaitResult(C* p, R (C::*fn)(A0), R* ret, typename SelfType<A0>::Type a0)
    { return PushCommand(ThreadCommandMF1<C,R,A0>(p, fn, ret, a0, true)); }
    template<class C, class R, class A0, class A1>
    bool PushCallAndWaitResult(C* p, R (C::*fn)(A0, A1), R* ret,
                               typename SelfType<A0>::Type a0, typename SelfType<A1>::Type a1)
    { return PushCommand(ThreadCommandMF2<C,R,A0,A1>(p, fn, ret, a0, a1, true)); }

private:
    class ThreadCommandQueueImpl* pImpl;
};


}

#endif // OVR_ThreadCommandQueue_h
