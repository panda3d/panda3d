// Filename: ipc_thread.h
// Created by:  cary (16Sep98)
//
////////////////////////////////////////////////////////////////////

#ifndef __IPC_THREAD_H__
#define __IPC_THREAD_H__

#include <pandabase.h>
#include <stdio.h>

#include "ipc_traits.h"
#include "ipc_mutex.h"

class EXPCL_PANDAEXPRESS base_thread {
   public:
      typedef ipc_traits traits;
      typedef traits::thread_class thread_class;
      typedef base_mutex mutex;
      typedef base_mutex_lock mutex_lock;

      // expand this to allow real priorities and scheduling.  determine the
      // priority limits based on the scheduling.  If scheduling is not
      // allowed, silently ignore those requests.
      enum priority_t {
	 PRIORITY_LOW,
	 PRIORITY_NORMAL,
	 PRIORITY_HIGH
      };

      enum state_t {
	 STATE_NEW,       // thread exists, but hasn't started yet.
	 STATE_RUNNING,   // thread is running
	 STATE_TERMINATED // thread is done, but storage hasn't been reclaimed
      };

      static base_thread* const Null;
      // Constructors set up the thread object but the thread won't start until
      // start() is called.  The create(...) functions can be used to construct
      // and start a thread in a single call.
      //
      // should be able to get rid of the voids here with clever templating.
      INLINE base_thread(void (*fn)(void*), void* arg = (void*)0L,
			 const priority_t pri = PRIORITY_NORMAL) :
	_thread(thread_class::Null), _fn_void(fn), _fn_ret(0L), _mutex(),
	_state(STATE_NEW), _priority(pri), _thread_arg(arg), _detached(true) {
	 common_constructor();
      }
      INLINE base_thread(void* (*fn)(void*), void* arg = (void*)0L,
			 const priority_t pri = PRIORITY_NORMAL) :
	_thread(thread_class::Null), _fn_void(0L), _fn_ret(fn), _mutex(),
        _state(STATE_NEW), _priority(pri), _thread_arg(arg), _detached(false) {
	 common_constructor();
      }
      // causes this thread to start executing
      INLINE void start(void) {
	 mutex_lock l(_mutex);
	 if (_state != STATE_NEW) {
	    cerr << "base_thread::start throwing thread_invalid()" << endl;
#ifdef BROKEN_EXCEPTIONS
	    return;
#else
	    throw thread_invalid();
#endif /* BROKEN_EXCEPTIONS */
	 }
	 _thread = traits::make_thread(this);
	 _thread->start_pre(thread_wrapper, _detached, _priority);
	 _state = STATE_RUNNING;
	 _thread->start_post(_detached, _priority);
      }
      // causes the calling thread to wait for this thread's completion,
      // putting the return value in the space passed by the parameter.  Only
      // undetached threads may be joined.  Storage for the thread will be
      // reclaimed when it exits.
      // Again, clever templating should be able to eliminate this void
      // nonsense.  It's gross!
      INLINE void join(void** status) {
	 _mutex.lock();
	 if ((_state != STATE_RUNNING) && (_state != STATE_TERMINATED)) {
	    _mutex.unlock();
#ifdef BROKEN_EXCEPTIONS
	    return;
#else
	    throw thread_invalid();
#endif /* BROKEN_EXCEPTIONS */
	 }
	 _mutex.unlock();
	 if (this == self())
#ifdef BROKEN_EXCEPTIONS
	    return;
#else
	    throw thread_invalid();
#endif /* BROKEN_EXCEPTIONS */
	 if (_detached)
#ifdef BROKEN_EXCEPTIONS
	    return;
#else
	    throw thread_invalid();
#endif /* BROKEN_EXCEPTIONS */
	 _thread->join(status);
	 delete this;
      }
      // set the priority of this thread
      INLINE void set_priority(const priority_t pri) {
	 mutex_lock l(_mutex);
	 if (_state != STATE_RUNNING)
#ifdef BROKEN_EXCEPTIONS
	    return;
#else
	    throw thread_invalid();
#endif /* BROKEN_EXCEPTIONS */
	 _thread->set_priority(pri);
	 _priority = pri;
      }
      // return this thread's priority
      INLINE priority_t get_priority(void) {
	 mutex_lock l(_mutex);
	 // in a more general priority system, we should check that the thread
	 // is running at the priority we think it is.
	 return _priority;
      }
      // return this thread's state
      INLINE state_t get_state(void) {
	 mutex_lock l(_mutex);
	 return _state;
      }
      // return this thread's id within the current process
      INLINE int get_id(void) {
	 return _id;  // presumably this doesn't change
      }
      // causes the calling thread to exit, again the void action here is
      // unhappy.
      static INLINE void exit(void* return_value = (void*)0L) {
	cout.flush();
	 base_thread* me = self();
	 if (me != (base_thread*)0L) {
	    me->_mutex.lock();
	    // if (me->_state != STATE_RUNNING)  non-fatal error
	    me->_state = STATE_TERMINATED;
	    me->_mutex.unlock();
	    if (me->_detached)
	       delete me;
	 }
	 thread_class::exit(return_value);
      }
      // returns the caller's thread object.  If the calling thread is not the
      // main thread or created by us, thread_class::Null is returned
      static INLINE base_thread* self(void) {
	cout.flush();
	 thread_class* tclass=thread_class::self();
	 base_thread* ret=(base_thread*)0L;
	 if (tclass != thread_class::Null)
	    ret = (base_thread*)(tclass->back_ptr());
	 return ret;
      }
      // calling task releasing control to the scheduler
      static INLINE void yield(void) {
	 thread_class::yield();
      }
      // sleep for the specified amount of time
      static INLINE void sleep(const unsigned long secs,
			       const unsigned long nsecs = 0) {
	 traits::sleep(secs, nsecs);
      }
      // calculates an absolute time in seconds and nanoseconds, suitable for
      // use in timed_waits on condition variables, which is the current time
      // plus the given relative offset.
      static INLINE void get_time(unsigned long& abs_secs,
				  unsigned long& abs_nsecs,
				  const unsigned long rel_secs = 0,
				  const unsigned long rel_nsecs = 0) {
	 traits::get_time(abs_secs, abs_nsecs, rel_secs, rel_nsecs);
      }
      // shortcut function to create and start a thread in one call
      static INLINE base_thread* create(void (*fn)(void*),
					void* arg = (void*)0L,
					const priority_t pri=PRIORITY_NORMAL) {
	 base_thread* t = new base_thread(fn, arg, pri);
	 t->start();
	 return t;
      }
      static INLINE base_thread* create(void* (*fn)(void*),
					void* arg = (void*)0L,
					const priority_t pri=PRIORITY_NORMAL) {
	 base_thread* t = new base_thread(fn, arg, pri);
	 t->start();
	 return t;
      }
      // a handle to the implementation specific thread, this really should
      // NEVER be called from user code.
      INLINE thread_class* get_thread(void) { return _thread; }
   protected:
      // this constructor is used in a derived class.  The thread will execute
      // the run() or run_undetached() member functions depending on whether
      // start() or start_undetached() is called, respectively.
      INLINE base_thread(void* arg = (void*)0L,
			 const priority_t pri = PRIORITY_NORMAL) :
        _thread(thread_class::Null), _fn_void(0L), _fn_ret(0L), _mutex(),
        _state(STATE_NEW), _priority(pri), _thread_arg(arg), _detached(true) {
	 common_constructor();
      }
      // can be used with the above constructor in a derived class to cause the
      // thread to be undetached.  In this case the thread executes the
      // run_undetached member function.
      INLINE void start_undetached(void) {
	 if ((_fn_void != NULL) || (_fn_ret != NULL))
#ifdef BROKEN_EXCEPTIONS
	    return;
#else
	    throw thread_invalid();
#endif /* BROKEN_EXCEPTIONS */
	 _detached = false;
	 start();
      }
      // destructor cannot be called by user (except via a derived class).
      // Use exit() instead.  This also means a thread object must be allocated
      // with new, it cannot be statically or automatically allocated.  The
      // destructor of a class that inherits from thread<...> shouldn't be
      // public either (otherwise the thread object can be destroyed while the
      // underlying thread is still running (this is Bad(tm))).
   virtual ~base_thread(void) { 
     delete _thread; 
   }

