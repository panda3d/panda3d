/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file asyncTaskChain.h
 * @author drose
 * @date 2006-08-23
 */

#ifndef ASYNCTASKCHAIN_H
#define ASYNCTASKCHAIN_H

#include "pandabase.h"

#include "asyncTask.h"
#include "asyncTaskCollection.h"
#include "typedReferenceCount.h"
#include "thread.h"
#include "conditionVar.h"
#include "pvector.h"
#include "pdeque.h"
#include "pStatCollector.h"
#include "clockObject.h"

class AsyncTaskManager;

/**
 * The AsyncTaskChain is a subset of the AsyncTaskManager.  Each chain
 * maintains a separate list of tasks, and will execute them with its own set
 * of threads.  Each chain may thereby operate independently of the other
 * chains.
 *
 * The AsyncTaskChain will spawn a specified number of threads (possibly 0) to
 * serve the tasks.  If there are no threads, you must call poll() from time
 * to time to serve the tasks in the main thread.  Normally this is done by
 * calling AsyncTaskManager::poll().
 *
 * Each task will run exactly once each epoch.  Beyond that, the tasks' sort
 * and priority values control the order in which they are run: tasks are run
 * in increasing order by sort value, and within the same sort value, they are
 * run roughly in decreasing order by priority value, with some exceptions for
 * parallelism.  Tasks with different sort values are never run in parallel
 * together, but tasks with different priority values might be (if there is
 * more than one thread).
 */
class EXPCL_PANDA_EVENT AsyncTaskChain : public TypedReferenceCount, public Namable {
public:
  AsyncTaskChain(AsyncTaskManager *manager, const std::string &name);
  ~AsyncTaskChain();

PUBLISHED:
  void set_tick_clock(bool tick_clock);
  bool get_tick_clock() const;

  BLOCKING void set_num_threads(int num_threads);
  int get_num_threads() const;
  int get_num_running_threads() const;

  BLOCKING void set_thread_priority(ThreadPriority priority);
  ThreadPriority get_thread_priority() const;

  void set_frame_budget(double frame_budget);
  double get_frame_budget() const;

  void set_frame_sync(bool frame_sync);
  bool get_frame_sync() const;

  void set_timeslice_priority(bool timeslice_priority);
  bool get_timeslice_priority() const;

  BLOCKING void stop_threads();
  void start_threads();
  INLINE bool is_started() const;

  bool has_task(AsyncTask *task) const;

  BLOCKING void wait_for_tasks();

  int get_num_tasks() const;
  AsyncTaskCollection get_tasks() const;
  AsyncTaskCollection get_active_tasks() const;
  AsyncTaskCollection get_sleeping_tasks() const;

  void poll();
  double get_next_wake_time() const;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level = 0) const;

protected:
  class AsyncTaskChainThread;
  typedef pvector< PT(AsyncTask) > TaskHeap;

  void do_add(AsyncTask *task);
  bool do_remove(AsyncTask *task, bool upon_death=false);
  void do_wait_for_tasks();
  void do_cleanup();

  bool do_has_task(AsyncTask *task) const;
  int find_task_on_heap(const TaskHeap &heap, AsyncTask *task) const;

  void service_one_task(AsyncTaskChainThread *thread);
  void cleanup_task(AsyncTask *task, bool upon_death, bool clean_exit);
  bool finish_sort_group();
  void filter_timeslice_priority();
  void do_stop_threads();
  void do_start_threads();
  AsyncTaskCollection do_get_active_tasks() const;
  AsyncTaskCollection do_get_sleeping_tasks() const;
  void do_poll();
  void cleanup_pickup_mode();
  INLINE double do_get_next_wake_time() const;
  static INLINE double get_wake_time(AsyncTask *task);
  void do_output(std::ostream &out) const;
  void do_write(std::ostream &out, int indent_level) const;

  void write_task_line(std::ostream &out, int indent_level, AsyncTask *task, double now) const;

protected:
  class AsyncTaskChainThread : public Thread {
  public:
    AsyncTaskChainThread(const std::string &name, AsyncTaskChain *chain);
    virtual void thread_main();

    AsyncTaskChain *_chain;
    AsyncTask *_servicing;
  };

  class AsyncTaskSortWakeTime {
  public:
    bool operator () (AsyncTask *a, AsyncTask *b) const {
      return AsyncTaskChain::get_wake_time(a) > AsyncTaskChain::get_wake_time(b);
    }
  };

  class AsyncTaskSortPriority {
  public:
    bool operator () (AsyncTask *a, AsyncTask *b) const {
      if (a->get_sort() != b->get_sort()) {
        return a->get_sort() > b->get_sort();
      }
      if (a->get_priority() != b->get_priority()) {
        return a->get_priority() < b->get_priority();
      }
      if (a->get_start_time() != b->get_start_time()) {
        return a->get_start_time() > b->get_start_time();
      }
      // Failing any other ordering criteria, we sort the tasks based on the
      // order in which they were added to the task chain.
      return a->_implicit_sort > b->_implicit_sort;
    }
  };

  typedef pvector< PT(AsyncTaskChainThread) > Threads;

  AsyncTaskManager *_manager;

  ConditionVar _cvar;  // signaled when one of the task heaps, _state, or _current_sort changes, or a task finishes.

  enum State {
    S_initial,     // no threads yet
    S_started,     // threads have been started
    S_interrupted, // task returned DS_interrupt, requested from sub-thread.
    S_shutdown     // waiting for thread shutdown, requested from main thread
  };

  bool _tick_clock;
  bool _timeslice_priority;
  int _num_threads;
  ThreadPriority _thread_priority;
  Threads _threads;
  double _frame_budget;
  bool _frame_sync;
  int _num_busy_threads;
  int _num_tasks;
  int _num_awaiting_tasks;
  TaskHeap _active;
  TaskHeap _this_active;
  TaskHeap _next_active;
  TaskHeap _sleeping;
  State _state;
  int _current_sort;
  bool _pickup_mode;
  bool _needs_cleanup;

  int _current_frame;
  double _time_in_frame;
  bool _block_till_next_frame;

  unsigned int _next_implicit_sort;

  static PStatCollector _task_pcollector;
  static PStatCollector _wait_pcollector;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "AsyncTaskChain",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class AsyncFuture;
  friend class AsyncTaskChainThread;
  friend class AsyncTask;
  friend class AsyncTaskManager;
  friend class AsyncTaskSortWakeTime;
};

INLINE std::ostream &operator << (std::ostream &out, const AsyncTaskChain &chain) {
  chain.output(out);
  return out;
};

#include "asyncTaskChain.I"

#endif
