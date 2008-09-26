// Filename: test_task.cxx
// Created by:  drose (16Sep08)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "pandabase.h"
#include "asyncTask.h"
#include "asyncTaskManager.h"
#include "perlinNoise2.h"

class MyTask : public AsyncTask {
public:
  MyTask(const string &name, double length, int repeat_count) :
    AsyncTask(name),
    _length(length),
    _repeat_count(repeat_count)
  {
  }
  ALLOC_DELETED_CHAIN(MyTask);
    
  virtual DoneStatus do_task() {
    cerr << "Doing " << *this << ", sort = " << get_sort()
         << ", priority = " << get_priority()
         << ": " << *Thread::get_current_thread() << "\n";
    Thread::sleep(_length);
    cerr << "Done " << *this << ": " << *Thread::get_current_thread() << "\n";
    --_repeat_count;
    if (_repeat_count > 0) {
      return DS_cont;
    }
    return DS_done;
  }

  double _length;
  int _repeat_count;
};

static const int grid_size = 10;
static const int num_threads = 10;

int
main(int argc, char *argv[]) {
  PT(AsyncTaskManager) task_mgr = new AsyncTaskManager("task_mgr");
  PT(AsyncTaskChain) chain = task_mgr->make_task_chain("default");
  chain->set_tick_clock(true);
  chain->set_num_threads(num_threads);

  PerlinNoise2 length_noise(grid_size, grid_size);
  PerlinNoise2 delay_noise(grid_size, grid_size);
  PerlinNoise2 repeat_count_noise(grid_size, grid_size);
  PerlinNoise2 sort_noise(grid_size, grid_size);
  PerlinNoise2 priority_noise(grid_size, grid_size);

  cerr << "Making tasks.\n";
  for (int yi = 0; yi < grid_size; ++yi) {
    for (int xi = 0; xi < grid_size; ++xi) {
      ostringstream namestrm;
      namestrm << "task_" << xi << "_" << yi;

      double length = max(length_noise.noise(xi, yi) + 1.0, 0.0);
      double delay = max(delay_noise.noise(xi, yi), 0.0) * 3.0;
      int repeat_count = (int)floor(max(repeat_count_noise.noise(xi, yi) + 1.0, 0.0) * 1.5);
      int sort = (int)floor(sort_noise.noise(xi, yi) * 2.0);
      int priority = (int)floor(priority_noise.noise(xi, yi) * 5.0);

      PT(MyTask) task = new MyTask(namestrm.str(), length, repeat_count);
      if (delay > 0.0) {
        task->set_delay(delay);
      }
      task->set_sort(sort);
      task->set_priority(priority);

      cerr << *task << ", delay = " << delay << ", length = " << length << "\n";
      task_mgr->add(task);
    }
  }

  task_mgr->wait_for_tasks();

  exit(0);
}
