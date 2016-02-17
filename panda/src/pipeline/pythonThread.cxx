/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pythonThread.cxx
 * @author drose
 * @date 2007-04-13
 */

#include "pythonThread.h"
#include "pnotify.h"

#ifdef HAVE_PYTHON
#include "py_panda.h"

TypeHandle PythonThread::_type_handle;

/**

 */
PythonThread::
PythonThread(PyObject *function, PyObject *args,
             const string &name, const string &sync_name) :
  Thread(name, sync_name)
{
  _function = function;
  Py_INCREF(_function);
  _args = NULL;
  _result = NULL;

  if (!PyCallable_Check(_function)) {
    nassert_raise("Invalid function passed to PythonThread constructor");
  }

  if (args == Py_None) {
    // None means no arguments; create an empty tuple.
    _args = PyTuple_New(0);
  } else {
    _args = NULL;
    if (PySequence_Check(args)) {
      _args = PySequence_Tuple(args);
    }
    if (_args == NULL) {
      nassert_raise("Invalid args passed to PythonThread constructor");
    }
  }
}

/**

 */
PythonThread::
~PythonThread() {
  Py_DECREF(_function);
  Py_XDECREF(_args);
  Py_XDECREF(_result);
}

/**
 * Blocks the calling process until the thread terminates.  If the thread has
 * already terminated, this returns immediately.  The PythonThread flavor of
 * this function returns the same value returned by the thread function.
 */
PyObject *PythonThread::
join() {
  Thread::join();

  if (_result == NULL) {
    // No result; return None.
    return Py_BuildValue("");
  }

  Py_INCREF(_result);
  return _result;
}

/**

 */
void PythonThread::
thread_main() {
  _result = call_python_func(_function, _args);
}

#endif  // HAVE_PYTHON
