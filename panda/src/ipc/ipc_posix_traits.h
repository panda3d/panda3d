// Filename: ipc_posix_traits.h
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

#ifndef __IPC_POSIX_TRAITS_H__
#define __IPC_POSIX_TRAITS_H__

#include <pandabase.h>

#if defined(__alpha__) && defined(__osf1__) || defined(__hpux__)
// stop unnecessary definitions of TRY, etc on OSF
#ifndef EXC_HANDLING
#define EXC_HANDLING
#endif /* EXC_HANDLING */
#endif /* __alpha__, __hpux__ */

// hack alert!
#ifdef __GNUC__
#define _MIT_POSIX_THREADS 1
#define PthreadDraftVersion 7
#define NoNanoSleep

#include <unistd.h>
/*
extern "C" {
void usleep(unsigned long);
}
*/
#endif /* __GNUC__ */

#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

#if (defined(__GLIBC__) && __GLIBC__ >= 2)
// typedef of struct timeval and gettimeofday();
#include <sys/time.h>
#include <unistd.h>
#endif /* __GLIBC__ */

// #if defined(__linux__) && defined(_MIT_POSIX_THREADS)
// #include <sys/timers.h>
// #endif /* __linux__ & _MIT_POSIX_THREADS */

#if defined(__sgi)
#define PthreadSupportThreadPriority
// this next is just a guess
#define PthreadDraftVersion 10
#include <sched.h>
#endif

#define THROW_FATAL(x, t) throw t##_fatal(x)
#if (PthreadDraftVersion <= 6)
#define ERRNO(x) (((x) != 0) ? (errno) : 0)
#ifdef __VMS
// pthread_setprio returns old priority on success (draft version 4:
// OpenVms version < 7)
#define THROW_ERRORS(x, t) { if ((x) == -1) THROW_FATAL(errno, t); }
#else
#define THROW_ERRORS(x, t) { if ((x) != 0) THROW_FATAL(errno, t); }
#endif /* __VMS */
#else
#define ERRNO(x) (x)
#define THROW_ERRORS(x, t) { int rc = (x); \
                             if (rc != 0) THROW_FATAL(rc, t); }
#endif /* PthreadDraftVersion 6 */
#define THROW_ERRORS_G(x) THROW_ERRORS(x, ipc)
#define THROW_ERRORS_M(x) THROW_ERRORS(x, mutex)
#define THROW_ERRORS_C(x) THROW_ERRORS(x, condition)
#define THROW_ERRORS_S(x) THROW_ERRORS(x, semaphore)
#define THROW_ERRORS_T(x) THROW_ERRORS(x, thread)

