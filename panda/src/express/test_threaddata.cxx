// Filename: test_threaddata.cxx
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

#include "pandabase.h"
#include "thread.h"
#include "pmutex.h"
#include "mutexHolder.h"
#include "pointerTo.h"

Mutex *cout_mutex = (Mutex *)NULL;

// Test forking a thread with some private data.
class ThreadWithData : public Thread {
public:
  ThreadWithData(const string &name, int parameter);

  virtual void thread_main();

private:
  int _parameter;
};


ThreadWithData::
ThreadWithData(const string &name, int parameter) : 
  Thread(name),
  _parameter(parameter)
{
  MutexHolder holder(cout_mutex);
  cout << "Creating thread " << get_name() << " with parameter " << _parameter
       << "\n";
}

void ThreadWithData::
thread_main() {
  for (int i = 0; i < _parameter; i++) {
    {
      MutexHolder holder(cout_mutex);
      cout << "Running thread " << get_name()
           << " with parameter " << _parameter
           << ", i = " << i << "\n" << flush;
      Thread *thread = get_current_thread();
      nassertv(thread == this);
    }
    Thread::sleep((double) i / 10.0);
  }
}

int
main() {
  cout << "main beginning.\n";
  for (int i = 0; i < 10; i++) {
    string name = string("thread_") + (char)(i + 'a');
    PT(Thread) thread = new ThreadWithData(name, i);
    if (!thread->start(TP_low, false, false)) {
      MutexHolder holder(cout_mutex);
      cout << "Unable to start " << name << ".\n";
    } else {
      MutexHolder holder(cout_mutex);
      cout << "Started " << name << ", count = " 
           << thread->get_ref_count() << "\n";
    }
  }

  {
    MutexHolder holder(cout_mutex);
    cout << "main preparing to exit.\n";
  }

  Thread::prepare_for_exit();

  cout << "main exiting.\n";

  return 0;
}
