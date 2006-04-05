// Filename: test_mutex.cxx
// Created by:  drose (29Mar06)
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
#include "pointerTo.h"
#include "trueClock.h"

static const double thread_duration = 5.0;

class MyThread : public Thread {
public:
  MyThread(const string &name, MutexImpl &m1, double period) : 
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
      _m1.release();
    }
  }

  MutexImpl &_m1;
  double _period;
};

int
main(int argc, char *argv[]) {
  MutexImpl _m1;

  _m1.lock();
  _m1.release();

  cerr << "Making threads.\n";
  MyThread *a = new MyThread("a", _m1, 1.0);
  MyThread *b = new MyThread("b", _m1, 0.9);

  cerr << "Starting threads.\n";
  a->start(TP_normal, true, true);
  b->start(TP_normal, true, true);

  a->join();
  b->join();

  exit(0);
}
