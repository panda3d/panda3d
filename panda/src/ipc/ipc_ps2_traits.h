// Filename: ipc_ps2_traits.h
// Created by:  
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

#ifndef __ipc_ps2_traits_h__
#define __ipc_ps2_traits_h__

#include <stdio.h>
#include <pandabase.h>
#include <stdlib.h>
#include <eekernel.h>
#include "plist.h"
#include "pvector.h"
#include "pmap.h"

// OK.  The deal with the PS2 is that there is native library thread support
// for threads and semaphors.  Nothing else.  This is why mutex and condition
// are based off of semaphore_class- there's no other way to have threads sleep
// (instead of spin).

// Spinning is VERY BAD on the PS2.  There's no time-sharing- it's based off of
// priority.  Threads are tasked out when they enter WAIT (sleep) mode.  Threads
// are NOT tasked out when they're banging a while loop.  If a thread is waiting
// for a variable, but not sleeping, deadlock.

class EXPCL_PANDAEXPRESS ipc_traits
  {
  public:

    // prototype all of the classes here

    class mutex_class;
    class condition_class;
    class semaphore_class;
    class thread_class;
    class library_class;

    //// semaphore_class definition

    class semaphore_class
    {
      private:

        SemaParam m_sema_param;
        int m_sema_id;

        static const int sema_max_size = 32;

      public:

        static semaphore_class* const Null;

        INLINE semaphore_class(const unsigned int initial = 1) {
          m_sema_param.initCount = initial;
          m_sema_param.maxCount = sema_max_size;
          m_sema_param.option = 0;

          m_sema_id = CreateSema(&m_sema_param);

          if (m_sema_id < 0)
            throw semaphore_fatal(-1);
        }

        INLINE ~semaphore_class(void) {
          DeleteSema(m_sema_id);
        }

        INLINE void wait(void) {
          int result;

          result = WaitSema(m_sema_id);

          // see if we overflowed the semaphore's queue

          if (result != m_sema_id)
            throw semaphore_fatal(-1);
        }

        INLINE int trywait(void) {
          SemaParam sema_param;

          // query the semaphor in question directly
          // we can't use poll here, because if the semaphor is
          // grabbable, poll will grab it.

          ReferSemaStatus(m_sema_id, &sema_param);

          if (sema_param.currentCount == 0)
            return 1;

          return 0;
        }

        INLINE void post(void) {
          SignalSema(m_sema_id);
        }
    };

    //// mutex_class definition

    class mutex_class
    {
      private:

        semaphore_class m_semaphore;

      public:

        static mutex_class* const Null;

        inline mutex_class(void) {}
        inline ~mutex_class(void) {}

        inline void lock(void) {
          m_semaphore.wait();
        }

        inline void unlock(void) {
          m_semaphore.post();
        }
    };

    //// condition_class definition

    class condition_class
    {
      private:

      // for an explanation of this bloody mess, see ipc_nt_traits.h
      // cary's rant is more than sufficient.

        class WaitingThread
        {
          public:

          // note that the semaphor is being initialized to 0,
          // meaning that a wait() on it will fail until someone
          // else posts to it.

            int semaphore_id;
            int thread_id;
        };

        plist<WaitingThread> waiting_thread_list;
        mutex_class list_access_lock;
        mutex_class* _mutex;

      public:

        static condition_class* const Null;

        INLINE condition_class(mutex_class *m) : _mutex(m) {}
        INLINE ~condition_class(void) {}

        INLINE void wait(void)
        {
          WaitingThread waiting_thread;
          SemaParam sp;

          sp.initCount = 0;
          sp.maxCount = 1;

          waiting_thread.semaphore_id = CreateSema(&sp);
          waiting_thread.thread_id = GetThreadId();

          // get the lock and add the current thread to the list.

          list_access_lock.lock();
          waiting_thread_list.push_back(waiting_thread);
          list_access_lock.unlock();

          // lock this thread away

          _mutex->unlock();
          WaitSema(waiting_thread.semaphore_id);
          _mutex->lock();

          // the thread is now sitting.
          //
          //
          // when it gets here, it will have been released by signal/broadcast

          DeleteSema(waiting_thread.semaphore_id);
        }

        INLINE int timedwait(const unsigned long secs,
                             const unsigned long nsecs) {
          return -1;
        }

        INLINE void signal(void)
        {
          WaitingThread first_in_line = waiting_thread_list.front();
          waiting_thread_list.pop_front();

          // release this node.

          SignalSema(first_in_line.semaphore_id);
        }

        INLINE void broadcast(void)
        {
          plist<WaitingThread>::iterator begin = waiting_thread_list.begin();
          plist<WaitingThread>::iterator end = waiting_thread_list.end();
          plist<WaitingThread>::iterator cur = begin;

          int s_id;

          while (cur != end)
          {
            // touch/release this node

            s_id = (*cur).semaphore_id;
            SignalSema(s_id);

            cur++;
          }

          // and now clear the list out.

          waiting_thread_list.erase(begin, end);
        }
    };

    //// thread_class definition

    class thread_class
    {
      private:

        void *_b_ptr;  // for reverse lookup
        void *(*_fn)(void *);

        struct ThreadParam m_thread_param;
        int m_thread_id;
        int m_priority;

        static const int m_thread_stacksize = 16384;
        char m_thread_stack[m_thread_stacksize] __attribute__ ((aligned(16)));

        INLINE int priority_map(const int pri) {
          switch (pri) {
            case 2:
              return 0;

            case 0:
              return 2;

            case 1:
            default:
              return 1;
          }
        }

        class pointer_lookup
        {
          public:

            int thread_id;
            thread_class *thread_ptr;
        };

        static pvector<pointer_lookup> thread_addr_vector;
        static void thread_wrapper(void *);

      public:

        static thread_class *m_root_thread;

        static void BuildRootThread(void)
        {
          static bool initialized = false;

          // this will run once, and only once.  Config calls it.

          if (initialized == true)
            return;

          // create the new root_thread placeholder

          m_root_thread = new thread_class(NULL);

          m_root_thread->_fn = NULL;
          m_root_thread->m_thread_id = 1;
          m_root_thread->m_priority = 0;

          // now add it to the pointer lookup table

          pointer_lookup root_pl;
          root_pl.thread_id = 1;
          root_pl.thread_ptr = m_root_thread;

          thread_addr_vector.push_back(root_pl);
          initialized = true;
        }

        static thread_class* const Null;

        INLINE thread_class(void *data) : _b_ptr(data) {}

        INLINE ~thread_class(void) {
          cout.flush();
          DeleteThread(GetThreadId());
        }

        INLINE void manual_init(void)
        {
          m_thread_id = GetThreadId();
          set_priority(1);
        }

        INLINE void start_pre(void *(*fn)(void *), const bool, const int pri)
        {
          _fn = fn;
          m_priority = priority_map(pri);

          // set up the thread_param structure

          m_thread_param.entry = thread_wrapper;
          m_thread_param.stack = m_thread_stack;
          m_thread_param.stackSize = m_thread_stacksize;
          m_thread_param.initPriority = m_priority;
          m_thread_param.gpReg = &_gp;

          // write this info back to the cache.

          FlushCache(0);

          // create and register the thread

          m_thread_id = CreateThread(&m_thread_param);

          if (m_thread_id < 0)
            throw thread_fatal(-1);

          pointer_lookup pl;
          pl.thread_id = m_thread_id;
          pl.thread_ptr = this;

          thread_addr_vector.push_back(pl);

          // start the thread

          StartThread(m_thread_id, (void *) this);
        }

        INLINE void start_in(void) {}
        INLINE void start_post(const bool det, const int) {}
        INLINE void *back_ptr(void) { return _b_ptr; }

        static INLINE void exit(void *)
        {
          int id = GetThreadId();

          pvector<pointer_lookup>::iterator cur;

          for (cur = thread_addr_vector.begin(); cur != thread_addr_vector.end();
               cur++)
          {
            if ((*cur).thread_id == id)
            {
              thread_addr_vector.erase(cur);
              break;
            }
          }

          ExitThread();
        }

      // WRITE ME (or maybe ... don't?)

        INLINE void join(void **status) {}

        INLINE void set_priority(const int pri)
        {
          m_priority = priority_map(pri);
          ChangeThreadPriority(m_thread_id, m_priority);
        }

        static INLINE thread_class *self(void)
        {
          int id, result;

          struct ThreadParam thread_param;
          pvector<pointer_lookup>::iterator cur;

          id = GetThreadId();

          for (cur = thread_addr_vector.begin(); cur != thread_addr_vector.end();
               cur++)
          {
            if ((*cur).thread_id == id)
              return (*cur).thread_ptr;
          }

          return NULL;
        }

        static INLINE void yield(void)
        {
          int id, result, priority;
          struct ThreadParam thread_param;

          id = GetThreadId();
          result = ReferThreadStatus(id, &thread_param);

          // make sure this thread isn't thrashed.

          if (result != id)
            throw thread_fatal(-1);

          priority = thread_param.currentPriority;
          RotateThreadReadyQueue(priority);
        }
    };

    //// library_class definition

    class library_class
    {
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

    //// timing stuff

    static INLINE void sleep(const unsigned long secs,
                             const unsigned long nsecs) {
    }

    static INLINE void get_time(unsigned long& abs_secs,
                                unsigned long& abs_nsecs,
                                const unsigned long rel_secs = 0,
                                const unsigned long rel_nsecs = 0) {
    }

    //// generator stuff

    static INLINE mutex_class *make_mutex(void) {
      return new mutex_class;
    }

    static INLINE condition_class *make_condition(mutex_class *m) {
      return new condition_class(m);
    }

    static INLINE semaphore_class *make_semaphore(unsigned int initial) {
      return new semaphore_class(initial);
    }

    static INLINE thread_class *make_thread(void *data) {
      return new thread_class(data);
    }

    static INLINE library_class *make_library(ConfigString& str) {
      return new library_class(str);
    }
};

#endif // __IPC_PS2_TRAITS_H__
