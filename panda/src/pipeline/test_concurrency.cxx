/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_concurrency.cxx
 * @author drose
 * @date 2006-04-06
 */

#include "pandabase.h"
#include "thread.h"
#include "pmutex.h"
#include "mutexHolder.h"
#include "pointerTo.h"
#include "trueClock.h"

// The number of iterations to spin within each thread, before printing
// output.
static const long long iterations_per_output = 100000000;

// The amount of time, in seconds, to wait between spawning threads.
static const double delay_between_threads = 2.0;

// The amount of time, in seconds, for each thread to run.
static const double thread_run_time = 15.0;

// The number of threads to spawn.
static const int number_of_threads = 4;

// Number of bytes to reserve between writable memory pointers.
static const int block_size = 64;

static Mutex _output_lock;

union MemBlock {
  char _pad[block_size];
  int _value;
};

volatile MemBlock memblock[number_of_threads];

#define OUTPUT(stuff) { \
  MutexHolder holder(_output_lock); \
  stuff; \
}

class MyThread : public Thread {
public:
  MyThread(const std::string &name, int index) :
    Thread(name, name),
    _index(index)
  {
  }

  virtual void thread_main() {
    OUTPUT(nout << *this << " beginning.\n");

    double total_seconds = 0.0;
    TrueClock *clock = TrueClock::get_global_ptr();
    volatile int snarf;

    while (total_seconds < thread_run_time) {
      double start_time = clock->get_short_time();

      for (long long i = 0; i < iterations_per_output; ++i) {
        memblock[_index]._value = snarf;
      }

      double end_time = clock->get_short_time();

      double elapsed_seconds = end_time - start_time;
      double iterations_per_second = iterations_per_output / elapsed_seconds;
      OUTPUT(nout << *this << " achieving "
             << iterations_per_second / 1000000.0
             << " million iterations per second.\n");

      total_seconds += elapsed_seconds;
    }

    OUTPUT(nout << *this << " exiting.\n");
  }

  int _index;
};

int
main(int argc, char *argv[]) {
  OUTPUT(nout << "Making " << number_of_threads << " threads.\n");

  typedef pvector< PT(MyThread) > Threads;
  Threads threads;

  PT(MyThread) thread = new MyThread("a", 0);
  threads.push_back(thread);
  thread->start(TP_normal, true);

  for (int i = 1; i < number_of_threads; ++i) {
    char name = 'a' + i;
    Thread::sleep(delay_between_threads);
    PT(MyThread) thread = new MyThread(std::string(1, name), i);
    threads.push_back(thread);
    thread->start(TP_normal, true);
  }

  // Now join all the threads.
  Threads::iterator ti;
  for (ti = threads.begin(); ti != threads.end(); ++ti) {
    (*ti)->join();
  }

  Thread::prepare_for_exit();
  return 0;
}
