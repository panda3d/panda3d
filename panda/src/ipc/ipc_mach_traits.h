// Filename: ipc_mach_traits.h
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

#ifndef __IPC_MACH_TRAITS_H__
#define __IPC_MACH_TRAITS_H__

#include <pandabase.h>

#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <mach/cthreads.h>

class EXPCL_PANDAEXPRESS ipc_traits {
   public:
      class mutex_class;
      class condition_class;
      class semaphore_class;
      class thread_class;
      class library_class;
      class mutex_class {
         private:
            struct mutex _mutex;
         public:
            // interface to mutex
            static const mutex_class* Null;
            INLINE mutex_class(void) { mutex_init(&_mutex); }
            INLINE ~mutex_class(void) { mutex_clear(&_mutex); }
            INLINE void lock(void) { mutex_lock(&_mutex); }
            INLINE void unlock(void) { mutex_unlock(&_mutex); }
            INLINE struct mutex* get_mutex(void) { return &_mutex; }
      };
      class condition_class {
         private:
            struct condition _condition;
            mutex_class *_mutex;
            typedef struct alarmclock_args {
               unsigned long secs;
               unsigned long nsecs;
               bool wakeup;
               condition_class* condition;
               mutex_class* mutex;
            };
            INLINE any_t alarmclock(any_t arg) {
               alarmclock_args* alarm = (alarmclock_args*)arg;
               thread_ipc_traits::sleep(alarm->secs, alarm->nsecs);
               alarm->mutex->lock();
               alarm->wakeup = TRUE;
               alarm->condition->signal();
               alarm->mutex->unlock();
               return (any_t)TRUE;
            }
         public:
            // interface to condition variables
            static const condition_class* Null;
            INLINE condition_class(mutex_class* m) : _mutex(m) {
              condition_init(&_condition);
            }
            INLINE ~condition_class(void) { condition_clear(&_condition); }
            INLINE void wait(void) {
               condition_wait(&_condition, _mutex->get_mutex());
            }
            INLINE int timedwait(const unsigned long secs,
                                 const unsigned long nsecs) {
               alarmclock_args alarm;

               thread_ipc_traits::get_time(&alarm.secs, &alarm.nsecs, 0, 0);
               if (secs < alarm.secs || (secs == alarm.secs &&
                                         nsecs <= alarm.nsecs))
                  return ETIMEDOUT;
               alarm.secs = secs - alarm.secs;
               if (nsecs <= alarm.nsecs) {
                  alarm.nsecs = 1000000 - alarm.nsecs + nsecs;
                  --alarm.secs;
               } else
                  alarm.nsecs = nsecs - alarm.nsecs;
               alarm.mutex = _mutex;
               alarm.condition = this;
               alarm.wakeup = FALSE;
               cthread_t ct = cthread_fork((cthread_fn_t)alarmclock,
                                           (any_t)&alarm);
               cthread_detach(ct);

               condition_wait(&_condition, _mutex->get_mutex());

               if (alarm.wakeup)
                  return 0;

               // interrupt the alarmclock thread sleep
               cthread_abort(ct);

               // wait until it has signalled the condition
               condition_wait(&_condition, _mutex->get_mutex());
               return 1;
            }
            INLINE void signal(void) { condition_signal(&_condition); }
            INLINE void broadcast(void) { condition_signal(&_condition); }
      };
      class semaphore_class {
         private:
            mutex_class _mutex;
            condition_class _condition;
            int _value;
         public:
            // interface to semaphores
            static const semaphore_class* Null;
            INLINE semaphore_class(const unsigned int initial) : _mutex(),
               _condition(&_mutex), _value(initial) {}
            INLINE ~semaphore_class(void) {}
            INLINE void wait(void) {
               _mutex->lock();
               while (_value == 0)
                  _condition.wait(&_mutex);
               --_value;
               _mutex->unlock();
            }
            INLINE int trywait(void) {
               _mutex->lock();
               if (_value == 0) {
                  _mutex->unlock();
                  return 0;
               }
               --_value;
               _mutex->unlock();
               return 1;
            }
            INLINE void post(void) {
               _mutex->lock();
               if (_value == 0)
                  _condition.signal();
               ++_value;
               _mutex->unlock();
            }
      };
      class thread_class {
         private:
            chread_d _thread;
            void* _b_ptr; // a handle to hang a pointer back to the high-level
                          // thread class on.  Gross, but I couldn't think of
                          // a better way to do this reverse lookup.
            void* (*_fn)(void*);
            INLINE int priority_map(const int pri) {
               switch (pri) {
               case 0:
                  return 0;
               case 1:
                  return normal_priority;
               case 2:
                  return high_priority;
               default:
                  return -1;
               }
            }
            static void* thread_wrapper(void*);
         public:
            // interface to threads
            static const thread_class* Null;
            INLINE thread_class(void* data) : _b_ptr(data) {}
            INLINE ~thread_class(void) {}
            INLINE void manual_init(void) {
               _thread = cthread_self();
            }
            INLINE void start_pre(void* (*fn)(void*), const bool, const int) {
               _fn = fn;
               _thread = cthread_fork(thread_wrapper, (any_t)this);
            }
            INLINE void start_in(void) {
               cthread_set_data(cthread_self(), (any_t)this);
            }
            INLINE void start_post(const bool det, const int) {
               if (det)
                  cthread_detach(_thread);
            }
            INLINE void join(void** status) {
               *status = cthread_join(_thread);
            }
            INLINE void set_priority(const int pri) {
               kern_return_t rc = cthread_priority(_thread, priority_map(pri),
                                                   FALSE);
               if (rc != KERN_SUCCESS)
                  throw thread_fatal(errno);
            }
            INLINE void* back_ptr(void) { return _b_ptr; }
            static INLINE void exit(void* return_value) {
               cthread_exit(return_value);
            }
            static INLINE thread_class* self(void) {
               thread_class* me = (thread_class*)cthread_data(cthread_self());
               // if (me == thread_class::Null)  not one of ours
               return me;
            }
            static INLINE void yield(void) {
               cthread_yield();
            }
      };
      class library_class {
      public:
        static library_class* const Null;
        INLINE library_class(str&) {
          throw lib_load_invalid();
        }
        INLINE ~library_class(void) {
          throw lib_load_invalid();
        }
        INLINE void* get_symbol(str&) {
          throw lib_load_invalid();
        }
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
      static INLINE library_class *make_library(str&) {
        throw lib_load_invalid();
      }
      static INLINE void sleep(const unsigned long secs,
                               const unsigned long nsecs = 0) {
         const unsigned MAX_SLEEP_SECONDS=(unsigned)4294966;  // (2**32-2)/1000
         if (secs <= MAX_SLEEP_SECONDS) {
            thread_switch(THREAD_NULL, SWITCH_OPTION_WAIT,
                          secs * 1000 + nsecs / 1000000);
            return;
         }
         unsigned no_of_max_sleeps = secs / MAX_SLEEP_SECONDS;
         for (unsigned i=0; i<no_of_max_sleeps; ++i)
            thread_switch(THREAD_NULL, SWITCH_OPTION_WAIT,
                          MAX_SLEEP_SECONDS * 1000);
         thread_switch(THREAD_NULL, SWITCH_OPTION_WAIT,
                       (secs % MAX_SLEEP_SECONDS) * 1000 + nsecs / 1000000);
      }
      static INLINE void get_time(unsigned long& abs_secs,
                                  unsigned long& abs_nsecs,
                                  const unsigned long rel_secs = 0,
                                  const unsigned long rel_nsecs = 0) {
         int rc;
         unsigned long tv_sec;
         unsigned long tv_nsec;
         struct timeval tv;

         rc = gettimeofday(&tv, NULL);
         if (rc)
            throw thread_fatal(rc);
         tv_sec = tv.tv_sec;
         tv_nsec = tv.tv_usec * 1000;
         tv_nsec += rel_nsecs;
         tv_sec += rel_secs + tv_nsec / 1000000000;
         tv_nsec = tv_nsec % 1000000000;
         abs_secs = tv_sec;
         abs_nsecs = tv_nsec;
      }
      static INLINE int set_atomic(int&, int) {
        throw set_atomic_invalid();
      }
      static INLINE int inc_atomic(int&) {
        throw inc_atomic_invalid();
      }
      static INLINE int dec_atomic(int&) {
        throw dec_atomic_invalid();
      }
      // these really should be private, but it's annoying to make config a
      // friend
      static INLINE __set_normal_priority(int p) {
        normal_priority = p;
      }
      static INLINE __set_highest_priority(int p) {
        highest_priority = p;
      }
   private:
      // other data specific to this implementation
      static int normal_priority;
      static int highest_priority;
};

#endif /* __IPC_MACH_TRAITS_ */
