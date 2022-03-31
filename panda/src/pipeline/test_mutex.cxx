/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_mutex.cxx
 * @author drose
 * @date 2006-03-29
 */

#include "pandabase.h"
#include "thread.h"
#include "pmutex.h"
#include "mutexHolder.h"
#include "pointerTo.h"
#include "trueClock.h"

static const double thread_duration = 5.0;

class MyThread : public Thread {
public:
  MyThread(const std::string &name, MutexImpl &m1, double period) :
    Thread(name, name),
    _m1(m1), _period(period)
  {
  }

  virtual void thread_main() {
    TrueClock *clock = TrueClock::get_global_ptr();
    double start = clock->get_short_time();
    double end = start + thread_duration;
    while (clock->get_short_time() < end) {
      _m1.lock();
      Thread::sleep(_period);
      _m1.unlock();
    }
  }

  MutexImpl &_m1;
  double _period;
};

int
main(int argc, char *argv[]) {
  MutexImpl _m1;

  _m1.lock();
  _m1.unlock();

  std::cerr << "Making threads.\n";
  MyThread *a = new MyThread("a", _m1, 1.0);
  MyThread *b = new MyThread("b", _m1, 0.9);

  std::cerr << "Starting threads.\n";
  a->start(TP_normal, true);
  b->start(TP_normal, true);

  a->join();
  b->join();

  exit(0);
}
