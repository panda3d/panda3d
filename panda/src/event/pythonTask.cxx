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
  _upon_death = NULL;
  _owner = NULL;
  _registered_to_owner = false;
  _generator = NULL;

  set_function(function);
  set_args(Py_None, true);
  set_upon_death(Py_None);
  set_owner(Py_None);

  _dict = PyDict_New();

#ifndef SIMPLE_THREADS
  // Ensure that the Python threading system is initialized and ready
  // to go.
#ifdef WITH_THREAD  // This symbol defined within Python.h

#if PY_VERSION_HEX >= 0x03020000
  Py_Initialize();
#endif

  PyEval_InitThreads();
#endif
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
  Py_XDECREF(_generator);
  Py_XDECREF(_owner);
  Py_XDECREF(_upon_death);
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
  if (_function != Py_None && !PyCallable_Check(_function)) {
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

    this->ref();
    PyObject *self =
      DTool_CreatePyInstanceTyped(this, Dtool_TypedReferenceCount,
                                  true, false, get_type_index());
    PyTuple_SET_ITEM(with_task, num_args, self);
    return with_task;

  } else {
    Py_INCREF(_args);
    return _args;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PythonTask::set_upon_death
//       Access: Published
//  Description: Replaces the function that is called when the task
//               finishes.  The parameter should be a Python callable
//               object.
////////////////////////////////////////////////////////////////////
void PythonTask::
set_upon_death(PyObject *upon_death) {
  Py_XDECREF(_upon_death);

  _upon_death = upon_death;
  Py_INCREF(_upon_death);
  if (_upon_death != Py_None && !PyCallable_Check(_upon_death)) {
    nassert_raise("Invalid upon_death function passed to PythonTask");
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PythonTask::get_upon_death
//       Access: Published
//  Description: Returns the function that is called when the task
//               finishes.
////////////////////////////////////////////////////////////////////
PyObject *PythonTask::
get_upon_death() {
  Py_INCREF(_upon_death);
  return _upon_death;
}

////////////////////////////////////////////////////////////////////
//     Function: PythonTask::set_owner
//       Access: Published
//  Description: Specifies a Python object that serves as the "owner"
//               for the task.  This owner object must have two
//               methods: _addTask() and _clearTask(), which will be
//               called with one parameter, the task object.
//
//               owner._addTask() is called when the task is added
//               into the active task list, and owner._clearTask() is
//               called when it is removed.
////////////////////////////////////////////////////////////////////
void PythonTask::
set_owner(PyObject *owner) {
  if (_owner != NULL && _owner != Py_None && _state != S_inactive) {
    unregister_from_owner();
  }

  Py_XDECREF(_owner);
  _owner = owner;
  Py_INCREF(_owner);

  if (_owner != Py_None && _state != S_inactive) {
    register_to_owner();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PythonTask::get_owner
//       Access: Published
//  Description: Returns the "owner" object.  See set_owner().
////////////////////////////////////////////////////////////////////
PyObject *PythonTask::
get_owner() {
  Py_INCREF(_owner);
  return _owner;
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
  if (task_cat.is_debug()) {
    PyObject *str = PyObject_Repr(v);
    task_cat.debug() 
      << *this << ": task." << attr_name << " = "
#if PY_MAJOR_VERSION >= 3
      << PyUnicode_AsUTF8(str) << "\n";
#else
      << PyString_AsString(str) << "\n"; 
#endif
    Py_DECREF(str);
  }

  if (attr_name == "delayTime") {
    if (v == Py_None) {
      clear_delay();
    } else {
      double delay = PyFloat_AsDouble(v);
      if (!PyErr_Occurred()) {
        set_delay(delay);
      }
    }

  } else if (attr_name == "name") {
#if PY_MAJOR_VERSION >= 3
    char *name = PyUnicode_AsUTF8(v);
#else
    char *name = PyString_AsString(v);
#endif
    if (name != (char *)NULL) {
      set_name(name);
    }

  } else if (attr_name == "id" || attr_name == "time" || 
             attr_name == "frame" || attr_name == "wakeTime") {
    nassert_raise("Cannot set constant value");
    return true;

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

  } else if (attr_name == "name") {
#if PY_MAJOR_VERSION >= 3
    return PyUnicode_FromString(get_name().c_str());
#else
    return PyString_FromString(get_name().c_str());
#endif

  } else if (attr_name == "wakeTime") {
    return PyFloat_FromDouble(get_wake_time());

  } else if (attr_name == "delayTime") {
    if (!has_delay()) {
      Py_RETURN_NONE;
    }
    return PyFloat_FromDouble(get_delay());

  } else if (attr_name == "frame") {
#if PY_MAJOR_VERSION >= 3
    return PyLong_FromLong(get_elapsed_frames());
#else
    return PyInt_FromLong(get_elapsed_frames());
#endif

  } else if (attr_name == "id") {
#if PY_MAJOR_VERSION >= 3
    return PyLong_FromLong(_task_id);
#else
    return PyInt_FromLong(_task_id);
#endif

  } else {
    return PyMapping_GetItemString(_dict, (char *)attr_name.c_str());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PythonTask::is_runnable
//       Access: Protected, Virtual
//  Description: Override this function to return true if the task can
//               be successfully executed, false if it cannot.  Mainly
//               intended as a sanity check when attempting to add the
//               task to a task manager.
//
//               This function is called with the lock held.
////////////////////////////////////////////////////////////////////
bool PythonTask::
is_runnable() {
  return _function != Py_None;
}

////////////////////////////////////////////////////////////////////
//     Function: PythonTask::do_task
//       Access: Protected, Virtual
//  Description: Override this function to do something useful for the
//               task.
//
//               This function is called with the lock *not* held.
////////////////////////////////////////////////////////////////////
AsyncTask::DoneStatus PythonTask::
do_task() {
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  // Use PyGILState to protect this asynchronous call.
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();
#endif

  DoneStatus result = do_python_task();

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyGILState_Release(gstate);
#endif

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PythonTask::do_python_task
//       Access: Protected
//  Description: The Python calls that implement do_task().  This
//               function is separate so we can acquire the Python
//               interpretor lock while it runs.
////////////////////////////////////////////////////////////////////
AsyncTask::DoneStatus PythonTask::
do_python_task() {
  PyObject *result = NULL;

  if (_generator == (PyObject *)NULL) {
    // We are calling the function directly.
    PyObject *args = get_args();
    result = 
      Thread::get_current_thread()->call_python_func(_function, args);
    Py_DECREF(args);

#ifdef PyGen_Check
    if (result != (PyObject *)NULL && PyGen_Check(result)) {
      // The function has yielded a generator.  We will call into that
      // henceforth, instead of calling the function from the top
      // again.
      if (task_cat.is_debug()) {
#if PY_MAJOR_VERSION >= 3
        PyObject *str = PyObject_ASCII(_function);
        task_cat.debug()
          << PyUnicode_AsUTF8(str) << " in " << *this
          << " yielded a generator.\n";
#else
        PyObject *str = PyObject_Repr(_function);
        task_cat.debug()
          << PyString_AsString(str) << " in " << *this
          << " yielded a generator.\n";
#endif
        Py_DECREF(str);
      }
      _generator = result;
      result = NULL;
    }
#endif
  }

  if (_generator != (PyObject *)NULL) {
    // We are calling a generator.
    PyObject *func = PyObject_GetAttrString(_generator, "next");
    nassertr(func != (PyObject *)NULL, DS_interrupt);

    result = PyObject_CallObject(func, NULL);
    Py_DECREF(func);

    if (result == (PyObject *)NULL && PyErr_Occurred() &&
        PyErr_ExceptionMatches(PyExc_StopIteration)) {
      // "Catch" StopIteration and treat it like DS_done.
      PyErr_Clear();
      Py_DECREF(_generator);
      _generator = NULL;
      return DS_done;
    }
  }

  if (result == (PyObject *)NULL) {
    if (PyErr_Occurred() && PyErr_ExceptionMatches(PyExc_SystemExit)) {
      // Don't print an error message for SystemExit.  Or rather, make
      // it a debug message.
      if (task_cat.is_debug()) {
        task_cat.debug()
          << "SystemExit occurred in " << *this << "\n";
      }
    } else {
      task_cat.error()
        << "Exception occurred in " << *this << "\n";
    }
    return DS_interrupt;
  }

  if (result == Py_None) {
    Py_DECREF(result);
    return DS_done;
  }

#if PY_MAJOR_VERSION >= 3
  if (PyLong_Check(result)) {
    long retval = PyLong_AS_LONG(result);
#else
  if (PyInt_Check(result)) {
    long retval = PyInt_AS_LONG(result);
#endif

    switch (retval) {
    case DS_again:
      Py_XDECREF(_generator);
      _generator = NULL;
      // Fall through.

    case DS_done:
    case DS_cont:
    case DS_pickup:
    case DS_exit:
    case DS_pause:
      // Legitimate value.
      Py_DECREF(result);
      return (DoneStatus) retval;

    case -1:
      // Legacy value.
      Py_DECREF(result);
      return DS_done;

    default:
      // Unexpected value.
      break;
    }
  }

  ostringstream strm;
#if PY_MAJOR_VERSION >= 3
  PyObject *str = PyObject_ASCII(result);
  strm
    << *this << " returned " << PyUnicode_AsUTF8(str);
#else
  PyObject *str = PyObject_Repr(result);
  strm
    << *this << " returned " << PyString_AsString(str);
#endif
  Py_DECREF(str);
  Py_DECREF(result);
  string message = strm.str();
  nassert_raise(message);

  return DS_interrupt;
}

////////////////////////////////////////////////////////////////////
//     Function: PythonTask::upon_birth
//       Access: Protected, Virtual
//  Description: Override this function to do something useful when the
//               task has been added to the active queue.
//
//               This function is called with the lock *not* held.
////////////////////////////////////////////////////////////////////
void PythonTask::
upon_birth(AsyncTaskManager *manager) {
  AsyncTask::upon_birth(manager);
  register_to_owner();
}

////////////////////////////////////////////////////////////////////
//     Function: PythonTask::upon_death
//       Access: Protected, Virtual
//  Description: Override this function to do something useful when the
//               task has been removed from the active queue.  The
//               parameter clean_exit is true if the task has been
//               removed because it exited normally (returning
//               DS_done), or false if it was removed for some other
//               reason (e.g. AsyncTaskManager::remove()).  By the
//               time this method is called, _manager has been
//               cleared, so the parameter manager indicates the
//               original AsyncTaskManager that owned this task.
//
//               The normal behavior is to throw the done_event only
//               if clean_exit is true.
//
//               This function is called with the lock *not* held.
////////////////////////////////////////////////////////////////////
void PythonTask::
upon_death(AsyncTaskManager *manager, bool clean_exit) {
  AsyncTask::upon_death(manager, clean_exit);

  if (_upon_death != Py_None) {
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
    // Use PyGILState to protect this asynchronous call.
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
#endif
    
    call_function(_upon_death);
    
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
    PyGILState_Release(gstate);
#endif
  }
  unregister_from_owner();
}

////////////////////////////////////////////////////////////////////
//     Function: PythonTask::register_to_owner
//       Access: Private
//  Description: Tells the owner we are now his task.
////////////////////////////////////////////////////////////////////
void PythonTask::
register_to_owner() {
  if (_owner != Py_None && !_registered_to_owner) {
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
    // Use PyGILState to protect this asynchronous call.
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
#endif
    
    _registered_to_owner = true;
    call_owner_method("_addTask");
    
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
    PyGILState_Release(gstate);
#endif
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PythonTask::unregister_from_owner
//       Access: Private
//  Description: Tells the owner we are no longer his task.
////////////////////////////////////////////////////////////////////
void PythonTask::
unregister_from_owner() {
  // make sure every call to _clearTask corresponds to a call to _addTask
  if (_owner != Py_None && _registered_to_owner) {
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
    // Use PyGILState to protect this asynchronous call.
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
#endif
    
    _registered_to_owner = false;
    call_owner_method("_clearTask");
    
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
    PyGILState_Release(gstate);
#endif
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PythonTask::call_owner_method
//       Access: Private
//  Description: Calls the indicated method name on the given object,
//               if defined, passing in the task object as the only
//               parameter.
////////////////////////////////////////////////////////////////////
void PythonTask::
call_owner_method(const char *method_name) {
  if (_owner != Py_None) {
    PyObject *func = PyObject_GetAttrString(_owner, (char *)method_name);
    if (func == (PyObject *)NULL) {
#if PY_MAJOR_VERSION >= 3
      PyObject *str = PyObject_ASCII(_owner);
      task_cat.error()
        << "Owner object " << PyUnicode_AsUTF8(str) << " added to "
        << *this << " has no method " << method_name << "().\n";
#else
      PyObject *str = PyObject_Repr(_owner);
      task_cat.error()
        << "Owner object " << PyString_AsString(str) << " added to "
        << *this << " has no method " << method_name << "().\n";
#endif
      Py_DECREF(str);

    } else {
      call_function(func);
      Py_DECREF(func);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PythonTask::call_function
//       Access: Private
//  Description: Calls the indicated Python function, passing in the
//               task object as the only parameter.
////////////////////////////////////////////////////////////////////
void PythonTask::
call_function(PyObject *function) {
  if (function != Py_None) {
    this->ref();
    PyObject *self = 
      DTool_CreatePyInstanceTyped(this, Dtool_TypedReferenceCount,
                                  true, false, get_type_index());
    PyObject *args = Py_BuildValue("(O)", self);
    Py_DECREF(self);
    
    PyObject *result = PyObject_CallObject(function, args);
    Py_XDECREF(result);
    Py_DECREF(args);
  }
}

#endif  // HAVE_PYTHON
