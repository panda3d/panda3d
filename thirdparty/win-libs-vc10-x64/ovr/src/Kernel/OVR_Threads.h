/************************************************************************************

PublicHeader:   None
Filename    :   OVR_Threads.h
Content     :   Contains thread-related (safe) functionality
Created     :   September 19, 2012
Notes       : 

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

************************************************************************************/
#ifndef OVR_Threads_h
#define OVR_Threads_h

#include "OVR_Types.h"
#include "OVR_Atomic.h"
#include "OVR_RefCount.h"
#include "OVR_Array.h"

// Defines the infinite wait delay timeout
#define OVR_WAIT_INFINITE 0xFFFFFFFF

// To be defined in the project configuration options
#ifdef OVR_ENABLE_THREADS


namespace OVR {

//-----------------------------------------------------------------------------------
// ****** Declared classes

// Declared with thread support only
class   Mutex;
class   WaitCondition;
class   Event;
// Implementation forward declarations
class MutexImpl;
class WaitConditionImpl;



//-----------------------------------------------------------------------------------
// ***** Mutex

// Mutex class represents a system Mutex synchronization object that provides access 
// serialization between different threads, allowing one thread mutually exclusive access 
// to a resource. Mutex is more heavy-weight then Lock, but supports WaitCondition.

class Mutex
{
    friend class WaitConditionImpl;    
    friend class MutexImpl;

    MutexImpl  *pImpl; 

public:
    // Constructor/destructor
    Mutex(bool recursive = 1);
    ~Mutex();

    // Locking functions
    void  DoLock();
    bool  TryLock();
    void  Unlock();

    // Returns 1 if the mutes is currently locked by another thread
    // Returns 0 if the mutex is not locked by another thread, and can therefore be acquired. 
    bool  IsLockedByAnotherThread();
    
    // Locker class; Used for automatic locking of a mutex withing scope    
    class Locker
    {
    public:
        Mutex *pMutex;
        Locker(Mutex *pmutex)
            { pMutex = pmutex; pMutex->DoLock(); }
        ~Locker()
            { pMutex->Unlock(); }
    };
};


//-----------------------------------------------------------------------------------
// ***** WaitCondition

/*
    WaitCondition is a synchronization primitive that can be used to implement what is known as a monitor.
    Dependent threads wait on a wait condition by calling Wait(), and get woken up by other threads that
    call Notify() or NotifyAll().

    The unique feature of this class is that it provides an atomic way of first releasing a Mutex, and then 
    starting a wait on a wait condition. If both the mutex and the wait condition are associated with the same
    resource, this ensures that any condition checked for while the mutex was locked does not change before
    the wait on the condition is actually initiated.
*/

class WaitCondition
{
    friend class WaitConditionImpl;
    // Internal implementation structure
    WaitConditionImpl *pImpl;

public:
    // Constructor/destructor
    WaitCondition();
    ~WaitCondition();

    // Release mutex and wait for condition. The mutex is re-aquired after the wait.
    // Delay is specified in milliseconds (1/1000 of a second).
    bool    Wait(Mutex *pmutex, unsigned delay = OVR_WAIT_INFINITE);

    // Notify a condition, releasing at one object waiting
    void    Notify();
    // Notify a condition, releasing all objects waiting
    void    NotifyAll();
};


//-----------------------------------------------------------------------------------
// ***** Event

// Event is a wait-able synchronization object similar to Windows event.
// Event can be waited on until it's signaled by another thread calling
// either SetEvent or PulseEvent.

class Event
{
    // Event state, its mutex and the wait condition
    volatile bool   State;
    volatile bool   Temporary;  
    mutable Mutex   StateMutex;
    WaitCondition   StateWaitCondition;

    void updateState(bool newState, bool newTemp, bool mustNotify);

public:    
    Event(bool setInitially = 0) : State(setInitially), Temporary(false) { }
    ~Event() { }

