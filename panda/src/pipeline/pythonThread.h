// Filename: pythonThread.h
// Created by:  drose (13Apr07)
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

#ifndef PYTHONTHREAD_H
#define PYTHONTHREAD_H

#include "pandabase.h"

#include "thread.h"

#ifdef HAVE_PYTHON

#undef _POSIX_C_SOURCE
#include <Python.h>

#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//       Class : PythonThread
// Description : This class is exposed to Python to allow creation of
//               a Panda thread from the Python level.  It will spawn
//               a thread that executes an arbitrary Python functor.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PIPELINE PythonThread : public Thread {
PUBLISHED:
  PythonThread(PyObject *function, PyObject *args,
               const string &name, const string &sync_name);
  virtual ~PythonThread();

  BLOCKING PyObject *join();

protected:
  virtual void thread_main();

private:
  void handle_python_exception();

private:
  PyObject *_function;
  PyObject *_args;
  PyObject *_result;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Thread::init_type();
    register_type(_type_handle, "PythonThread",
                  Thread::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};
#endif  // HAVE_PYTHON

#endif

