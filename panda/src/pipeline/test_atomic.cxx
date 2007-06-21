// Filename: test_atomic.cxx
// Created by:  drose (19Apr06)
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

#include "pandabase.h"
#include "thread.h"
#include "pmutex.h"
#include "mutexHolder.h"

// The number of threads to spawn.
static const int number_of_threads = 4;

// The number of iterations within each thread.
static const int number_of_iterations = 50000000;

#define OUTPUT(stuff) { \
  MutexHolder holder(Mutex::_notify_mutex); \
  stuff; \
}

PN_int32 _inc_count = 0;
PN_int32 _dec_count = 0;
PN_int32 _net_count = 0;
PN_int32 _num_net_count_incremented = 0;

class MyThread : public Thread {
public:
  MyThread(const string &name) : Thread(name, name)
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
    PT(MyThread) thread = new MyThread(string(1, name));
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
