/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pythonThread.h
 * @author drose
 * @date 2007-04-13
 */

#ifndef PYTHONTHREAD_H
#define PYTHONTHREAD_H

#include "pandabase.h"

#include "thread.h"

#ifdef HAVE_PYTHON
/**
 * This class is exposed to Python to allow creation of a Panda thread from
 * the Python level.  It will spawn a thread that executes an arbitrary Python
 * functor.
 */
class EXPCL_PANDA_PIPELINE PythonThread : public Thread {
PUBLISHED:
  PythonThread(PyObject *function, PyObject *args,
               const string &name, const string &sync_name);
  virtual ~PythonThread();

  BLOCKING PyObject *join();

protected:
  virtual void thread_main();

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
