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
 *
 */
PythonThread::
PythonThread(PyObject *function, PyObject *args,
             const std::string &name, const std::string &sync_name) :
  Thread(name, sync_name)
{
  _function = function;
  Py_INCREF(_function);
  _args = nullptr;
  _result = nullptr;

  if (!PyCallable_Check(_function)) {
    nassert_raise("Invalid function passed to PythonThread constructor");
  }

  set_args(args);

#ifndef SIMPLE_THREADS
  // Ensure that the Python threading system is initialized and ready to go.
#ifdef WITH_THREAD  // This symbol defined within Python.h
  PyEval_InitThreads();
#endif
#endif
}

/**
 *
 */
PythonThread::
~PythonThread() {
  // Unfortunately, we need to grab the GIL to release these things,
  // since the destructor could be called from any thread.
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();
#endif

  Py_DECREF(_function);
  Py_XDECREF(_args);
  Py_XDECREF(_result);

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyGILState_Release(gstate);
#endif
}

/**
 * Blocks the calling process until the thread terminates.  If the thread has
 * already terminated, this returns immediately.
 *
 * The PythonThread flavor of this function returns the same value returned by
 * the thread function.
 */
PyObject *PythonThread::
join() {
  Thread::join();

  if (_result == nullptr) {
    // No result; return None.
    Py_INCREF(Py_None);
    return Py_None;
  }

  Py_INCREF(_result);
  return _result;
}

/**
 *
 */
PyObject *PythonThread::
get_args() const {
  return _args;
}

/**
 *
 */
void PythonThread::
set_args(PyObject *args) {
  Py_XDECREF(_args);

  if (args == Py_None) {
    // None means no arguments; create an empty tuple.
    _args = PyTuple_New(0);
  } else {
    _args = nullptr;
    if (PySequence_Check(args)) {
      _args = PySequence_Tuple(args);
    }
    if (_args == nullptr) {
      Dtool_Raise_TypeError("PythonThread args must be a tuple");
    }
  }
}

#ifdef HAVE_PYTHON
/**
 * Internal function to safely call a Python function within a sub-thread,
 * that might execute in parallel with existing Python code.  The return value
 * is the return value of the Python function, or NULL if there was an
 * exception.
 */
PyObject *PythonThread::
call_python_func(PyObject *function, PyObject *args) {
  Thread *current_thread = get_current_thread();

  // Create a new Python thread state data structure, so Python can properly
  // lock itself.
  PyObject *result = nullptr;

  if (current_thread == get_main_thread()) {
    // In the main thread, just call the function.
    result = PyObject_Call(function, args, nullptr);

    if (result == nullptr) {
      if (PyErr_Occurred() && PyErr_ExceptionMatches(PyExc_SystemExit)) {
        // If we caught SystemExit, let it pass by without bothering to print
        // a callback.

      } else {
        // Temporarily save and restore the exception state so we can print a
        // callback on-the-spot.
        PyObject *exc, *val, *tb;
        PyErr_Fetch(&exc, &val, &tb);

        Py_XINCREF(exc);
        Py_XINCREF(val);
        Py_XINCREF(tb);
        PyErr_Restore(exc, val, tb);
        PyErr_Print();

        PyErr_Restore(exc, val, tb);
      }
    }

  } else {
#ifndef HAVE_THREADS
    // Shouldn't be possible to come here without having some kind of
    // threading support enabled.
    nassert_raise("threading support disabled");
    return nullptr;
#else

#ifdef SIMPLE_THREADS
    // We can't use the PyGILState interface, which assumes we are using true
    // OS-level threading.  PyGILState enforces policies like only one thread
    // state per OS-level thread, which is not true in the case of
    // SIMPLE_THREADS.

    // For some reason I don't fully understand, I'm getting a crash when I
    // clean up old PyThreadState objects with PyThreadState_Delete().  It
    // appears that the thread state is still referenced somewhere at the time
    // I call delete, and the crash occurs because I've deleted an active
    // pointer.

    // Storing these pointers in a vector for permanent recycling seems to
    // avoid this problem.  I wish I understood better what's going wrong, but
    // I guess this workaround will do.
    static pvector<PyThreadState *> thread_states;

    PyThreadState *orig_thread_state = PyThreadState_Get();
    PyInterpreterState *istate = orig_thread_state->interp;
    PyThreadState *new_thread_state;
    if (thread_states.empty()) {
      new_thread_state = PyThreadState_New(istate);
    } else {
      new_thread_state = thread_states.back();
      thread_states.pop_back();
    }
    PyThreadState_Swap(new_thread_state);

    // Call the user's function.
    result = PyObject_Call(function, args, nullptr);
    if (result == nullptr && PyErr_Occurred()) {
      // We got an exception.  Move the exception from the current thread into
      // the main thread, so it can be handled there.
      PyObject *exc, *val, *tb;
      PyErr_Fetch(&exc, &val, &tb);

      thread_cat.error()
        << "Exception occurred within " << *current_thread << "\n";

      // Temporarily restore the exception state so we can print a callback
      // on-the-spot.
      Py_XINCREF(exc);
      Py_XINCREF(val);
      Py_XINCREF(tb);
      PyErr_Restore(exc, val, tb);
      PyErr_Print();

      PyThreadState_Swap(orig_thread_state);
      thread_states.push_back(new_thread_state);
      // PyThreadState_Clear(new_thread_state);
      // PyThreadState_Delete(new_thread_state);

      PyErr_Restore(exc, val, tb);

      // Now attempt to force the main thread to the head of the ready queue,
      // so it can respond to the exception immediately.  This only works if
      // the main thread is not blocked, of course.
      Thread::get_main_thread()->preempt();

    } else {
      // No exception.  Restore the thread state normally.
      PyThreadState_Swap(orig_thread_state);
      thread_states.push_back(new_thread_state);
      // PyThreadState_Clear(new_thread_state);
      // PyThreadState_Delete(new_thread_state);
    }

#else  // SIMPLE_THREADS
    // With true threading enabled, we're better off using PyGILState.
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    // Call the user's function.
    result = PyObject_Call(function, args, nullptr);
    if (result == nullptr && PyErr_Occurred()) {
      // We got an exception.  Move the exception from the current thread into
      // the main thread, so it can be handled there.
      PyObject *exc, *val, *tb;
      PyErr_Fetch(&exc, &val, &tb);

      thread_cat.error()
        << "Exception occurred within " << *current_thread << "\n";

      // Temporarily restore the exception state so we can print a callback
      // on-the-spot.
      Py_XINCREF(exc);
      Py_XINCREF(val);
      Py_XINCREF(tb);
      PyErr_Restore(exc, val, tb);
      PyErr_Print();

      PyGILState_Release(gstate);

      PyErr_Restore(exc, val, tb);
    } else {
      // No exception.  Restore the thread state normally.
      PyGILState_Release(gstate);
    }


#endif  // SIMPLE_THREADS
#endif  // HAVE_THREADS
  }

  return result;
}
#endif  // HAVE_PYTHON

/**
 *
 */
void PythonThread::
thread_main() {
  _result = call_python_func(_function, _args);
}

#endif  // HAVE_PYTHON