    // Wait on an event condition until it is set
    // Delay is specified in milliseconds (1/1000 of a second).
    bool  Wait(unsigned delay = OVR_WAIT_INFINITE);
    
    // Set an event, releasing objects waiting on it
    void  SetEvent()
    { updateState(true, false, true); }

    // Reset an event, un-signaling it
    void  ResetEvent()
    { updateState(false, false, false); }

    // Set and then reset an event once a waiter is released.
    // If threads are already waiting, they will be notified and released
    // If threads are not waiting, the event is set until the first thread comes in
    void  PulseEvent()
    { updateState(true, true, true); }
};


//-----------------------------------------------------------------------------------
// ***** Thread class

// ThreadId uniquely identifies a thread; returned by GetCurrentThreadId() and
// Thread::GetThreadId.
typedef void* ThreadId;


// *** Thread flags

// Indicates that the thread is has been started, i.e. Start method has been called, and threads
// OnExit() method has not yet been called/returned.
#define OVR_THREAD_STARTED               0x01
// This flag is set once the thread has ran, and finished.
#define OVR_THREAD_FINISHED              0x02
// This flag is set temporarily if this thread was started suspended. It is used internally.
#define OVR_THREAD_START_SUSPENDED       0x08
// This flag is used to ask a thread to exit. Message driven threads will usually check this flag
// and finish once it is set.
#define OVR_THREAD_EXIT                  0x10


class Thread : public RefCountBase<Thread>
{ // NOTE: Waitable must be the first base since it implements RefCountImpl.    

public:

    // *** Callback functions, can be used instead of overriding Run

    // Run function prototypes.    
    // Thread function and user handle passed to it, executed by the default
    // Thread::Run implementation if not null.
    typedef int (*ThreadFn)(Thread *pthread, void* h);
    
    // Thread ThreadFunction1 is executed if not 0, otherwise ThreadFunction2 is tried
    ThreadFn    ThreadFunction;    
    // User handle passes to a thread
    void*       UserHandle;

    // Thread state to start a thread with
    enum ThreadState
    {
        NotRunning  = 0,
        Running     = 1,
        Suspended   = 2
    };

    // Thread priority
    enum ThreadPriority
    {
        CriticalPriority,
        HighestPriority,
        AboveNormalPriority,
        NormalPriority,
        BelowNormalPriority,
        LowestPriority,
        IdlePriority,
    };

    // Thread constructor parameters
    struct CreateParams
    {
        CreateParams(ThreadFn func = 0, void* hand = 0, UPInt ssize = 128 * 1024, 
                     int proc = -1, ThreadState state = NotRunning, ThreadPriority prior = NormalPriority)
                     : threadFunction(func), userHandle(hand), stackSize(ssize), 
                       processor(proc), initialState(state), priority(prior) {}
        ThreadFn       threadFunction;   // Thread function
        void*          userHandle;       // User handle passes to a thread
        UPInt          stackSize;        // Thread stack size
        int            processor;        // Thread hardware processor
        ThreadState    initialState;     // 
        ThreadPriority priority;         // Thread priority
    };

    // *** Constructors

    // A default constructor always creates a thread in NotRunning state, because
    // the derived class has not yet been initialized. The derived class can call Start explicitly.
    // "processor" parameter specifies which hardware processor this thread will be run on. 
    // -1 means OS decides this. Implemented only on Win32
    Thread(UPInt stackSize = 128 * 1024, int processor = -1);
    // Constructors that initialize the thread with a pointer to function.
    // An option to start a thread is available, but it should not be used if classes are derived from Thread.
    // "processor" parameter specifies which hardware processor this thread will be run on. 
    // -1 means OS decides this. Implemented only on Win32
    Thread(ThreadFn threadFunction, void*  userHandle = 0, UPInt stackSize = 128 * 1024,
           int processor = -1, ThreadState initialState = NotRunning);
    // Constructors that initialize the thread with a create parameters structure.
    explicit Thread(const CreateParams& params);

