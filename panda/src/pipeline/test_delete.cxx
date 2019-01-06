/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_delete.cxx
 * @author drose
 * @date 2006-04-18
 */

#include "pandabase.h"
#include "thread.h"
#include "pmutex.h"
#include "mutexHolder.h"
#include "deletedChain.h"

// This is the number of passes to make per thread.
static const int num_passes = 100;

// This is the number of doobers to allocate in each pass.
static const int num_doobers_per_pass = 1000000;

// This is the max number to allocate in each chunk.
static const int max_doobers_per_chunk = 1000;

// The number of threads to spawn.
static const int number_of_threads = 4;

#if defined(WIN32_VC) || defined(WIN64_VC)
static int last_rand = 0;
#endif /* __WIN32__ */

Mutex rand_mutex;

static double
random_f(double max) {
  MutexHolder l(rand_mutex);
  int i = rand();
#if defined(WIN32_VC) || defined(WIN64_VC)
  last_rand = i;
#endif /* __WIN32__ */
  return max * (double)i / (double)RAND_MAX;
}

#define OUTPUT(stuff) { \
  MutexHolder holder(Mutex::_notify_mutex); \
  stuff; \
}

class Doober {
public:
  Doober(int counter) : _counter(counter) {
  }
  ~Doober() {
    nassertv(_counter != -1);
    _counter = -1;
  }
  ALLOC_DELETED_CHAIN(Doober);

  int _counter;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "Doober");
  }

private:
  static TypeHandle _type_handle;
};
typedef pvector<Doober *> Doobers;

TypeHandle Doober::_type_handle;


class MyThread : public Thread {
public:
  MyThread(const std::string &name) : Thread(name, name)
  {
  }

  virtual void
  thread_main() {
    OUTPUT(nout << *this << " beginning.\n");

#if defined(WIN32_VC) || defined(WIN64_VC)
    rand_mutex.acquire();
    srand(last_rand);
    rand_mutex.release();
    random_f(1.0);
#endif /* __WIN32__ */

    Doobers doobers;

    for (int i = 0; i < num_passes; ++i) {
      int counter = 0;

      while (counter < num_doobers_per_pass) {
        int num_alloc = (int)random_f(max_doobers_per_chunk);
        for (int i = 0; i < num_alloc; ++i) {
          doobers.push_back(new Doober(++counter));
        }
        int num_del = (int)random_f(max_doobers_per_chunk);
        num_del = std::min(num_del, (int)doobers.size());

        for (int j = 0; j < num_del; ++j) {
          assert(!doobers.empty());
          delete doobers.back();
          doobers.pop_back();
        }
      }
      OUTPUT(nout << get_name() << " has " << doobers.size() << " doobers.\n");
    }

    OUTPUT(nout << *this << " ending.\n");
  }

};

int
main(int argc, char *argv[]) {
  Doober::init_type();

  OUTPUT(nout << "Making " << number_of_threads << " threads.\n");

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

  Thread::prepare_for_exit();
  return (0);
}
