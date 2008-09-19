// Filename: asyncTaskManager.h
// Created by:  drose (23Aug06)
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

#ifndef ASYNCTASKMANAGER_H
#define ASYNCTASKMANAGER_H

#include "pandabase.h"

#include "asyncTask.h"
#include "asyncTaskCollection.h"
#include "typedReferenceCount.h"
#include "thread.h"
#include "pmutex.h"
#include "conditionVarFull.h"
#include "pvector.h"
#include "pdeque.h"
#include "pStatCollector.h"
#include "clockObject.h"

////////////////////////////////////////////////////////////////////
//       Class : AsyncTaskManager
// Description : A class to manage a loose queue of isolated tasks,
//               which can be performed by a background thread (in
//               particular, for instance, loading a model file).
//
//               The AsyncTaskManager will spawn a specified number of
//               threads (possibly 0) to serve the tasks.  If there
//               are no threads, you must call poll() from time to
//               time to serve the tasks in the main thread.
//
//               Each task will run exactly once each epoch.  Beyond
//               that, the tasks' sort and priority values control the
//               order in which they are run: tasks are run in
//               increasing order by sort value, and within the same
//               sort value, they are run roughly in decreasing order
//               by priority value, with some exceptions for
//               parallelism.  Tasks with different sort values are
//               never run in parallel together, but tasks with
//               different priority values might be (if there is more
//               than one thread).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_EVENT AsyncTaskManager : public TypedReferenceCount, public Namable {
PUBLISHED:
  AsyncTaskManager(const string &name, int num_threads = 0);
  BLOCKING virtual ~AsyncTaskManager();

  INLINE void set_clock(ClockObject *clock);
  INLINE ClockObject *get_clock();

  INLINE void set_tick_clock(bool tick_clock);
  INLINE bool get_tick_clock() const;

  BLOCKING void set_num_threads(int num_threads);
  INLINE int get_num_threads() const;
  INLINE int get_num_running_threads() const;

  BLOCKING void stop_threads();
  void start_threads();
  INLINE bool is_started() const;

  void add(AsyncTask *task);
  bool has_task(AsyncTask *task) const;

  AsyncTask *find_task(const string &name) const;
  AsyncTaskCollection find_tasks(const string &name) const;
  AsyncTaskCollection find_tasks_matching(const GlobPattern &pattern) const;

  bool remove(AsyncTask *task);
  int remove(const AsyncTaskCollection &tasks);

  BLOCKING void wait_for_tasks();

  INLINE int get_num_tasks() const;

  AsyncTaskCollection get_tasks();
  AsyncTaskCollection get_active_tasks();
  AsyncTaskCollection get_sleeping_tasks();

  void poll();

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  class AsyncTaskManagerThread;
  typedef pvector< PT(AsyncTask) > TaskHeap;

  bool do_has_task(AsyncTask *task) const;
  int find_task_on_heap(const TaskHeap &heap, AsyncTask *task) const;

  INLINE void add_task_by_name(AsyncTask *task);
  void remove_task_by_name(AsyncTask *task);

  void service_one_task(AsyncTaskManagerThread *thread);
  void cleanup_task(AsyncTask *task, bool clean_exit);
  bool finish_sort_group();
  void do_stop_threads();
  void do_start_threads();
  void do_poll();

protected:
  class AsyncTaskManagerThread : public Thread {
  public:
    AsyncTaskManagerThread(const string &name, AsyncTaskManager *manager);
    virtual void thread_main();

    AsyncTaskManager *_manager;
    AsyncTask *_servicing;
  };

  class AsyncTaskSortWakeTime {
  public:
    bool operator () (AsyncTask *a, AsyncTask *b) const {
      return a->get_wake_time() > b->get_wake_time();
    }
  };
  
  class AsyncTaskSortPriority {
  public:
    bool operator () (AsyncTask *a, AsyncTask *b) const {
      if (a->get_sort() != b->get_sort()) {
        return a->get_sort() > b->get_sort();
      }
      return a->get_priority() < b->get_priority();
    }
  };
  
  class AsyncTaskSortName {
  public:
    bool operator () (AsyncTask *a, AsyncTask *b) const {
      return a->get_name() < b->get_name();
    }
  };

  typedef pvector< PT(AsyncTaskManagerThread) > Threads;
  typedef pmultiset<AsyncTask *, AsyncTaskSortName> TasksByName;

  int _num_threads;

  Mutex _lock;  // Protects all the following members.
  ConditionVarFull _cvar;  // signaled when _active, _next_active, _sleeping, _state, or _current_sort changes, or a task finishes.

  enum State {
    S_initial,  // no threads yet
    S_started,  // threads have been started
    S_aborting, // task returned DS_abort, shutdown requested from sub-thread.
    S_shutdown  // waiting for thread shutdown, requested from main thread
  };

  Threads _threads;
  int _num_tasks;
  int _num_busy_threads;
  TaskHeap _active;
  TaskHeap _next_active;
  TaskHeap _sleeping;
  TasksByName _tasks_by_name;
  State _state;
  int _current_sort;
  PT(ClockObject) _clock;
  bool _tick_clock;
  
  static PStatCollector _task_pcollector;
  static PStatCollector _wait_pcollector;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "AsyncTaskManager",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class AsyncTaskManagerThread;
  friend class AsyncTask;
};

#include "asyncTaskManager.I"

#endif
