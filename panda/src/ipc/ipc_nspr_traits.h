// Filename: ipc_nspr_traits.h
// Created by:  cary (12Jan00)
//
////////////////////////////////////////////////////////////////////

#ifndef __IPC_NSPR_TRAITS_H__
#define __IPC_NSPR_TRAITS_H__

#include <pandabase.h>
#include <nspr.h>

#include "datagram.h"

class EXPCL_PANDAEXPRESS ipc_traits {
public:
  class mutex_class;
  class condition_class;
  class semaphore_class;
  class thread_class;
  class library_class;

  class EXPCL_PANDAEXPRESS mutex_class {
  private:
    PRLock* _mutex;
  public:
    // interface to mutex
    static mutex_class* const Null;
    INLINE mutex_class(void) : _mutex(PR_NewLock()) {
      if (_mutex == (PRLock*)0L)
	cerr << "COULD NOT GET A MUTEX!" << endl;
    }
    INLINE ~mutex_class(void) {
      PR_DestroyLock(_mutex);
    }
    INLINE void lock(void) {
      PR_Lock(_mutex);
    }
    INLINE void unlock(void) {
      PR_Unlock(_mutex);
    }
    INLINE PRLock* get_mutex(void) {
      return _mutex;
    }
  };
  class EXPCL_PANDAEXPRESS condition_class {
  private:
    mutex_class* _mutex;
    PRCondVar* _condition;
  public:
    static condition_class* const Null;
    INLINE condition_class(mutex_class* m)
      : _mutex(m), _condition(PR_NewCondVar(m->get_mutex())) {}
    INLINE ~condition_class(void) {
      PR_DestroyCondVar(_condition);
    }
    INLINE void wait(void) {
      PR_WaitCondVar(_condition, PR_INTERVAL_NO_TIMEOUT);
    }
    INLINE int timedwait(const unsigned long secs, const unsigned long nsecs) {
      unsigned long s, n;
      ipc_traits::get_time(s, n);
      PRIntervalTime i = PR_SecondsToInterval(secs - s) +
	PR_MicrosecondsToInterval((nsecs - n) / 1000);
      PR_WaitCondVar(_condition, i);
      return 1;
    }
    INLINE void signal(void) {
      PR_NotifyCondVar(_condition);
    }
    INLINE void broadcast(void) {
      PR_NotifyAllCondVar(_condition);
    }
  };
  class EXPCL_PANDAEXPRESS semaphore_class {
  private:
    mutex_class _mutex;
    condition_class _condition;
    int _value;
    class EXPCL_PANDAEXPRESS mutex_class_lock {
    public:
      INLINE mutex_class_lock(mutex_class& m) : _mutex(m) {
	_mutex.lock();
      }
      INLINE ~mutex_class_lock(void) {
	_mutex.unlock();
      }
    private:
      mutex_class& _mutex;
    };
  public:
    static semaphore_class* const Null;
    INLINE semaphore_class(const unsigned int initial)
      : _mutex(), _condition(&_mutex), _value(initial) {}
    INLINE ~semaphore_class(void) {}
    INLINE void wait(void) {
      mutex_class_lock l(_mutex);
      while (_value == 0)
	_condition.wait();
      --_value;
    }
    INLINE int trywait(void) {
      mutex_class_lock l(_mutex);
      if (_value == 0)
	return 0;
      --_value;
      return 1;
    }
    INLINE void post(void) {
      mutex_class_lock l(_mutex);
      if (_value == 0)
	_condition.signal();
      ++_value;
    }
  };
  class EXPCL_PANDAEXPRESS thread_class {
  private:
    PRThread* _thread;
    void* _b_ptr;  // a handle to hang a pointer back to the high-level
                   // thread class on.  Gross, but I couldn't think of a
                   // better way to do this reverse lookup.
    void* (*_fn)(void*);
    void* _return_value;
    INLINE PRThreadPriority priority_map(const int pri) {
      switch (pri) {
      case 0:
	return PR_PRIORITY_LOW;
      case 1:
	return PR_PRIORITY_NORMAL;
      case 2:
	return PR_PRIORITY_HIGH;
      }
      return PR_PRIORITY_NORMAL;
    }
    static void thread_wrapper(void*);
  public:
    static thread_class* const Null;
    INLINE thread_class(void* data) : _b_ptr(data) {}
    INLINE ~thread_class(void) {}
    INLINE void manual_init(void) {
      _thread = PR_GetCurrentThread();
    }
    INLINE void start_pre(void* (*fn)(void*), const bool det, const int pri) {
      _fn = fn;
      // create the thread, decide it's priority, if it's detached, and if it's
      // a kernel thread, etc
      _thread = PR_CreateThread(PR_USER_THREAD, thread_wrapper, this,
				priority_map(pri), PR_GLOBAL_BOUND_THREAD,
				det?PR_UNJOINABLE_THREAD:PR_JOINABLE_THREAD,
				256*1024);
    }
    INLINE void start_in(void) {
      PR_SetThreadPrivate(__get_data_index(), this);
    }
    INLINE void start_post(const bool, const int) {}
    INLINE void join(void** status) {
      PR_JoinThread(_thread);
      if (status)
	*status = _return_value;
    }
    INLINE void set_priority(const int pri) {
      PR_SetThreadPriority(_thread, priority_map(pri));
    }
    INLINE void* back_ptr(void) { return _b_ptr; }
    static INLINE void exit(void* return_value) {
      thread_class* me = self();
      if (me != thread_class::Null)
	me->_return_value = return_value;
      // This gives us a seg fault upon exit - NSPR wants us to return from
      // the callback function rather than exit()
      //exit(0);  // assuming that this is really a kernel thread.  NSPR offers
                // no 'thread exit' facility
    }
    static INLINE thread_class* self(void) {
      return (thread_class*)PR_GetThreadPrivate(__get_data_index());
    }
    static INLINE void yield(void) {
      PR_Sleep(PR_INTERVAL_NO_WAIT);
    }
  };
  class EXPCL_PANDAEXPRESS library_class {
  private:
    PRLibrary* _lib;
  public:
    static library_class* const Null;
    INLINE library_class(const std::string& lib) {
      _lib = PR_LoadLibrary(lib.c_str());
      if (_lib == (PRLibrary*)0L) {
	nout << "failed to load library '" << lib << "'" << endl;
      }
    }
    INLINE ~library_class(void) {
      if (_lib != (PRLibrary*)0L) {
	PRStatus stat = PR_UnloadLibrary(_lib);
	if (stat == PR_FAILURE)
	  nout << "failed to unload library" << endl;
      }
    }
    INLINE void* get_symbol(const std::string& sym) {
      if (_lib == (PRLibrary*)0L) {
	nout << "library not correctly loaded" << endl;
	return (void *)0L;
      }
      return PR_FindSymbol(_lib, sym.c_str());
    }
  };
  class EXPCL_PANDAEXPRESS file_class {
      //interface to file IO
      public:
        INLINE file_class(const std::string& file = "") : _filename(file), _fhandle((PRFileDesc*)NULL){}
        INLINE ~file_class() { close(); }
        INLINE void setFile(const std::string& file) { _filename = file; close(); }
        INLINE bool open(const int mode) {
          close();
	  switch(mode){
	  case 0: _fhandle = PR_Open(_filename.c_str(), PR_RDONLY, 00666); break;
	  case 1: _fhandle = PR_Open(_filename.c_str(), 
				     PR_WRONLY | PR_CREATE_FILE | PR_TRUNCATE, 00666); break;
	  case 2: _fhandle = PR_Open(_filename.c_str(), 
				     PR_RDWR | PR_CREATE_FILE | PR_TRUNCATE, 00666); break;
	  case 3: _fhandle = PR_Open(_filename.c_str(), PR_APPEND | PR_CREATE_FILE, 00666); break;
	  default: _fhandle = PR_Open(_filename.c_str(), PR_RDONLY, 00666); break;
	  }
	  return (_fhandle != (PRFileDesc *)NULL);
	}
	INLINE void close(void) {
	  if (_fhandle != (PRFileDesc *)NULL) { PR_Close(_fhandle); _fhandle = (PRFileDesc *)NULL; }
	}
        INLINE bool empty(void){
	  return (PR_Available(_fhandle) <= 0);
        }
        INLINE int readin(string &block, int numBytes) {
           //Don't try and read less than nothing
           nassertr(numBytes >= 0, 0);
           int numRead;
           char* temp = new char[numBytes];
           if (_fhandle != (PRFileDesc *)NULL){
 	     numRead = PR_Read(_fhandle, temp, numBytes);
             block.assign(temp, numRead);
             delete temp;
             return numRead;
  	   }
           return 0;
  	}
  	INLINE int writeout(const string &block) {
            //Make sure there is something to write out
            nassertr(block.size() > 0, 0);
           if (_fhandle != (PRFileDesc *)NULL) {
  	    return PR_Write(_fhandle, (void*)block.data(), block.size());
  	   }
  	   return 0;
	}
      private:
        string _filename;
	PRFileDesc* _fhandle;
  };

