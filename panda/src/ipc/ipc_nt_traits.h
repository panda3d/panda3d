// Filename: ipc_nt_traits.h
// Created by:  cary (16Sep98)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef __IPC_NT_TRAITS_H__
#define __IPC_NT_TRAITS_H__

#include <pandabase.h>

#include <stdlib.h>
#include <errno.h>
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#undef WINDOWS_LEAN_AND_MEAN
#include <process.h>

#ifdef WIN32_VC
const unsigned long SEMAPHORE_MAX = 0x7fffffff;
#endif

// much of the *next and *prev linked list stuff in here should probably be
// done with STL containers.  Examine this when time permits.
class EXPCL_PANDAEXPRESS ipc_traits {
   public:
      class mutex_class;
      class condition_class;
      class semaphore_class;
      class thread_class;
      class library_class;
      class file_class;
      class mutex_class {
         private:
            CRITICAL_SECTION _mutex;
         public:
            // interface to mutex
            static const mutex_class* Null;
            INLINE mutex_class(void) { InitializeCriticalSection(&_mutex); }
            INLINE ~mutex_class(void) { DeleteCriticalSection(&_mutex); }
            INLINE void lock(void) { EnterCriticalSection(&_mutex); }
            INLINE void unlock(void) { LeaveCriticalSection(&_mutex); }
      };
      class thread_class {
         private:
            HANDLE _handle;
            DWORD _id;
            void* _return_value;
            HANDLE _condition;
            thread* _tnext;
            thread* _tprev;
            BOOL _waiting;
            void* _b_ptr; // a handle to hang a pointer back to the high-level
                          // thread class on.  Gross, but I couldn't think of
                          // a better way to do this reverse lookup.
            void* (*_fn)(void*);
            INLINE int priority_map(const int pri) {
              switch (pri) {
              case 0:
                 return THREAD_PRIORITY_LOWEST;
              case 1:
                 return THREAD_PRIORITY_NORMAL;
              case 2:
                 return THREAD_PRIORITY_HIGHEST;
              }
              throw thread_invalid();
              // never gets here, but keeps the compiler happy
              return 0;
            }
#ifndef __BCPLUSPLUS__
            static unsigned _stdcall thread_wrapper(void*);
#else
            static void _USERENTRY thread_wrapper(void*);
#endif
         public:
            // interface to threads
            static const thread_class* Null;
            INLINE thread_class(void* data) : _b_ptr(data),
              _tnext(thread_class::Null), _tprev(thread_class::Null),
              _waiting(FALSE), _handle(NULL) {
               _condition = CreateSemaphore(NULL, 0, SEMAPHORE_MAX, NULL);
               if (_condition == NULL)
                  throw thread_fatal(GetLastError());
            }
            INLINE ~thread_class(void) {
               if (!CloseHandle(_handle))
                  throw thread_fatal(GetLastError());
               if (!CloseHandle(_condition))
                  throw thread_fatal(GetLastError());
            }
            INLINE void manual_init(void) {
               if (!DuplicateHandle(GetCurrentProcess(), GetCurrentThread(),
                                    GetCurrentProcess(), &_handle, 0, FALSE,
                                    DUPLICATE_SAME_ACCESS))
                  throw thread_fatal(GetLastError());
               _id = GetCurrentThreadId();
               if (!SetThreadPriority(_handle, priority_map(PRIORITY_NORMAL)))
                  throw thread_fatal(GetLastError());
            }
            INLINE void start_pre(void* (*fn)(void*), const bool, const int) {
               _fn = fn;
#ifndef __BCPLUSPLUS__
               // MSVC++ or compatable
               unsigned int t;
               _handle = (HANDLE)_beginthreadex(NULL, 0, thread_wrapper,
                                                (LPVOID)this, CREATE_SUSPENDED,
                                                &t);
               _id = t;
               if (_handle == NULL)
                  throw thread_fatal(GetLastError());
#else
               // Borland C++
               _handle = (HANDLE)_beginthreadNT(thread_wrapper, 0, (void*)this,
                                                NULL, CREATE_SUSPENDED, &_id);
               if (_handle == INVALID_HANDLE_VALUE)
                  throw thread_fatal(errno);
#endif
            }
            INLINE void start_in(void) {
               if (!TlsSetValue(self_tls_index, (LPVOID)this))
                  throw thread_fatal(GetLastError());
            }
            INLINE void start_post(const bool, const int pri) {
               if (!SetThreadPriority(_handle, pri))
                  throw thread_fatal(GetLastError());
               if (ResumeThread(_handle) == 0xffffffff)
                  throw thread_fatal(GetLastError());
            }
            INLINE void join(void** status) {
               if (WaitForSingleObject(_handle, INFINITE) != WAIT_OBJECT_0)
                  throw thread_fatal(GetLastError());
               if (status)
                  *status = _return_value;
            }
            INLINE void set_priority(const int pri) {
               if (!SetThreadPriority(_handle, priority_map(pri)))
                  throw thread_fatal(GetLastError());
            }
            INLINE void* back_ptr(void) { return _b_ptr; }
            static INLINE void exit(void* return_value) {
               thread_class* me = self();
               if (me != thread_class::Null)
                  me->_return_val = return_value;
#ifndef __BCPLUSPLUS__
               _endthreadex(0);
#else
               _endthread();
#endif
            }
            static INLINE thread_class* self(void) {
               LPVOID me = TlsGetValue(self_tls_index);
               if (me == NULL)
                  return thread_class::Null;
               return (thread_class*)me;
            }
            static INLINE void yield(void) {
               Sleep(0);
            }
      };
      // Condition variables are rather annoying to implement using NT
      // synchronisation primitives, since none of them have the atomic
      // "release mutex and wait to be signalled" which is central to the
      // idea of a condition variable.  To get around this the solution is to
      // record which threads are waiting and explicitly wake up those threads.
      //
      // Here we implement a condition variable using a list of waiting threads
      // (protected by a critical section), and a per-thread semaphore (which
      // actually only needs to be a binary semaphore).
      //
      // To wait on the cv, a thread puts itself on the list of waiting threads
      // for that xv, then releases the mutex and waits on its own personal
      // semaphore.  A signalling thread simply takes a thread from the head of
      // the list and kicks that thread's semaphore.  Broadcast is simply
      // implemented by kicking the semaphore of each waiting thread.
      //
      // The only other tricky part comes when a thread gets a timeout from a
      // timed wait on it's semaphore.  Between returning with a timeout from
      // the wait and entering the critical section, a signalling thread could
      // get in, kick the waiting thread's semaphore and remove it from the
      // list.  If this happens, the waiting thread's semaphore is now out of
      // step so it needs resetting, and the thread should indicate that it was
      // signalled reather than that it timed out.
      //
      // It is possible that the thread calling wait or timedwait is not one
      // of our threads.  In this case we have to provide a temporary data
      // structure, i.e. for the duration of the call, for the thread to link
      // itself on the list of waiting threads.  _internal_thread_dummy
      // provides such a data structure and _internal_thread_helper is a helper
      // class to deal with this special case for wait() and timedwait().  Once
      // created, the _internal_thread_dummy is cached for use by the next
      // wait() or timedwait() call from a thread that is not one of ours.
      // This is probably worth doing because creating a Semaphore is quite
      // heavy weight.
      class condition_class {
         private:
            CRITICAL_SECTION _condition;
            thread_class* waiting_head;
            thread_class* waiting_tail;
            mutex_class* _mutex;
            class _internal_thread_helper;
            class _internal_thread_dummy : public thread_class {
               public:
                  INLINE _internal_thread_dummy(void) :
                    next((_internal_thread_dummy*)0L) {}
                  INLINE ~_internal_thread_dummy(void) {}
                  friend class _internal_thread_helper;
               private:
                  _internal_thread_dummy* next;
            };
            class _internal_thread_helper {
               public:
                  INLINE _internal_thread_helper(void) :
                    d((_internal_thread_dummy*)0L),
                    t(thread_class::self()) {
                     if (cachelock == mutex_class::Null)
                        cachelock = new mutex_class;
                     if (t == thread_class::Null) {
                        cachelock->lock();
                        if (cache) {
                           d = cache;
                           cache = cache->next;
                        } else
                           d = new _internal_tread_dummy;
                        t = d;
                        cachelock->unlock();
                     }
                  }
                  INLINE ~_internal_thread_helper(void) {
                     if (d != (_internal_thread_dummy*)0L) {
                        cachelock->lock();
                        d->next = cache;
                        cache = d;
                        cachelock->unlock();
                     }
                  }
                  INLINE operator thread_class*(void) { return t; }
                  INLINE thread_class* operator->(void) { return t; }
                  static _internal_thread_dummy* cache;
                  static mutex_class* cachelock;
               private:
                  _internal_thread_dummy* d;
                  thread_class* t;
            };
         public:
            // interface to condition variables
            static const condition_class* Null;
            INLINE condition_class(mutex_class*) : _mutex(m) {
               InitializeCriticalSection(&_condition);
               waiting_head = waiting_tail = thread_class::Null;
            }
            INLINE ~condition_class(void) {
               DeleteCriticalSection(&_condition);
               if (waiting_head != thread_class::Null)
                  DEBUG() << "thread_ipc_traits<\"nt\">::condition_class::~condition_class : list of waiting threads is not empty" << nend;
            }
            INLINE void wait(void) {
               _internal_thread_helper me;
               EnterCriticalSection(&_condition);

               me->cond_next = thread_class::Null;
               me->cond_prev = waiting_tail;
               if (waiting_head == thread_class::Null)
                  waiting_head = me;
               else
                  waiting_tail->cond_next = me;
               waiting_tail = me;
               me->cond_waiting = TRUE;

               LeaveCriticalSection(&_condition);
               _mutex->unlock();
               DWORD result = WaitForSingleObject(me->cond_semaphore, INFINITE);
               _mutex->lock();
               if (result != WAIT_OBJECT_0)
                  throw condition_fatal(GetLastError());
            }
            INLINE int timedwait(const unsigned long secs,
                                 const unsigned long nsecs) {
               _internal_thread_helper me;
               EnterCriticalSection(&_condition);

               me->cond_next = thread_class::Null;
               me->cond_pref = waiting_tail;
               if (waiting_tail == thread_class::Null)
                  waiting_head = me;
               else
                  waiting_tail->cond_next = me;
               waiting_tail = me;
               me->cond_waiting = TRUE;

               LeaveCriticalSection(&_condition);
               _mutex->unlock();
               unsigned long now_sec, now_nsec;
               thread_ipc_traits::get_time_now(now_sec, now_nsec);
               DWORD timeout = (secs - now_sec) * 1000 +
                 (nsecs - now_nsecs) / 1000000;
               if ((secs <= new_sec) && ((secs < now_sec) ||
                                         (nsecs < now_nsecs)))
                  timeout = 0;
               DWORD result = WaitForSingleObject(me->cond_semaphore, timeout);
               if (result == WAIT_TIMEOUT) {
                  EnterCriticalSection(&_condition);
                  if (me->cond_waiting) {
                     if (me->cond_prev != thread_class::Null)
                        me->cond_prev->cond_next = me->cond_next;
                     else
                        waiting_head = me->cond_next;
                     if (me->cond_next != thread_class::Null)
                        me->cond_next->cond_prev = me->cond_prev;
                     else
                        waiting_tail = me->cond_prev;
                     me->cond_waiting = FALSE;
                     LeaveCriticalSection(&_condition);
                     _mutex->lock();
                     return 0;
                  }
                  // We timed out but another thread still signalled us.  Wait
                  // for the semaphore (it _must_ have been signalled) to
                  // decrement it again.  Return that we were signalled, not
                  // that we timed out.
                  LeaveCriticalSection(&_condition);
                  result = WaitForSingleObject(me->cond_semaphore, INFINITE);
               }
               if (result != WAIT_OBJECT_0)
                  throw condition_fatal(GetLastError());
               _mutex->lock();
               return 1;
            }
            INLINE void signal(void) {
               EnterCriticalSection(&_condition);
               if (waiting_head != thread_class::Null) {
                  thread_class* t = waiting_head;
                  waiting_head = t->cond_next;
                  if (waiting_head == thread_class::Null)
                     waiting_tail = thread_class::Null;
                  else
                     waiting_head->cond_prev = thread_class::Null;
                  t->cond_waiting = FALSE;
                  if (!ReleaseSemaphore(t->cond_semaphore, 1, NULL)) {
                     int rc = GetLastError();
                     LeaveCriticalSection(&_condition);
                     throw condition_fatal(rc);
                  }
               }
               LeaveCriticalSection(&_condition);
            }
            INLINE void broadcast(void) {
               EnterCriticalSection(&_condition);
               while (waiting_head != thread_class::Null) {
                  thread_class* t = waiting_head;
                  waiting_head = t->cond_next;
                  if (waiting_head == thread_class::Null)
                     waiting_tail = thread_class::Null;
                  else
                     waiting_head->cond_prev = thread_class::Null;
                  t->cond_waiting = FALSE;
                  if (!ReleaseSemaphore(t->cond_semaphore, 1, NULL)) {
                     int rc = GetLastError();
                     LeaveCriticalSection(&_condition);
                     throw condition_fatal(rc);
                  }
               }
               LeaveCriticalSection(&_condition);
            }
      };
      class semaphore_class {
         private:
            HANDLE _semaphore;
#ifndef WIN32_VC
            const unsigned long SEMAPHORE_MAX = 0x7fffffff;
#endif
         public:
            // interface to semaphores
            static const semaphore_class* Null;
            INLINE semaphore_class(const unsigned int initial) :
              _semaphore(CreateSemaphore(NULL, initial, SEMAPHORE_MAX, NULL)) {
               if (_semaphore == NULL) {
                  DEBUG() << "thread_ipc_traits<\"nt\">::semaphore_class::semaphore_class : CreateSemaphore error "
                          << GetLastError() << nend;
                  throw semaphore_fatal(GetLastError());
               }
            }
            INLINE ~semaphore_class(void) {
               if (!CloseHandle(_semaphore)) {
                  DEBUG() << "thread_ipc_traits<\"nt\">::semaphore_class::~semaphore_class : CloseHandle error "
                          << GetLastError() << nend;
                  throw semaphore_fatal(GetLastError());
               }
            }
            INLINE void wait(void) {
               if (WaitForSingleObject(_semaphore, INFINITE) != WAIT_OBJECT_0)
                  throw semaphore_fatal(GetLastError());
            }
            INLINE int trywait(void) {
               switch (WaitForSingleObject(_semaphore, 0)) {
               case WAIT_OBJECT_0: return 1;
               case WAIT_TIMEOUT: return 0;
               }
               throw semaphore_fatal(GetLastError());
               return 0;  // never really gets here, but the compilers like it
            }
            INLINE void post(void) {
               if (!ReleaseSemaphore(_semaphore, 1, NULL))
                  throw semaphore_fatal(GetLastError());
            }
      };
      class library_class {
      public:
        // interface to dynamically loaded libraries
        static library_class* const Null;
        INLINE library_class(std::string&) {
          throw lib_load_invalid();
        }
        INLINE ~library_class(void) {
          throw lib_load_invalid();
        }
        INLINE void* get_symbol(std::string&) {
          throw lib_load_invalid();
        }
      };
      // BROKEN!!  NOT FINISHED
      class file_class {
        //interface to file IO
      public:
        INLINE file_class(std::string& file) : _filename(file), _fhandle(NULL){};
        INLINE file_class() : _filename(NULL), _fhandle(NULL) {};
        INLINE ~file_class() { close(); }
        INLINE void setFile(std::string& file) { _filename = file; close(); }
        INLINE bool open(const int mode) {
          close();
          if (_filename.data() != NULL) {
            switch(mode){
            case 0: _fhandle = fopen(_filename.data(), "rb"); break;
            case 1: _fhandle = fopen(_filename.data(), "wb"); break;
            case 2: _fhandle = fopen(_filename.data(), "rwb"); break;
            default: _fhandle = fopen(_filename.data(), "rb"); break;
            }
            if (_fhandle != NULL) return true;
          }
          return false;
        }
        INLINE void close(void) {
          if (_fhandle != NULL) { fclose(_fhandle); _fhandle = NULL; }
        }
        /*
        INLINE bool readin(MemBuf& buffer, int numBytes) {
          BYTE* block = new BYTE[numBytes];
          if (_fhandle != NULL){
            fread(block, sizeof(BYTE), numBytes, _fhandle);
            if (!ferror(_fhandle)){ buffer.addBack(block); delete [] block; return true; }
          }
          delete [] block;
          return false;
        }
        INLINE bool writeout(MemBuf& buffer, int numBytes) {
          BYTE* block = buffer.getBlock(0, numBytes);
          if (_fhandle != NULL) {
            fwrite(block, sizeof(BYTE), numBytes, _fhandle);
            if (!ferror(_fhandle)) { delete [] block; return true; }
          }
          delete [] block;
          return false;
        }
        INLINE bool readin(BYTE* block, int numBytes) {
          if (_fhandle != NULL){
            fread(block, sizeof(BYTE), numBytes, _fhandle);
            if (!ferror(_fhandle)){ return true; }
          }
          return false;
        }
        INLINE bool writeout(BYTE* block, int numBytes) {
          if (_fhandle != NULL) {
            fwrite(block, sizeof(BYTE), numBytes, _fhandle);
            if (!ferror(_fhandle)) { return true; }
          }
          return false;
        }
        */
      private:
        string _filename;
        FILE* fhandle;
      };
      static INLINE mutex_class *make_mutex(void) {
         return new mutex_class;
      }
      static INLINE condition_class *make_condition(mutex_class* m) {
         return new condition_class(m);
      }
      static INLINE semaphore_class *make_semaphore(const unsigned int initial) {
         return new semaphore_class(initial);
      }
      static INLINE thread_class *make_thread(void* data) {
         return new thread_class(data);
      }
      static INLINE library_class *make_thread(std::string&) {
        throw lib_load_invalid();
      }
      static INLINE file_class *make_file(std::string& file) {
        return new file_class(file);
      }
      static INLINE void sleep(const unsigned long secs,
                               const unsigned long nsecs = 0) {
         const DWORD MAX_SLEEP_SECONDS = (DWORD)4294966; // (2**32-2)/1000
         if (secs <= MAX_SLEEP_SECONDS) {
            Sleep(secs * 1000 + nsecs / 1000000);
            return;
         }
         DWORD no_of_max_sleeps = secs / MAX_SLEEP_SECONDS;
         for (DWORD i=0; i<no_of_max_sleeps; ++i)
            Sleep(MAX_SLEEP_SECONDS * 1000);
         Sleep((secs % MAX_SLEEP_SECONDS) * 1000 + nsecs / 1000000);
      }
      static INLINE void get_time(unsigned long& abs_secs,
                                  unsigned long& abs_nsecs,
                                  const unsigned long rel_secs = 0,
                                  const unsigned long rel_nsecs = 0) {
         get_time_now(&abs_secs, &abs_nsecs);
         abs_nsecs += rel_nsecs;
         abs_secs += rel_secs + abs_nsecs / 1000000000;
         abs_nsecs = abs_nsecs % 1000000000;
      }
      static INLINE int set_atomic(int&, int) {
        throw set_atomic_invalid();
      }
      static INLINE int inc_atomic(int&, int) {
        throw inc_atomic_invalid();
      }
      static INLINE int dec_atomic(int&, int) {
        throw dec_atomic_invalid();
      }
      // this really should be private, but it is annoying to make config a
      // friend
      static INLINE void __set_self_tls_index(DWORD i) {
        self_tls_index = i;
      }
   private:
      // other data specific to this implementation
      static DWORD self_tls_index;
      // and a special, bonus, NT implementation function
      static const int days_in_preceding_months[12];
      static const int days_in_preceding_months_leap[12];
      static INLINE void get_time_now(unsigned long& abs_sec,
                                      unsigned long& abs_nsec) {
         SYSTEMTIME st;
         GetSystemTime(&st);
         abs_nsec = st.wMilliseconds * 1000000;
         // this formula should work until the 1st of March 2100.  If this code
         // is still in use by then, I pitty us all.
         DWORD days = ((st.wYear - 1970) * 365 + (st.wYear - 1969) / 4
                       + ((st.wYear % 4)
                          ? days_in_preceding_months[st.wMonth - 1]
                          : days_in_preceding_months_leap[st.wMonth - 1])
                       + st.wDay - 1);
         abs_sec = st.wSecond + 60 * (st.wMinute + 60 * (st.wHour + 24 * days));
      }
};

#endif /* __IPC_NT_TRAITS_H__ */
