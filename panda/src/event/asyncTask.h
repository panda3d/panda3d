// Filename: asyncTask.h
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

#ifndef ASYNCTASK_H
#define ASYNCTASK_H

#include "pandabase.h"

#include "typedReferenceCount.h"
#include "namable.h"
#include "pmutex.h"
#include "conditionVar.h"

#ifdef HAVE_PYTHON

#undef HAVE_LONG_LONG  // NSPR and Python both define this.
#undef _POSIX_C_SOURCE
#include <Python.h>

#endif  // HAVE_PYTHON

class AsyncTaskManager;

////////////////////////////////////////////////////////////////////
//       Class : AsyncTask
// Description : This class represents a concrete task performed by an
//               AsyncManager.  Normally, you would subclass from this
//               class, and override do_task(), to define the
//               functionality you wish to have the task perform.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_EVENT AsyncTask : public TypedReferenceCount {
public:
  INLINE AsyncTask();
  virtual ~AsyncTask();
  ALLOC_DELETED_CHAIN(AsyncTask);

PUBLISHED:
  enum State {
    S_inactive,
    S_active,
    S_servicing,
  };

  INLINE State get_state() const;

  INLINE void set_done_event(const string &done_event);
  INLINE const string &get_done_event() const;

#ifdef HAVE_PYTHON
  INLINE void set_python_object(PyObject *python_object);
  INLINE PyObject *get_python_object() const;
#endif  // HAVE_PYTHON

  virtual void output(ostream &out) const;

protected:
  virtual bool do_task()=0;

protected:
  string _done_event;
  State _state;
  AsyncTaskManager *_manager;

private:
#ifdef HAVE_PYTHON
  PyObject *_python_object;
#endif  // HAVE_PYTHON
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "AsyncTask",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class AsyncTaskManager;
};

INLINE ostream &operator << (ostream &out, const AsyncTask &task) {
  task.output(out);
  return out;
};

#include "asyncTask.I"

#endif