  static INLINE mutex_class* make_mutex(void) {
    return new mutex_class;
  }
  static INLINE condition_class* make_condition(mutex_class* m) {
    return new condition_class(m);
  }
  static INLINE semaphore_class* make_semaphore(const unsigned int initial) {
    return new semaphore_class(initial);
  }
  static INLINE thread_class* make_thread(void* data) {
    return new thread_class(data);
  }
  static INLINE library_class *make_library(const std::string& lib) {
    return new library_class(lib);
  }
  static INLINE file_class *make_file(const std::string& file) {
    return new file_class(file);
  }
  static INLINE void sleep(const unsigned long secs,
			   const unsigned long nsecs = 0) {
    PRIntervalTime i = PR_SecondsToInterval(secs) +
      PR_MicrosecondsToInterval(nsecs / 1000);
    PR_Sleep(i);
  }
  static INLINE void get_time(unsigned long& abs_secs,
			      unsigned long& abs_nsecs,
			      const unsigned long rel_secs = 0,
			      const unsigned long rel_nsecs = 0) {
    PRTime t = PR_Now();
    PRTime t_offset;
    PRTime usec_convert;
    PRTime rel_time;
    PRTime w, x, y;
    PRTime s, n;

    // really this step should only ever be done once
    LL_UI2L(usec_convert, PR_USEC_PER_SEC);

    LL_UI2L(w, rel_secs);
    LL_UI2L(x, (rel_nsecs / 1000));
    LL_MUL(y, usec_convert, w);
    LL_ADD(rel_time, x, y);  // the PRTime representing the rel componant

    LL_ADD(t_offset, t, rel_time);

    LL_DIV(s, t_offset, usec_convert);
    LL_MOD(n, t_offset, usec_convert);

    LL_L2UI(abs_secs, s);
    LL_L2UI(abs_nsecs, n);
    abs_nsecs *= 1000;
  }
  static INLINE int set_atomic(int& var, int val) {
    return PR_AtomicSet(&var, val);
  }
  static INLINE int inc_atomic(int& var) {
    return PR_AtomicIncrement(&var);
  }
  static INLINE int dec_atomic(int& var) {
    return PR_AtomicDecrement(&var);
  }
  // this really should be private, but it's annoying to make config a
  // friend
  static INLINE void __set_data_index(PRUintn d) {
    _data_index = d;
  }
  static INLINE PRUintn __get_data_index(void) {
    return _data_index;
  }
private:
  // other data specific to this implementation
  static PRUintn _data_index;
};

#endif /* __IPC_NSPR_TRAITS_H__ */
