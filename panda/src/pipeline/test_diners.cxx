/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_diners.cxx
 * @author cary
 * @date 1998-09-16
 */

// A solution to the famous dining philosophers, implemented using the
// threading abstraction.  This program exercises thread creation and
// destruction, mutexes, and condition variables.

#include "pandabase.h"
#include "thread.h"
#include "conditionVar.h"
#include "mutexHolder.h"
#include "pointerTo.h"
#include "referenceCount.h"
#include "trueClock.h"
#include "pstrtod.h"

using std::cerr;

#ifdef WIN32_VC
// Under Windows, the rand() function seems to return a sequence per-thread,
// so we use this trick to set each thread to a different seed.
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

#define PRINTMSG(x) { MutexHolder l(Mutex::_notify_mutex); x << std::flush; }

// n philosophers sharing n chopsticks.  Philosophers are poor folk and can't
// afford luxuries like 2 chopsticks per person.
#define N_DINERS 5

class ChopstickMutex : public Mutex {
public:
  void output(std::ostream &out) const {
    out << "chopstick " << _n;
  }
  int _n;
};


ChopstickMutex chopsticks[N_DINERS];

// At most n philosophers are allowed into the room, others would have to wait
// at the door.  This restriction demonstrates the use of condition variables.

Mutex room_mutex;

ConditionVar room_condition(room_mutex);
int room_occupancy = 0;

class philosopher;
PT(Thread) phils[N_DINERS];

class philosopher : public Thread {
private:
  int _id;
  void thread_main() {
#ifdef WIN32_VC
    rand_mutex.acquire();
    srand(last_rand);
    rand_mutex.release();
    random_f(1.0);
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
             << "\n");
    int count = (int)random_f(10.0) + 1;
    while (--count) {
      chopsticks[r].acquire();
      Thread::sleep(1);
      chopsticks[l].acquire();
      PRINTMSG(cerr << "Philosopher #" << _id
               << " is eating spaghetti now.\n");
      Thread::sleep(random_f(3.0));
      chopsticks[l].release();
      chopsticks[r].release();
      PRINTMSG(cerr << "Philosopher #" << _id
               << " is pondering about life.\n");
      Thread::sleep(random_f(3.0));
    }
    room_mutex.acquire();
    --room_occupancy;
    cerr << "clearing philosopher " << _id << "\n";
    phils[_id] = (philosopher*)0L;
    room_condition.notify();
    room_mutex.release();
    PRINTMSG(cerr << "Philosopher #" << _id << " has left the room ("
             << room_occupancy << " left).\n");
  }

  inline void* make_arg(const int i) { return (void*)new int(i); }
public:
  philosopher(const int id) : Thread("philosopher", "a") {
    _id = id;
  }

  virtual void output(std::ostream &out) const {
    out << "philosopher " << _id;
  }
};

int
main(int argc, char *argv[]) {
  double has_run_time = false;
  double run_time = 0.0;
  if (argc > 1) {
    run_time = patof(argv[1]);
    cerr << "Running for " << run_time << " seconds\n";
    has_run_time = true;
  } else {
    cerr << "Running indefinitely\n";
  }

  int i;
  room_mutex.acquire();
  for (i=0; i<N_DINERS; ++i) {
    chopsticks[i]._n = i;
  }
  for (i=0; i<N_DINERS; ++i) {
    phils[i] = new philosopher(i);
    phils[i]->start(TP_normal, false);
  }
  room_occupancy = N_DINERS;

  TrueClock *clock = TrueClock::get_global_ptr();
  double start_time = clock->get_short_time();
  double end_time = start_time + run_time;

  while (!has_run_time || clock->get_short_time() < end_time) {
    if (room_occupancy == N_DINERS) {
      PRINTMSG(cerr << "main thread about to block " << room_occupancy
               << "\n");
      while (room_occupancy == N_DINERS) {
        room_condition.wait();
      }
    }
    // hmm.. someone left the room.
    room_mutex.release();
    // sleep for a while and then create a new philosopher
    PRINTMSG(cerr << "main thread sleep\n");
    Thread::sleep(2.0);
    PRINTMSG(cerr << "main thread wake up\n");
    room_mutex.acquire();
    for (i=0; i<N_DINERS; ++i)
      if (phils[i] == (philosopher*)0L)
        break;
    assert(i != N_DINERS);
    phils[i] = new philosopher(i);
    phils[i]->start(TP_normal, false);
    ++room_occupancy;
  }

  PRINTMSG(cerr << "Waiting for philosophers to finish\n");

  while (room_occupancy != 0) {
    room_condition.wait();
  }

  room_mutex.release();

  cerr << "Exiting.\n";
  return 0;
}
