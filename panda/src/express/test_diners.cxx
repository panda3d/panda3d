// Filename: test_diners.cxx
// Created by:  cary (16Sep98)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

// A solution to the famous dining philosophers, implemented using the
// threading abstraction.  This program exercises thread creation and
// destruction, mutexes, and condition variables.

#include "pandabase.h"
#include "thread.h"
#include "conditionVar.h"
#include "mutexHolder.h"
#include "pointerTo.h"

#ifdef WIN32_VC
static int last_rand = 0;
#endif /* __WIN32__ */

Mutex rand_mutex;

static double random_f(double max)
{
   MutexHolder l(rand_mutex);
   int i = rand();
#ifdef WIN32_VC
   last_rand = i;
#endif /* __WIN32__ */
   return max * (double)i / (double)RAND_MAX;
}

Mutex print_mutex;

#define PRINTMSG(x) { MutexHolder l(print_mutex); x; }

// n philosophers sharing n chopsticks.  Philosophers are poor folk and can't
// afford luxuries like 2 chopsticks per person.
#define N_DINERS 5

Mutex chopsticks[N_DINERS];

// At most n philosophers are allowed into the room, others would have to
// wait at the door.  This restriction demonstrates the use of condition
// variables.

Mutex room_mutex;

ConditionVar room_condition(room_mutex);
int room_occupancy = 0;

class philosopher;
PT(philosopher) phils[N_DINERS];

class philosopher : public Thread {
   private:
     int _id;
     void thread_main() {
#ifdef WIN32_VC
         rand_mutex.lock();
         srand(last_rand);
         rand_mutex.release();
#endif /* __WIN32__ */
         int l = _id;
         int r = l+1;
         if (r == N_DINERS)
            r = 0;
         if (l & 1) {
            int t = l;
            l = r;
            r = t;
         }
         PRINTMSG(cerr << "Philosopher #" << _id << " has entered the room."
                  << endl);
         int count = (int)random_f(10.0) + 1;
         while (--count) {
            chopsticks[l].lock();
            chopsticks[r].lock();
            PRINTMSG(cerr << "Philosopher #" << _id
                     << " is eating spaghetti now." << endl);
            Thread::sleep(random_f(3.0));
            chopsticks[l].release();
            chopsticks[r].release();
            PRINTMSG(cerr << "Philosopher #" << _id
                     << " is pondering about life." << endl);
            Thread::sleep(random_f(3.0));
         }
         room_mutex.lock();
         --room_occupancy;
         phils[_id] = (philosopher*)0L;
         room_condition.signal();
         room_mutex.release();
         PRINTMSG(cerr << "Philosopher #" << _id << " has left the room ("
                  << room_occupancy << " left)." << endl);
      }

      inline void* make_arg(const int i) { return (void*)new int(i); }
   public:
      philosopher(const int id) : Thread("philosopher") {
        _id = id;
      }
};

int main(int, char**)
{
   int i;
   room_mutex.lock();
   for (i=0; i<N_DINERS; ++i) {
      phils[i] = new philosopher(i);
      phils[i]->start(TP_normal, false, false);
   }
   room_occupancy = N_DINERS;
   while (1) {
      while (room_occupancy == N_DINERS) {
         PRINTMSG(cerr << "main thread about to block " << room_occupancy
                  << endl);
         room_condition.wait();
      }
      // hmm.. someone left the room.
      room_mutex.release();
      // sleep for a while and then create a new philosopher
      PRINTMSG(cerr << "main thread sleep" << endl);
      Thread::sleep(2.0);
      PRINTMSG(cerr << "main thread wake up" << endl);
      room_mutex.lock();
      for (i=0; i<N_DINERS; ++i)
         if (phils[i] == (philosopher*)0L)
            break;
      if (i == N_DINERS) {
         PRINTMSG(cerr
                  << "Contrary to what I was tolk, no one has left the room!!!"
                  << endl);
         PRINTMSG(cerr << "I give up!" << endl);
         Thread::prepare_for_exit();
         exit(1);
      }
      phils[i] = new philosopher(i);
      ++room_occupancy;
   }
}
