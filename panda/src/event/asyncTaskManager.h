/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file asyncTaskManager.h
 * @author drose
 * @date 2006-08-23
 */

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
#include "conditionVar.h"
#include "pvector.h"
#include "pdeque.h"
#include "pStatCollector.h"
#include "clockObject.h"
#include "ordered_vector.h"
#include "indirectCompareNames.h"

/**
 * A class to manage a loose queue of isolated tasks, which can be performed
 * either synchronously (in the foreground thread) or asynchronously (by a
 * background thread).
 *
 * The AsyncTaskManager is actually a collection of AsyncTaskChains, each of
 * which maintains a list of tasks.  Each chain can be either foreground or
 * background (it may run only in the main thread, or it may be serviced by
 * one or more background threads). See AsyncTaskChain for more information.
 *
 * If you do not require background processing, it is perfectly acceptable to
 * create only one AsyncTaskChain, which runs in the main thread.  This is a
 * common configuration.
 */
class EXPCL_PANDA_EVENT AsyncTaskManager : public TypedReferenceCount, public Namable {
PUBLISHED:
  explicit AsyncTaskManager(const std::string &name);
  BLOCKING virtual ~AsyncTaskManager();

  BLOCKING void cleanup();

  INLINE void set_clock(ClockObject *clock);
  INLINE ClockObject *get_clock();
  MAKE_PROPERTY(clock, get_clock, set_clock);

  int get_num_task_chains() const;
  AsyncTaskChain *get_task_chain(int n) const;
  MAKE_SEQ(get_task_chains, get_num_task_chains, get_task_chain);
  AsyncTaskChain *make_task_chain(const std::string &name);
  AsyncTaskChain *find_task_chain(const std::string &name);
  BLOCKING bool remove_task_chain(const std::string &name);

  void add(AsyncTask *task);
  bool has_task(AsyncTask *task) const;

  AsyncTask *find_task(const std::string &name) const;
  AsyncTaskCollection find_tasks(const std::string &name) const;
  AsyncTaskCollection find_tasks_matching(const GlobPattern &pattern) const;

  bool remove(AsyncTask *task);
  size_t remove(const AsyncTaskCollection &tasks);

  BLOCKING void wait_for_tasks();
  BLOCKING void stop_threads();
  void start_threads();

  INLINE size_t get_num_tasks() const;

  AsyncTaskCollection get_tasks() const;
  AsyncTaskCollection get_active_tasks() const;
  AsyncTaskCollection get_sleeping_tasks() const;
  MAKE_PROPERTY(tasks, get_tasks);
  MAKE_PROPERTY(active_tasks, get_active_tasks);
  MAKE_PROPERTY(sleeping_tasks, get_sleeping_tasks);

  void poll();
  double get_next_wake_time() const;
  MAKE_PROPERTY(next_wake_time, get_next_wake_time);

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level = 0) const;

  INLINE static AsyncTaskManager *get_global_ptr();

protected:
  AsyncTaskChain *do_make_task_chain(const std::string &name);
  AsyncTaskChain *do_find_task_chain(const std::string &name);

  INLINE void add_task_by_name(AsyncTask *task);
  void remove_task_by_name(AsyncTask *task);

  bool do_has_task(AsyncTask *task) const;

  virtual void do_output(std::ostream &out) const;

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

  // Protects all the following members.  This same lock is also used to
  // protect all of our AsyncTaskChain members.
  Mutex _lock;

  typedef ov_set<PT(AsyncTaskChain), IndirectCompareNames<AsyncTaskChain> > TaskChains;
  TaskChains _task_chains;

  size_t _num_tasks;
  TasksByName _tasks_by_name;
  PT(ClockObject) _clock;

  ConditionVar _frame_cvar;  // Signalled when the clock ticks.

  static AsyncTaskManager* _global_ptr;

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

  friend class AsyncFuture;
  friend class AsyncTaskChain;
  friend class AsyncTaskChain::AsyncTaskChainThread;
  friend class AsyncTask;
  friend class AsyncTaskSequence;
  friend class PythonTask;
};

INLINE std::ostream &operator << (std::ostream &out, const AsyncTaskManager &manager) {
  manager.output(out);
  return out;
};

#include "asyncTaskManager.I"

#endif
