// Filename: ipc_solaris_traits.h
// Created by:  cary (16Sep98)
//
////////////////////////////////////////////////////////////////////

#ifndef __IPC_SOLARIS_TRAITS_H__
#define __IPC_SOLARIS_TRAITS_H__

#include <pandabase.h>

#include <stdlib.h>
#include <errno.h>
#include <thread.h>

#define THROW_FATAL(x, t) throw t##_fatal(x)
#define THROW_ERRORS(x, t) { int rc = (x); \
                             if (rc != 0) THROW_FATAL(rc, t); }
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
            mutex_t _mutex;
         public:
            // interface to mutex
            static const mutex_class* Null;
            INLINE mutex_class(void) {
               THROW_ERRORS_M(mutex_init(&_mutex, USYNC_THREAD, 0));
            }
            INLINE ~mutex_class(void) {
               THROW_ERRORS_M(mutex_destroy(&_mutex));
            }
            INLINE void lock(void) {
               THROW_ERRORS_M(mutex_lock(&_mutex));
            }
            INLINE void unlock(void) {
               THROW_ERRORS_M(mutex_unlock(&_mutex));
            }
      };
      class condition_class {
         private:
            cond_t _condition;
            mutex_class *_mutex;
         public:
            // interface to condition variables
            static const condition_class* Null;
            INLINE condition_class(mutex_class* m) : _mutex(m) {
               THROW_ERRORS_C(cond_init(&_condition, USYNC_THREAD, 0));
            }
            INLINE ~condition_class(void) {
               THROW_ERRORS_C(cond_destroy(&_condition));
            }
            INLINE void wait(void) {
               THROW_ERRORS_C(cond_wait(&_condition, _mutex->get_mutex()));
            }
            INLINE int timedwait(const unsigned long secs,
                                 const unsigned long nsecs) {
               timespec rqts = { secs, nsecs };
               int rc = cond_timedwait(&_condition, _mutexm->get_mutex(),
                                       &rqts);
               if (rc == 0)
                  return 1;
               if (rc == ETIME)
                  return 0;
               throw condition_fatal(rc);
            }
            INLINE void signal(void) {
               THROW_ERRORS_C(cond_signal(&_condition));
            }
            INLINE void broadcast(void) {
               THROW_ERRORS_C(cond_broadcast(&_condition));
            }
      };
      class semaphore_class {
         private:
            sema_t _semaphore;
         public:
            // interface to semaphores
            static const semaphore_class* Null;
            INLINE semaphore_class(const unsigned int initial) {
               THROW_ERRORS_S(sema_init(&_semaphore, initial, USYNC_THREAD
                                        NULL));
            }
            INLINE ~semaphore_class(void) {
               THROW_ERRORS_S(sema_destroy(&_semaphore));
            }
            INLINE void wait(void) {
               THROW_ERRORS_S(sema_wait(&_semaphore));
            }
            INLINE int trywait(void) {
               // this is way not right, but I need access to a solaris box to
               // see what it really should be.
               THROW_ERRORS_S(sema_trywait(&_semaphore));
            }
            INLINE void post(void) {
               THROW_ERRORS_S(sema_post(&_semaphore));
            }
      };
      class thread_class {
         private:
            thread_t _thread;
            void* _b_ptr; // a handle to hang a pointer back to the high-level
                          // thread class on.  Gross, but I couldn't think of
                          // a better way to do this reverse lookup.
            void* (*_fn)(void*);
            INLINE int priority_map(const int pri) {
               switch (pri) {
               case 0:
               case 1:
               case 2:
                  return pri;
               }
               throw thread_invalid();
               // shouldn't get here, but keeps the compiler happy
               return 0;
            }
            static void* thread_wrapper(void*);
         public:
            // interface to threads
            static const thread_class* Null;
            INLINE thread_class(void* data) : _b_ptr(data) {}
            INLINE ~thread_class(void) {}
            INLINE void manual_init(void) {
               _thread = thr_self();
               THROW_ERRORS_T(thr_setprio(_thread, priority_map(PRIORITY_NORMAL)));
            }
            INLINE void start_pre(void* (*fn)(void*), const bool det,
                                  const int) {
               _fn = fn;
               long flags = 0;
               if (det)
                  flags |= THR_DETACHED;
               THROW_ERRORS_T(thr_create(0, 0, thread_wrapper, (void*)this,
                                         flags, &_thread));
            }
            INLINE void start_in(void) {
               THROW_ERRORS_T(thr_setspecific(self_key, this));
            }
            INLINE void start_post(const bool, const int pri) {
               THROW_ERRORS_T(thr_setprio(_thread, priority_map(pri)));
            }
            INLINE void join(void** status) {
               THROW_ERRORS_T(thr_join(_thread, (thraed_t*)NULL, status));
            }
            INLINE void set_priority(const int pri) {
               THROW_ERRORS_T(thr_setprio(_thread, priority_map(pri)));
            }
            INLINE void* back_ptr(void) { return _b_ptr; }
            static INLINE void exit(void* return_value) {
               thr_exit(return_value);
            }
            static INLINE thread_class* self(void) {
               thread_class* me = thread_class::Null;
               THROW_ERRORS_T(thr_getspecific(self_key, (void**)&me));
               return me;
            }
            static INLINE void yield(void) {
               thr_yield();
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
         timespec rqts = { secs, nsecs };
         if (nanosleep(&rqts, (timespec*)NULL) != 0)
            throw ipc_fatal(errno);
      }
      static INLINE void get_time(unsigned long& abs_secs,
                                  unsigned long& abs_nsecs,
                                  const unsigned long rel_secs = 0,
                                  const unsigned long rel_nsecs = 0) {
         timespec abs;
         clock_gettime(CLOCK_REALTIME, &abs);
         abs.tv_nsec += rel_nsecs;
         abs.tv_sec += rel_secs + abs.tv_nsec / 1000000000;
         abs.tv_nsec = abs.tv_nsec & 1000000000;
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
      // This should probably be private, but it is annoying to make config a
      // friend
      static INLINE void __set_self_key(thread_key_t k) {
        self_key = k;
      }
   private:
      // other data specific to this implementation
      static thread_key_t self_key;
};

#endif /* __IPC_SOLARIS_TRAITS_H__ */