   private:
      thread_class *_thread;  // the implementation specific thread
      void (*_fn_void)(void*);
      void* (*_fn_ret)(void*);
      mutex _mutex;  // used to protect members which can change after
                     // construction.  ie: _state and _priority
      state_t _state;
      priority_t _priority;
      int _id;
      void* _thread_arg;
      bool _detached;
      static mutex* _next_id_mutex;
      static int _next_id;

      // can be overridden in a derived class.  When constructed using the
      // constructor thread(void*, priority_t), these functions are called by
      // start() and start_undetached() respectively.
      virtual void run(void*);
      virtual void* run_undetached(void*);
      // implements the common parts of the constructors
      INLINE void common_constructor(void) {
	 if (_next_id_mutex == (mutex *)0L) {
	    // ok, we're the first in here.
	    _next_id_mutex = new mutex;
	    base_thread* t = new base_thread;
	    if (t->_state != STATE_NEW) {
	       // FATAL() << "thread initialization: problem creating initial thread object" << nend;
	       ::exit(1);
	    }
	    t->_state = STATE_RUNNING;
	    t->_thread = traits::make_thread(t);
	    t->_thread->manual_init();
	    t->_thread->start_in();
	 }
	 
	 mutex_lock l(*_next_id_mutex);
	 _id = _next_id++;
      }
      static void *thread_wrapper(void*);
};

typedef base_thread thread;

#endif /* __IPC_THREAD_H__ */
