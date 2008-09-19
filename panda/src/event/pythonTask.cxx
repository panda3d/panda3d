// Filename: pythonTask.cxx
// Created by:  drose (16Sep08)
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

#include "pythonTask.h"
#include "pnotify.h"

#ifdef HAVE_PYTHON
#include "py_panda.h"  

TypeHandle PythonTask::_type_handle;

#ifndef CPPPARSER
IMPORT_THIS struct Dtool_PyTypedObject Dtool_TypedReferenceCount;
#endif

////////////////////////////////////////////////////////////////////
//     Function: PythonTask::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PythonTask::
PythonTask(PyObject *function, const string &name) :
  AsyncTask(name)
{
  _function = NULL;
  _args = NULL;

  set_function(function);
  set_args(Py_None, true);

  _dict = PyDict_New();

#ifndef SIMPLE_THREADS
  // Ensure that the Python threading system is initialized and ready
  // to go.
  PyEval_InitThreads();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: PythonTask::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PythonTask::
~PythonTask() {
  Py_DECREF(_function);
  Py_DECREF(_args);
  Py_DECREF(_dict);
}

////////////////////////////////////////////////////////////////////
//     Function: PythonTask::set_function
//       Access: Published
//  Description: Replaces the function that is called when the task
//               runs.  The parameter should be a Python callable
//               object.
////////////////////////////////////////////////////////////////////
void PythonTask::
set_function(PyObject *function) {
  Py_XDECREF(_function);

  _function = function;
  Py_INCREF(_function);
  if (!PyCallable_Check(_function)) {
    nassert_raise("Invalid function passed to PythonTask");
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PythonTask::get_function
//       Access: Published
//  Description: Returns the function that is called when the task
//               runs.
////////////////////////////////////////////////////////////////////
PyObject *PythonTask::
get_function() {
  Py_INCREF(_function);
  return _function;
}

////////////////////////////////////////////////////////////////////
//     Function: PythonTask::set_args
//       Access: Published
//  Description: Replaces the argument list that is passed to the task
//               function.  The parameter should be a tuple or list of
//               arguments, or None to indicate the empty list.
////////////////////////////////////////////////////////////////////
void PythonTask::
set_args(PyObject *args, bool append_task) {
  Py_XDECREF(_args);
  _args = NULL;
    
  if (args == Py_None) {
    // None means no arguments; create an empty tuple.
    _args = PyTuple_New(0);
  } else {
    if (PySequence_Check(args)) {
      _args = PySequence_Tuple(args);
    }
  }

  if (_args == NULL) {
    nassert_raise("Invalid args passed to PythonTask");
    _args = PyTuple_New(0);
  }

  _append_task = append_task;
}

////////////////////////////////////////////////////////////////////
//     Function: PythonTask::get_args
//       Access: Published
//  Description: Returns the argument list that is passed to the task
//               function.
////////////////////////////////////////////////////////////////////
PyObject *PythonTask::
get_args() {
  if (_append_task) {
    // If we want to append the task, we have to create a new tuple
    // with space for one more at the end.  We have to do this
    // dynamically each time, to avoid storing the task itself in its
    // own arguments list, and thereby creating a cyclical reference.

    int num_args = PyTuple_GET_SIZE(_args);
    PyObject *with_task = PyTuple_New(num_args + 1);
    for (int i = 0; i < num_args; ++i) {
      PyObject *item = PyTuple_GET_ITEM(_args, i);
      Py_INCREF(item);
      PyTuple_SET_ITEM(with_task, i, item);
    }

    PyObject *self = 
      DTool_CreatePyInstanceTyped(this, Dtool_TypedReferenceCount,
                                  true, false, get_type_index());
    Py_INCREF(self);
    PyTuple_SET_ITEM(with_task, num_args, self);
    return with_task;

  } else {
    Py_INCREF(_args);
    return _args;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PythonTask::__setattr__
//       Access: Published
//  Description: Maps from an expression like "task.attr_name = v".
//               This is customized here so we can support some
//               traditional task interfaces that supported directly
//               assigning certain values.  We also support adding
//               arbitrary data to the Task object.
////////////////////////////////////////////////////////////////////
int PythonTask::
__setattr__(const string &attr_name, PyObject *v) {
  if (attr_name == "delayTime") {
    double delay = PyFloat_AsDouble(v);
    set_delay(delay);

  } else {
    return PyDict_SetItemString(_dict, attr_name.c_str(), v);
  }

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: PythonTask::__setattr__
//       Access: Published
//  Description: Maps from an expression like "del task.attr_name".
//               This is customized here so we can support some
//               traditional task interfaces that supported directly
//               assigning certain values.  We also support adding
//               arbitrary data to the Task object.
////////////////////////////////////////////////////////////////////
int PythonTask::
__setattr__(const string &attr_name) {
  return PyDict_DelItemString(_dict, attr_name.c_str());
}

////////////////////////////////////////////////////////////////////
//     Function: PythonTask::__getattr__
//       Access: Published
//  Description: Maps from an expression like "task.attr_name".
//               This is customized here so we can support some
//               traditional task interfaces that supported directly
//               querying certain values.  We also support adding
//               arbitrary data to the Task object.
////////////////////////////////////////////////////////////////////
PyObject *PythonTask::
__getattr__(const string &attr_name) const {
  if (attr_name == "time") {
    return PyFloat_FromDouble(get_elapsed_time());
  } else if (attr_name == "done") {
    return PyInt_FromLong(DS_done);
  } else if (attr_name == "cont") {
    return PyInt_FromLong(DS_cont);
  } else if (attr_name == "again") {
    return PyInt_FromLong(DS_again);
  } else {
    return PyMapping_GetItemString(_dict, (char *)attr_name.c_str());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PythonTask::do_task
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
AsyncTask::DoneStatus PythonTask::
do_task() {
  PyObject *args = get_args();
  PyObject *result = 
    Thread::get_current_thread()->call_python_func(_function, args);
  Py_DECREF(args);

  if (result == (PyObject *)NULL) {
    event_cat.error()
      << "Exception occurred in " << *this << "\n";
    return DS_abort;
  }

  if (result == Py_None) {
    Py_DECREF(result);
    return DS_done;
  }

  if (PyInt_Check(result)) {
    int retval = PyInt_AS_LONG(result);
    switch (retval) {
    case DS_done:
    case DS_cont:
    case DS_again:
      // Legitimate value.
      Py_DECREF(result);
      return (DoneStatus)retval;

    case -1:
      // Legacy value.
      Py_DECREF(result);
      return DS_done;

    default:
      // Unexpected value.
      break;
    }
  }

  PyObject *str = PyObject_Repr(result);
  ostringstream strm;
  strm
    << *this << " returned " << PyString_AsString(str);
  Py_DECREF(str);
  Py_DECREF(result);
  string message = strm.str();
  nassert_raise(message);

  return DS_abort;
}

#endif  // HAVE_PYTHON
