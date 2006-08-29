// Filename: asyncTaskManager.h
// Created by:  drose (23Aug06)
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

#ifndef ASYNCTASKMANAGER_H
#define ASYNCTASKMANAGER_H

#include "pandabase.h"

#include "asyncTask.h"
#include "typedReferenceCount.h"
#include "thread.h"
#include "pmutex.h"
#include "conditionVarFull.h"
#include "pvector.h"
#include "pdeque.h"

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
//               Each task, once added to the FIFO queue, will
//               eventually be executed by one of the threads; if the
//               task then indicates it has more work to do, it will
//               be replaced at the end of the queue to go around
//               again.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA AsyncTaskManager : public TypedReferenceCount, public Namable {
PUBLISHED:
  AsyncTaskManager(const string &name, int num_threads);
  virtual ~AsyncTaskManager();

  INLINE int get_num_threads() const;

  void add(AsyncTask *task);
  bool add_and_do(AsyncTask *task);
  bool remove(AsyncTask *task);
  bool has_task(AsyncTask *task) const;

  INLINE int get_num_tasks() const;

  void poll();

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  class AsyncTaskManagerThread;

  int find_task(AsyncTask *task) const;
  void service_one_task(AsyncTaskManagerThread *thread);
  void task_done(AsyncTask *task);

  class AsyncTaskManagerThread : public Thread {
  public:
    AsyncTaskManagerThread(AsyncTaskManager *manager);
    virtual void thread_main();

    AsyncTaskManager *_manager;
    AsyncTask *_servicing;
  };

  typedef pvector< PT(AsyncTaskManagerThread) > Threads;
  typedef pdeque< PT(AsyncTask) > Tasks;

  int _num_threads;

  Mutex _lock;  // Protects all the following members.
  ConditionVarFull _cvar;  // singaled when _active or _state changes, or a task finishes.

  enum State {
    S_initial,  // no threads yet
    S_started,  // threads have been started
    S_shutdown  // waiting for thread shutdown
  };

  Threads _threads;
  Tasks _active;
  int _num_tasks;
  State _state;
  
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
};

#include "asyncTaskManager.I"

#endif
