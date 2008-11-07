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
#include "asyncTaskChain.h"
#include "typedReferenceCount.h"
#include "thread.h"
#include "pmutex.h"
#include "mutexHolder.h"
#include "conditionVarFull.h"
#include "pvector.h"
#include "pdeque.h"
#include "pStatCollector.h"
#include "clockObject.h"
#include "ordered_vector.h"
#include "indirectCompareNames.h"

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
  AsyncTaskManager(const string &name);
  BLOCKING virtual ~AsyncTaskManager();

  BLOCKING void cleanup();

  INLINE void set_clock(ClockObject *clock);
  INLINE ClockObject *get_clock();

  int get_num_task_chains() const;
  AsyncTaskChain *get_task_chain(int n) const;
  MAKE_SEQ(get_task_chains, get_num_task_chains, get_task_chain);
  AsyncTaskChain *make_task_chain(const string &name);
  AsyncTaskChain *find_task_chain(const string &name);
  BLOCKING bool remove_task_chain(const string &name);

  void add(AsyncTask *task);
  bool has_task(AsyncTask *task) const;

  AsyncTask *find_task(const string &name) const;
  AsyncTaskCollection find_tasks(const string &name) const;
  AsyncTaskCollection find_tasks_matching(const GlobPattern &pattern) const;

  bool remove(AsyncTask *task);
  int remove(const AsyncTaskCollection &tasks);

  BLOCKING void wait_for_tasks();
  BLOCKING void stop_threads();
  void start_threads();

  INLINE int get_num_tasks() const;

  AsyncTaskCollection get_tasks() const;
  AsyncTaskCollection get_active_tasks() const;
  AsyncTaskCollection get_sleeping_tasks() const;

  void poll();
  double get_next_wake_time() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

  INLINE static AsyncTaskManager *get_global_ptr();

protected:
  AsyncTaskChain *do_make_task_chain(const string &name);
  AsyncTaskChain *do_find_task_chain(const string &name);

  INLINE void add_task_by_name(AsyncTask *task);
  void remove_task_by_name(AsyncTask *task);

  bool do_has_task(AsyncTask *task) const;

  virtual void do_output(ostream &out) const;

private:
  static void make_global_ptr();

protected:
  class AsyncTaskSortName {
  public:
    bool operator () (AsyncTask *a, AsyncTask *b) const {
      return a->get_name() < b->get_name();
    }
  };

  typedef pmultiset<AsyncTask *, AsyncTaskSortName> TasksByName;

  // Protects all the following members.  This same lock is also used
  // to protect all of our AsyncTaskChain members.
  Mutex _lock; 

  typedef ov_set<PT(AsyncTaskChain), IndirectCompareNames<AsyncTaskChain> > TaskChains;
  TaskChains _task_chains;

  int _num_tasks;
  TasksByName _tasks_by_name;
  PT(ClockObject) _clock;
  
  ConditionVarFull _frame_cvar;  // Signalled when the clock ticks.

  static PT(AsyncTaskManager) _global_ptr;

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

  friend class AsyncTaskChain;
  friend class AsyncTaskChain::AsyncTaskChainThread;
  friend class AsyncTask;
  friend class AsyncTaskSequence;
};

INLINE ostream &operator << (ostream &out, const AsyncTaskManager &manager) {
  manager.output(out);
  return out;
};

#include "asyncTaskManager.I"

#endif
