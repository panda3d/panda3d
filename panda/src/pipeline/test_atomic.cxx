/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_atomic.cxx
 * @author drose
 * @date 2006-04-19
 */

#include "pandabase.h"
#include "thread.h"
#include "pmutex.h"
#include "mutexHolder.h"
#include "atomicAdjust.h"

// The number of threads to spawn.
static const int number_of_threads = 4;

// The number of iterations within each thread.
static const int number_of_iterations = 50000000;

#define OUTPUT(stuff) { \
  MutexHolder holder(Mutex::_notify_mutex); \
  stuff; \
}

AtomicAdjust::Integer _inc_count = 0;
AtomicAdjust::Integer _dec_count = 0;
AtomicAdjust::Integer _net_count = 0;
AtomicAdjust::Integer _num_net_count_incremented = 0;

class MyThread : public Thread {
public:
  MyThread(const std::string &name) : Thread(name, name)
  {
  }

  virtual void
  thread_main() {
    OUTPUT(nout << *this << " beginning.\n");

    int local_count = 0;

    for (int i = 0; i < number_of_iterations; ++i) {
      AtomicAdjust::inc(_inc_count);
      AtomicAdjust::dec(_dec_count);
      if (AtomicAdjust::compare_and_exchange(_net_count, i, i + 1) == i) {
        AtomicAdjust::inc(_num_net_count_incremented);
        ++local_count;
      }
    }

    OUTPUT(nout << *this << " contributed " << local_count << " times.\n");
    OUTPUT(nout << *this << " ending.\n");
  }

};

int
main(int argc, char *argv[]) {
  nout << "Making " << number_of_threads << " threads.\n";

  typedef pvector< PT(MyThread) > Threads;
  Threads threads;

  PT(MyThread) thread = new MyThread("a");
  threads.push_back(thread);
  thread->start(TP_normal, true);

  for (int i = 1; i < number_of_threads; ++i) {
    char name = 'a' + i;
    PT(MyThread) thread = new MyThread(std::string(1, name));
    threads.push_back(thread);
    thread->start(TP_normal, true);
  }

  // Now join all the threads.
  Threads::iterator ti;
  for (ti = threads.begin(); ti != threads.end(); ++ti) {
    (*ti)->join();
  }

  nout << "inc_count = " << _inc_count << "\n"
       << "dec_count = " << _dec_count << "\n"
       << "net_count = " << _net_count << "\n"
       << "num_net_count_incremented = " << _num_net_count_incremented << "\n";

  Thread::prepare_for_exit();
  return (0);
}