class EXPCL_PANDAEXPRESS ipc_traits {
   public:
      class mutex_class;
      class condition_class;
      class semaphore_class;
      class thread_class;
      class library_class;
      class mutex_class {
         private:
            pthread_mutex_t _mutex;
         public:
            // interface to mutex
            static mutex_class* const Null;
            INLINE mutex_class(void) {
#if (PthreadDraftVersion == 4)
               THROW_ERRORS_M(pthread_mutex_init(&_mutex, pthread_mutexattr_default));
#else
               THROW_ERRORS_M(pthread_mutex_init(&_mutex, 0));
#endif
            }
            INLINE ~mutex_class(void) {
               THROW_ERRORS_M(pthread_mutex_destroy(&_mutex));
            }
            INLINE void lock(void) {
               THROW_ERRORS_M(pthread_mutex_lock(&_mutex));
            }
            INLINE void unlock(void) {
               THROW_ERRORS_M(pthread_mutex_unlock(&_mutex));
            }
            INLINE pthread_mutex_t* get_mutex(void) { return &_mutex; }
      };
      class condition_class {
         private:
            pthread_cond_t _condition;
            mutex_class* _mutex;
         public:
            // interface to condition variables
            static condition_class* const Null;
            INLINE condition_class(mutex_class* m) : _mutex(m) {
#if (PthreadDraftVersion == 4)
               THROW_ERRORS_C(pthread_cond_init(&_condition, pthread_condattr_default));
#else
               THROW_ERRORS_C(pthread_cond_init(&_condition, 0));
#endif
            }
            INLINE ~condition_class(void) {
               THROW_ERRORS_C(pthread_cond_destroy(&_condition));
            }
            INLINE void wait(void) {
               THROW_ERRORS_C(pthread_cond_wait(&_condition,
                                                _mutex->get_mutex()));
            }
            INLINE int timedwait(const unsigned long secs,
                                 const unsigned long nsecs) {
               timespec rqts = { (long)secs, (long)nsecs };
               int rc = ERRNO(pthread_cond_timedwait(&_condition,
                                                     _mutex->get_mutex(),
                                                     &rqts));
               if (rc == 0)
                  return 1;
#if (PthreadsDraftVersion <= 6)
               if (rc == EAGAIN)
                  return 0;
#endif
               if (rc == ETIMEDOUT)
                  return 0;
               throw condition_fatal(rc);
            }
            INLINE void signal(void) {
               THROW_ERRORS_C(pthread_cond_signal(&_condition));
            }
            INLINE void broadcast(void) {
               THROW_ERRORS_C(pthread_cond_broadcast(&_condition));
            }
      };
      class semaphore_class {
         private:
            mutex_class _mutex;
            condition_class _condition;
            int _value;
            class mutex_class_lock {
               public:
                  INLINE mutex_class_lock(mutex_class& m) : _mutex(m) {
                     _mutex.lock();
                  }
                  INLINE ~mutex_class_lock(void) { _mutex.unlock(); }
               private:
                  mutex_class& _mutex;
            };
         public:
            // interface to semaphores
            static semaphore_class* const Null;
            INLINE semaphore_class(const unsigned int initial) : _mutex(),
              _condition(&_mutex), _value(initial) {}
            INLINE ~semaphore_class(void) {}
            INLINE void wait(void) {
               mutex_class_lock l(_mutex);
               while (_value == 0)
                  _condition.wait(&_mutex);
               --_value;
            }
            INLINE int trywait(void) {
               mutex_class_lock l(_mutex);
               if (_value == 0) {
                  return 0;
               }
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
      class thread_class {
         private:
            pthread_t _thread;
            void* _b_ptr; // a handle to hang a pointer back to the high-level
                          // thread class on.  Gross, but I couldn't think of
                          // a better way to do this reverse lookup.
            void* (*_fn)(void*);
            INLINE int priority_map(const int pri) {
#ifdef PthreadSupportThreadPriority
               switch (pri) {
               case 0:
                  return lowest_priority;
               case 1:
                  return normal_priority;
               case 2:
                  return highest_priority;
               }
#endif /* PthreadSupportThreadPriority */
               throw thread_invalid();
            }
            static void* thread_wrapper(void*);
         public:
            // interface to threads
            static thread_class* const Null;
            INLINE thread_class(void* data) : _b_ptr(data) {}
            INLINE ~thread_class(void) {}
            INLINE void manual_init(void) {
               _thread = pthread_self();
#ifdef PthreadSupportPriority
#if (PthreadDraftVersion == 4)
               THROW_ERRORS_T(pthread_setprio(_thread,
                                              priority_map(PRIORITY_NORMAL)));
#elif (PthreadDraftVersion == 6)
               pthread_attr_t attr;
               pthread_attr_init(&attr);
               THROW_ERRORS_T(pthread_attr_setprio(&attr,
                                                   priority_map(PRIORITY_NORMAL)));
               THROW_ERRORS_T(pthread_setschedattr(_thread, attr));
#else
               struct sched_param sparam;
               sparam.sched_priority = priority_map(PRIORITY_NORMAL);
               THROW_ERRORS_T(pthread_setschedparam(_thread, SCHED_OTHER,
                                                    &param));
#endif /* PthreadDraftVersion */
#endif /* PthreadSupportPriority */
            }
            INLINE void start_pre(void* (*fn)(void*), const bool,
                                  const int pri) {
               _fn = fn;
               pthread_attr_t attr;
#if (PthreadDraftVersion == 4)
               pthread_attr_create(&attr);
#else
               pthread_attr_init(&attr);
#endif
#if (PthreadDraftVersion == 8)
               pthread_attr_setdetachedstate(&attr, PTHREAD_CREATE_UNDETACHED);
#endif
#ifdef PthreadSupportThreadPriority
#if (PthreadDraftVersion <= 6)
               THROW_ERRORS_T(pthread_attr_setprio(&attr, priority_map(pri)));
#else
               struct sched_param sparam;
               sparam.sched_priority = priority_map(pri);
               THROW_ERRORS_T(pthread_attr_setschedparam(&attr, &sparam));
#endif
#endif /* PthreadSupportThreadPriority */
#if defined(__osf1__) && defined(__alpha__) || defined(__VMS)
               // we're going to require a larger stack size then the default
               // (21120) on OSF/1
               THROW_ERRORS_T(pthread_attr_setstacksize(&attr, 32768));
#endif
#if (PthreadDraftVersion == 4)
               THROW_ERRORS_T(pthread_create(&_thread, attr, thread_wrapper,
                                             (void*)this));
               pthread_attr_delete(&attr);
#else
               THROW_ERRORS_T(pthread_create(&_thread, &attr, thread_wrapper,
                                             (void*)this));
               pthread_attr_destroy(&attr);
#endif
            }
            INLINE void start_in(void) {
               THROW_ERRORS_T(pthread_setspecific(self_key, this));
            }
            INLINE void start_post(const bool det, const int) {
               if (det) {
#if (PthreadDraftVersion <= 6)
                  THROW_ERRORS_T(pthread_detach(&_thread));
#else
                  THROW_ERRORS_T(pthread_detach(_thread));
#endif
               }
            }
            INLINE void join(void** status) {
               THROW_ERRORS_T(pthread_join(_thread, status));
#if (PthreadDraftVersion == 4)
               // With draft 4 pthreads implementations (HPUS 10.x and Digital
               // Unix 3.2), you have to detach the thread after join.  If not,
               // the storage for the thread will not be reclaimed.
               THROW_ERRORS_T(pthread_detach(&_thread));
#endif
            }
            INLINE void set_priority(const int pri) {
#ifdef PthreadSupportThreadPriority
#if (PthreadDraftVersion == 4)
               THROW_ERRORS_T(pthread_setprio(_thread, priority_map(pri)));
#elif (PthreadDraftVersion == 6)
               pthread_attr_t attr;
               pthread_attr_init(&attr);
               THROW_ERRORS_T(pthread_attr_setprio(&attr, priority_map(pri)));
               THROW_ERRORS_T(pthread_setschedattr(_thread, attr));
#else
               struct sched_param sparam;
               sparam.sched_priority = priority_map(pri);
               THROW_ERRORS_T(pthread_setschedparam(_thread, SCHED_OTHER,
                                                    &sparam));
#endif
#endif /* PthreadSupportThreadPriority */
            }
            INLINE void* back_ptr(void) { return _b_ptr; }
            static INLINE void exit(void* return_value) {
               pthread_exit(return_value);
            }
            static INLINE thread_class* self(void) {
               thread_class* me(thread_class::Null);
#if (PthreadDraftVersion <= 6)
               THROW_ERRORS_T(pthread_getspecific(self_key, (void**)&me));
#else
               me = (thread_class*)pthread_getspecific(self_key);
#endif
               return me;
            }
            static INLINE void yield(void) {
#if (PthreadDraftVersion == 6)
               pthread_yield(NULL);
#elif (PthreadDraftVersion < 9)
               pthread_yield();
#else
               THROW_ERRORS_T(sched_yield());
#endif
            }
      };
      class library_class {
      public:
        static library_class* const Null;
        INLINE library_class(ConfigString&) {
          throw lib_load_invalid();
        }
        INLINE ~library_class(void) {
          throw lib_load_invalid();
        }
        INLINE void* get_symbol(ConfigString&) {
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
      static INLINE library_class *make_library(ConfigString&) {
        throw lib_load_invalid();
      }
      static INLINE void sleep(const unsigned long secs,
                               const unsigned long nsecs = 0) {
         timespec rqts = { (long)secs, (long)nsecs};
#ifndef NoNanoSleep
         if (nanosleep(&rqts, (timespec*)NULL) != 0) {
            throw ipc_fatal(errno);
         }
#else
#if defined(__osf1__) && defined(__alpha__) || defined(__hpux__) && (__OSVERSION__ == 10) || defined(__VMS) || defined(__SINIX__)
         if (pthread_delay_np(&rqts) != 0) {
            throw ipc_fatal(errno);
         }
#elif defined(__linux__) || defined(__aix__)
         // this seems terribly inaccurate, and should be re-examined
         if (secs > 2000)
            sleep(secs);
         else
            usleep(secs * 1000000 + (nsecs / 1000));
#else
         throw ipc_invalid();
#endif
#endif /* NoNanoSleep */
      }
      static INLINE void get_time(unsigned long& abs_secs,
                                  unsigned long& abs_nsecs,
                                  const unsigned long rel_secs = 0,
                                  const unsigned long rel_nsecs = 0) {
         timespec abs;
#if defined(__osf1__) && defined(__alpha__) || defined(__hpux__) && (__OSVERSION__ == 10) || defined(__VMS) || defined(__SINIX__)
         timespec rel;
         rel.tv_sec = rel_secs;
         rel.tv_nsec = rel_nsecs;
         THROW_ERRORS_G(pthread_get_expiration_np(&rel, &abs));
#else
#if defined(__linux__) || defined(__aix__)
         struct timeval tv;
         gettimeofday(&tv, NULL);
         abs.tv_sec = tv.tv_sec;
         abs.tv_nsec = tv.tv_usec * 1000;
#else
         clock_gettime(CLOCK_REALTIME, &abs);
#endif /* __linux__, __aix__, ... */
         abs.tv_nsec += rel_nsecs;
         abs.tv_sec += rel_secs + abs.tv_nsec / 1000000000;
         abs.tv_nsec = abs.tv_nsec & 1000000000;
#endif /* __osf1__, __alpha__, ... */
         abs_secs = abs.tv_sec;
         abs_nsecs = abs.tv_nsec;
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
      // These should be private, but is annoying to make config a
      // friend
      static INLINE void __set_self_key(pthread_key_t k) {
        self_key = k;
      }
      static INLINE void __set_lowest_priority(int p) {
        lowest_priority = p;
      }
      static INLINE void __set_normal_priority(int p) {
        normal_priority = p;
      }
      static INLINE void __set_highest_priority(int p) {
        highest_priority = p;
      }
   private:
      // other data specific to this implementation
      static pthread_key_t self_key;
#ifdef PthreadSupportThreadPriority
      static int lowest_priority;
      static int normal_priority;
      static int highest_priority;
#endif /* PthreadSupportThreadPriority */
};

#endif /* __IPC_POSIX_TRAITS_H__ */