    // Destructor.
    virtual ~Thread();

    // Waits for all Threads to finish; should be called only from the root
    // application thread. Once this function returns, we know that all other
    // thread's references to Thread object have been released.
    static  void OVR_CDECL FinishAllThreads();


    // *** Overridable Run function for thread processing

    // - returning from this method will end the execution of the thread
    // - return value is usually 0 for success 
    virtual int   Run();
    // Called after return/exit function
    virtual void  OnExit();


    // *** Thread management

    // Starts the thread if its not already running
    // - internally sets up the threading and calls Run()
    // - initial state can either be Running or Suspended, NotRunning will just fail and do nothing
    // - returns the exit code
    virtual bool  Start(ThreadState initialState = Running);

    // Quits with an exit code
    virtual void  Exit(int exitCode=0);

    // Suspend the thread until resumed
    // Returns 1 for success, 0 for failure.
    bool  Suspend();
    // Resumes currently suspended thread
    // Returns 1 for success, 0 for failure.
    bool  Resume();

    // Static function to return a pointer to the current thread
    //static Thread* GetThread();


    // *** Thread status query functions

    bool          GetExitFlag() const;
    void          SetExitFlag(bool exitFlag);

    // Determines whether the thread was running and is now finished
    bool          IsFinished() const;
    // Determines if the thread is currently suspended
    bool          IsSuspended() const;
    // Returns current thread state
    ThreadState   GetThreadState() const;

    // Returns the number of available CPUs on the system 
    static int    GetCPUCount();

    // Returns the thread exit code. Exit code is initialized to 0,
    // and set to the return value if Run function after the thread is finished.
    inline int    GetExitCode() const { return ExitCode; }
    // Returns an OS handle 
#if defined(OVR_OS_WIN32)
    void*          GetOSHandle() const { return ThreadHandle; }
#else
    pthread_t      GetOSHandle() const { return ThreadHandle; }
#endif

#if defined(OVR_OS_WIN32)
    ThreadId       GetThreadId() const { return IdValue; }
#else
    ThreadId       GetThreadId() const { return (ThreadId)GetOSHandle(); }
#endif

    static int      GetOSPriority(ThreadPriority);
    // *** Sleep

    // Sleep secs seconds
    static bool    Sleep(unsigned secs);
    // Sleep msecs milliseconds
    static bool    MSleep(unsigned msecs);


    // *** Debugging functionality
#if defined(OVR_OS_WIN32)
    virtual void    SetThreadName( const char* name );
#else
    virtual void    SetThreadName( const char* name ) { OVR_UNUSED(name); }
#endif

private:
#if defined(OVR_OS_WIN32)
    friend unsigned WINAPI Thread_Win32StartFn(void *pthread);

#else
    friend void *Thread_PthreadStartFn(void * phandle);

    static int            InitAttr;
    static pthread_attr_t Attr;
#endif

protected:    
    // Thread state flags
    AtomicInt<UInt32>   ThreadFlags;
    AtomicInt<SInt32>   SuspendCount;
    UPInt               StackSize;

    // Hardware processor which this thread is running on.
    int            Processor;
    ThreadPriority Priority;

#if defined(OVR_OS_WIN32)
    void*               ThreadHandle;
    volatile ThreadId   IdValue;

    // System-specific cleanup function called from destructor
    void                CleanupSystemThread();

#else
    pthread_t           ThreadHandle;
#endif

    // Exit code of the thread, as returned by Run.
    int                 ExitCode;

    // Internal run function.
    int                 PRun();    
    // Finishes the thread and releases internal reference to it.
    void                FinishAndRelease();

    void                Init(const CreateParams& params);

    // Protected copy constructor
    Thread(const Thread &source) { OVR_UNUSED(source); }

};

// Returns the unique Id of a thread it is called on, intended for
// comparison purposes.
ThreadId GetCurrentThreadId();


} // OVR

#endif // OVR_ENABLE_THREADS
#endif // OVR_Threads_h
