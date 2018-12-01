/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file asyncTask.h
 * @author drose
 * @date 2006-08-23
 */

#ifndef ASYNCTASK_H
#define ASYNCTASK_H

#include "pandabase.h"
#include "asyncFuture.h"
#include "namable.h"
#include "pmutex.h"
#include "conditionVar.h"
#include "pStatCollector.h"

class AsyncTaskManager;
class AsyncTaskChain;

/**
 * This class represents a concrete task performed by an AsyncManager.
 * Normally, you would subclass from this class, and override do_task(), to
 * define the functionality you wish to have the task perform.
 */
class EXPCL_PANDA_EVENT AsyncTask : public AsyncFuture, public Namable {
public:
  AsyncTask(const std::string &name = std::string());
  ALLOC_DELETED_CHAIN(AsyncTask);

PUBLISHED:
  virtual ~AsyncTask();

  enum DoneStatus {
    DS_done,      // normal task completion
    DS_cont,      // run task again next epoch
    DS_again,     // start the task over from the beginning
    DS_pickup,    // run task again this frame, if frame budget allows
    DS_exit,      // stop the enclosing sequence
    DS_pause,     // pause, then exit (useful within a sequence)
    DS_interrupt, // interrupt the task manager, but run task again
    DS_await,     // await a different task's completion
  };

  enum State {
    S_inactive,
    S_active,
    S_servicing,
    S_servicing_removed,  // Still servicing, but wants removal from manager.
    S_sleeping,
    S_active_nested,      // active within a sequence.
    S_awaiting,           // Waiting for a dependent task to complete
  };

  INLINE State get_state() const;
  INLINE bool is_alive() const;
  INLINE AsyncTaskManager *get_manager() const;

  bool remove();

  INLINE void set_delay(double delay);
  INLINE void clear_delay();
  INLINE bool has_delay() const;
  INLINE double get_delay() const;
  double get_wake_time() const;
  void recalc_wake_time();

  INLINE double get_start_time() const;
  double get_elapsed_time() const;
  INLINE int get_start_frame() const;
  int get_elapsed_frames() const;

  void set_name(const std::string &name);
  INLINE void clear_name();
  std::string get_name_prefix() const;

  INLINE AtomicAdjust::Integer get_task_id() const;

  void set_task_chain(const std::string &chain_name);
  INLINE const std::string &get_task_chain() const;

  void set_sort(int sort);
  INLINE int get_sort() const;

  void set_priority(int priority);
  INLINE int get_priority() const;

  INLINE void set_done_event(const std::string &done_event);

  INLINE double get_dt() const;
  INLINE double get_max_dt() const;
  INLINE double get_average_dt() const;

  virtual void output(std::ostream &out) const;

PUBLISHED:
  MAKE_PROPERTY(state, get_state);
  MAKE_PROPERTY(alive, is_alive);
  MAKE_PROPERTY(manager, get_manager);

  // The name of this task.
  MAKE_PROPERTY(name, get_name, set_name);

  // This is a number guaranteed to be unique for each different AsyncTask
  // object in the universe.
  MAKE_PROPERTY(id, get_task_id);

  MAKE_PROPERTY(task_chain, get_task_chain, set_task_chain);
  MAKE_PROPERTY(sort, get_sort, set_sort);
  MAKE_PROPERTY(priority, get_priority, set_priority);
  MAKE_PROPERTY(done_event, get_done_event, set_done_event);

  MAKE_PROPERTY(dt, get_dt);
  MAKE_PROPERTY(max_dt, get_max_dt);
  MAKE_PROPERTY(average_dt, get_average_dt);

protected:
  void jump_to_task_chain(AsyncTaskManager *manager);
  DoneStatus unlock_and_do_task();

  virtual bool cancel() final;
  virtual bool is_task() const final {return true;}

  virtual bool is_runnable();
  virtual DoneStatus do_task();
  virtual void upon_birth(AsyncTaskManager *manager);
  virtual void upon_death(AsyncTaskManager *manager, bool clean_exit);

protected:
  AtomicAdjust::Integer _task_id;
  std::string _chain_name;
  double _delay;
  bool _has_delay;
  double _wake_time;
  int _sort;
  int _priority;
  unsigned int _implicit_sort;

  State _state;
  Thread *_servicing_thread;
  AsyncTaskChain *_chain;

  double _start_time;
  int _start_frame;

  double _dt;
  double _max_dt;
  double _total_dt;
  int _num_frames;

  static AtomicAdjust::Integer _next_task_id;

  static PStatCollector _show_code_pcollector;
  PStatCollector _task_pcollector;

  friend class PythonTask;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AsyncFuture::init_type();
    register_type(_type_handle, "AsyncTask",
                  AsyncFuture::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class AsyncFuture;
  friend class AsyncTaskManager;
  friend class AsyncTaskChain;
  friend class AsyncTaskSequence;
};

INLINE std::ostream &operator << (std::ostream &out, const AsyncTask &task) {
  task.output(out);
  return out;
};

#include "asyncTask.I"

#endif
