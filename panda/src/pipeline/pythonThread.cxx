// Filename: pythonThread.cxx
// Created by:  drose (13Apr07)
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

#include "pythonThread.h"
#include "pnotify.h"

#ifdef HAVE_PYTHON

TypeHandle PythonThread::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PythonThread::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: PythonThread::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PythonThread::
~PythonThread() {
  Py_DECREF(_function);
  Py_XDECREF(_args);
  Py_XDECREF(_result);
}

////////////////////////////////////////////////////////////////////
//     Function: PythonThread::join
//       Access: Published
//  Description: Blocks the calling process until the thread
//               terminates.  If the thread has already terminated,
//               this returns immediately.
//
//               The PythonThread flavor of this function returns the
//               same value returned by the thread function.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: PythonThread::thread_main
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void PythonThread::
thread_main() {
  _result = call_python_func(_function, _args);
}

#endif  // HAVE_PYTHON
