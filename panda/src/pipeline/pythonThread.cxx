// Filename: pythonThread.cxx
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
  _result = NULL;

  _function = function;
  if (!PyCallable_Check(_function)) {
    nassert_raise("Invalid function passed to PythonThread constructor");
  }
  Py_INCREF(_function);

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

  // Ensure that the Python threading system is initialized and ready
  // to go.
  PyEval_InitThreads();
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
  // Create a new Python thread state data structure, so Python can
  // properly lock itself.
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();

  // Call the user's function.
  _result = PyObject_Call(_function, _args, NULL);
  if (_result == (PyObject *)NULL && PyErr_Occurred()) {
    handle_python_exception();
  }

  // Release the thread state data structure.
  PyGILState_Release(gstate);
}

////////////////////////////////////////////////////////////////////
//     Function: PythonThread::handle_python_exception
//       Access: Private
//  Description: Called when a Python exception is raised during
//               processing of a thread.  Gets the error string and
//               passes it back to the calling Python process in a
//               sensible way.
////////////////////////////////////////////////////////////////////
void PythonThread::
handle_python_exception() {
  PyObject *exc, *val, *tb;
  PyErr_Fetch(&exc, &val, &tb);

  ostringstream strm;
  strm << "\n";

  PyObject *exc_name = PyObject_GetAttrString(exc, "__name__");
  if (exc_name == (PyObject *)NULL) {
    PyObject *exc_str = PyObject_Str(exc);
    strm << PyString_AsString(exc_str);
    Py_DECREF(exc_str);
  } else {
    PyObject *exc_str = PyObject_Str(exc_name);
    strm << PyString_AsString(exc_str);
    Py_DECREF(exc_str);
    Py_DECREF(exc_name);
  }
  Py_DECREF(exc);

  if (val != (PyObject *)NULL) {
    PyObject *val_str = PyObject_Str(val);
    strm << ": " << PyString_AsString(val_str);
    Py_DECREF(val_str);
    Py_DECREF(val);
  }
  if (tb != (PyObject *)NULL) {
    Py_DECREF(tb);
  }

  strm << "\nException occurred within thread " << get_name();
  string message = strm.str();
  nout << message << "\n";

  nassert_raise(message);
}

#endif  // HAVE_PYTHON
