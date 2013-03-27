// Filename: thread.cxx
// Created by:  drose (08Aug02)
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

#include "thread.h"
#include "mainThread.h"
#include "externalThread.h"
#include "config_pipeline.h"
#include "mutexDebug.h"
#include "conditionVarDebug.h"
#include "conditionVarFullDebug.h"

Thread *Thread::_main_thread;
Thread *Thread::_external_thread;
TypeHandle Thread::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: Thread::Constructor
//       Access: Protected
//  Description: Creates a new Thread object, but does not
//               immediately start executing it.  This gives the
//               caller a chance to store it in a PT(Thread) object,
//               if desired, before the thread gets a chance to
//               terminate and destruct itself.
//
//               Call start() to begin thread execution.
//
//               The name should be unique for each thread (though
//               this is not enforced, and not strictly required).
//               The sync_name can be shared between multiple
//               different threads; threads that run synchronously
//               with each other should be given the same sync_name,
//               for the benefit of PStats.
////////////////////////////////////////////////////////////////////
Thread::
Thread(const string &name, const string &sync_name) : 
  Namable(name), 
  _sync_name(sync_name), 
  _impl(this) 
{
  _started = false;
  _pstats_index = -1;
  _pstats_callback = NULL;
  _pipeline_stage = 0;
  _joinable = false;
  _current_task = NULL;

#ifdef HAVE_PYTHON
  _python_data = Py_None;
  Py_INCREF(_python_data);
#endif

#ifdef DEBUG_THREADS
  _blocked_on_mutex = NULL;
  _waiting_on_cvar = NULL;
  _waiting_on_cvar_full = NULL;
#endif

#if defined(HAVE_PYTHON) && !defined(SIMPLE_THREADS)
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
//     Function: Thread::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
Thread::
~Thread() {
#ifdef HAVE_PYTHON
  Py_DECREF(_python_data);
#endif

#ifdef DEBUG_THREADS
  nassertv(_blocked_on_mutex == NULL &&
           _waiting_on_cvar == NULL &&
           _waiting_on_cvar_full == NULL);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: Thread::bind_thread
//       Access: Published, Static
//  Description: Returns a new Panda Thread object associated with the
//               current thread (which has been created externally).
//               This can be used to bind a unique Panda Thread object
//               with an external thread, such as a new Python thread.
//
//               It is particularly useful to bind a Panda Thread
//               object to an external thread for the purposes of
//               PStats monitoring.  Without this call, each external
//               thread will be assigned the same global
//               ExternalThread object, which means they will all
//               appear in the same PStats graph.
//
//               It is the caller's responsibility to save the
//               returned Thread pointer for the lifetime of the
//               external thread.  It is an error for the Thread
//               pointer to destruct while the external thread is
//               still in the system.
//
//               It is also an error to call this method from the main
//               thread, or twice within a given thread, unless it is
//               given the same name each time (in which case the same
//               pointer will be returned each time).
////////////////////////////////////////////////////////////////////
PT(Thread) Thread::
bind_thread(const string &name, const string &sync_name) {
  Thread *current_thread = get_current_thread();
  if (current_thread != get_external_thread()) {
    // This thread already has an associated thread.
    nassertr(current_thread->get_name() == name && 
             current_thread->get_sync_name() == sync_name, current_thread);
    return current_thread;
  }

  PT(Thread) thread = new ExternalThread(name, sync_name);
  ThreadImpl::bind_thread(thread);
  return thread;
}

////////////////////////////////////////////////////////////////////
//     Function: Thread::set_pipeline_stage
//       Access: Published
//  Description: Specifies the Pipeline stage number associated with
//               this thread.  The default stage is 0 if no stage is
//               specified otherwise.
//
//               This must be a value in the range [0
//               .. pipeline->get_num_stages() - 1].  It specifies the
//               values that this thread observes for all pipelined
//               data.  Typically, an application thread will leave
//               this at 0, but a render thread may set it to 1 or 2
//               (to operate on the previous frame's data, or the
//               second previous frame's data).
////////////////////////////////////////////////////////////////////
void Thread::
set_pipeline_stage(int pipeline_stage) {
#ifdef THREADED_PIPELINE
  _pipeline_stage = pipeline_stage;
#else
  if (pipeline_stage != 0) {
    pipeline_cat.warning()
      << "Requested pipeline stage " << pipeline_stage
      << " but multithreaded render pipelines not enabled in build.\n";
  }
  _pipeline_stage = 0;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: Thread::output
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void Thread::
output(ostream &out) const {
  out << get_type() << " " << get_name();
}

////////////////////////////////////////////////////////////////////
//     Function: Thread::output_blocker
//       Access: Published
//  Description: Writes a description of the mutex or condition
//               variable that this thread is blocked on.  Writes
//               nothing if there is no blocker, or if we are not in
//               DEBUG_THREADS mode.
////////////////////////////////////////////////////////////////////
void Thread::
output_blocker(ostream &out) const {
#ifdef DEBUG_THREADS
  if (_blocked_on_mutex != (MutexDebug *)NULL) {
    _blocked_on_mutex->output_with_holder(out);
  } else if (_waiting_on_cvar != (ConditionVarDebug *)NULL) {
    out << *_waiting_on_cvar;
  } else if (_waiting_on_cvar_full != (ConditionVarFullDebug *)NULL) {
    out << *_waiting_on_cvar_full;
  }
#endif  // DEBUG_THREADS
}

////////////////////////////////////////////////////////////////////
//     Function: Thread::write_status
//       Access: Published, Static
//  Description:
////////////////////////////////////////////////////////////////////
void Thread::
write_status(ostream &out) {
#if defined(HAVE_THREADS) && defined(SIMPLE_THREADS)
  ThreadImpl::write_status(out);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: Thread::start
//       Access: Public
//  Description: Starts the thread executing.  It is only valid to
//               call this once.
//
//               The thread will begin executing its thread_main()
//               function, and will terminate when thread_main()
//               returns.
//
//               priority is intended as a hint to the relative
//               importance of this thread.  This may be ignored by
//               the thread implementation.
//
//               joinable should be set true if you intend to call
//               join() to wait for the thread to terminate, or false
//               if you don't care and you will never call join().
//               Note that the reference count on the Thread object is
//               incremented while the thread itself is running, so if
//               you just want to fire and forget a thread, you may
//               pass joinable = false, and never store the Thread
//               object.  It will automatically destruct itself when
//               it finishes.
//
//               The return value is true if the thread is
//               successfully started, false otherwise.
////////////////////////////////////////////////////////////////////
bool Thread::
start(ThreadPriority priority, bool joinable) {
  nassertr(!_started, false);

  if (!support_threads) {
    thread_cat->warning()
      << *this << " could not be started: support-threads is false.\n";
    return false;
  }

  _joinable = joinable;
  _started = _impl.start(priority, joinable);

  if (!_started) {
    thread_cat->warning()
      << *this << " could not be started!\n";
  }

  return _started;
}

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: Thread::set_python_data
//       Access: Published
//  Description: Sets an arbitrary Python object that may be
//               associated with this thread object.  This is just for
//               the purposes of associated arbitrary Python data with
//               the C++ object; other than managing the reference
//               count, the C++ code does nothing with this object.
////////////////////////////////////////////////////////////////////
void Thread::
set_python_data(PyObject *python_data) {
  Py_DECREF(_python_data);
  _python_data = python_data;
  Py_INCREF(_python_data);
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: Thread::get_python_data
//       Access: Published
//  Description: Returns the Python object that was set with
//               set_python_data().
////////////////////////////////////////////////////////////////////
PyObject *Thread::
get_python_data() const {
  Py_INCREF(_python_data);
  return _python_data;
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: Thread::call_python_func
//       Access: Public
//  Description: Internal function to safely call a Python function
//               within a sub-thread, that might execute in parallel
//               with existing Python code.  The return value is the
//               return value of the Python function, or NULL if there
//               was an exception.
////////////////////////////////////////////////////////////////////
PyObject *Thread::
call_python_func(PyObject *function, PyObject *args) {
  nassertr(this == get_current_thread(), NULL);

  // Create a new Python thread state data structure, so Python can
  // properly lock itself.  
  PyObject *result = NULL;

  if (this == get_main_thread()) {
    // In the main thread, just call the function.
    result = PyObject_Call(function, args, NULL);

    if (result == (PyObject *)NULL) {
      if (PyErr_Occurred() && PyErr_ExceptionMatches(PyExc_SystemExit)) {
        // If we caught SystemExit, let it pass by without bothering
        // to print a callback.

      } else {
        // Temporarily save and restore the exception state so we can
        // print a callback on-the-spot.
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
    nassertr(false, NULL);
#else

#ifdef SIMPLE_THREADS
    // We can't use the PyGILState interface, which assumes we are
    // using true OS-level threading.  PyGILState enforces policies
    // like only one thread state per OS-level thread, which is not
    // true in the case of SIMPLE_THREADS.

    // For some reason I don't fully understand, I'm getting a crash
    // when I clean up old PyThreadState objects with
    // PyThreadState_Delete().  It appears that the thread state is
    // still referenced somewhere at the time I call delete, and the
    // crash occurs because I've deleted an active pointer.

    // Storing these pointers in a vector for permanent recycling
    // seems to avoid this problem.  I wish I understood better what's
    // going wrong, but I guess this workaround will do.
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
    result = PyObject_Call(function, args, NULL);
    if (result == (PyObject *)NULL && PyErr_Occurred()) {
      // We got an exception.  Move the exception from the current
      // thread into the main thread, so it can be handled there.
      PyObject *exc, *val, *tb;
      PyErr_Fetch(&exc, &val, &tb);

      thread_cat.error()
        << "Exception occurred within " << *this << "\n";

      // Temporarily restore the exception state so we can print a
      // callback on-the-spot.
      Py_XINCREF(exc);
      Py_XINCREF(val);
      Py_XINCREF(tb);
      PyErr_Restore(exc, val, tb);
      PyErr_Print();

      PyThreadState_Swap(orig_thread_state);
      thread_states.push_back(new_thread_state);
      //PyThreadState_Clear(new_thread_state);
      //PyThreadState_Delete(new_thread_state);

      PyErr_Restore(exc, val, tb);

      // Now attempt to force the main thread to the head of the ready
      // queue, so it can respond to the exception immediately.  This
      // only works if the main thread is not blocked, of course.
      Thread::get_main_thread()->preempt();

    } else {
      // No exception.  Restore the thread state normally.
      PyThreadState *state = PyThreadState_Swap(orig_thread_state);
      thread_states.push_back(new_thread_state);
      //PyThreadState_Clear(new_thread_state);
      //PyThreadState_Delete(new_thread_state);
    }
    
#else  // SIMPLE_THREADS
    // With true threading enabled, we're better off using PyGILState.
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
    
    // Call the user's function.
    result = PyObject_Call(function, args, NULL);
    if (result == (PyObject *)NULL && PyErr_Occurred()) {
      // We got an exception.  Move the exception from the current
      // thread into the main thread, so it can be handled there.
      PyObject *exc, *val, *tb;
      PyErr_Fetch(&exc, &val, &tb);

      thread_cat.error()
        << "Exception occurred within " << *this << "\n";

      // Temporarily restore the exception state so we can print a
      // callback on-the-spot.
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

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: Thread::handle_python_exception
//       Access: Public
//  Description: Called when a Python exception is raised during
//               processing of a thread.  Gets the error string and
//               passes it back to the calling Python process in a
//               sensible way.
////////////////////////////////////////////////////////////////////
void Thread::
handle_python_exception() {
  /*
  PyObject *exc, *val, *tb;
  PyErr_Fetch(&exc, &val, &tb);

  ostringstream strm;
  strm << "\n";

  if (PyObject_HasAttrString(exc, "__name__")) {
    PyObject *exc_name = PyObject_GetAttrString(exc, "__name__");
    PyObject *exc_str = PyObject_Str(exc_name);
    strm << PyString_AsString(exc_str);
    Py_DECREF(exc_str);
    Py_DECREF(exc_name);
  } else {
    PyObject *exc_str = PyObject_Str(exc);
    strm << PyString_AsString(exc_str);
    Py_DECREF(exc_str);
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
  */

  thread_cat.error()
    << "Exception occurred within " << *this << "\n";

  // Now attempt to force the main thread to the head of the ready
  // queue, so it will be the one to receive the above assertion.
  // This mainly only has an effect if SIMPLE_THREADS is in use.
  Thread::get_main_thread()->preempt();
}
#endif  // HAVE_PYTHON

////////////////////////////////////////////////////////////////////
//     Function: Thread::init_main_thread
//       Access: Private, Static
//  Description: Creates the Thread object that represents the main
//               thread.
////////////////////////////////////////////////////////////////////
void Thread::
init_main_thread() {
  // There is a chance of mutual recursion at startup.  The count
  // variable here attempts to protect against that.
  static int count = 0;
  ++count;
  if (count == 1 && _main_thread == (Thread *)NULL) {
    _main_thread = new MainThread;
    _main_thread->ref();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Thread::init_external_thread
//       Access: Private, Static
//  Description: Creates the Thread object that represents all of the
//               external threads.
////////////////////////////////////////////////////////////////////
void Thread::
init_external_thread() {
  if (_external_thread == (Thread *)NULL) {
    _external_thread = new ExternalThread;
    _external_thread->ref();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Thread::PStatsCallback::Destructor
//       Access: Public, Virtual
//  Description: Since this class is just an interface definition,
//               there is no need to have a destructor.  However, we
//               must have one anyway to stop gcc's annoying warning.
////////////////////////////////////////////////////////////////////
Thread::PStatsCallback::
~PStatsCallback() {
}

////////////////////////////////////////////////////////////////////
//     Function: Thread::PStatsCallback::deactivate_hook
//       Access: Public, Virtual
//  Description: Called when the thread is deactivated (swapped for
//               another running thread).  This is intended to provide
//               a callback hook for PStats to assign time to
//               individual threads properly, particularly in the
//               SIMPLE_THREADS case.
////////////////////////////////////////////////////////////////////
void Thread::PStatsCallback::
deactivate_hook(Thread *) {
}

////////////////////////////////////////////////////////////////////
//     Function: Thread::PStatsCallback::activate_hook
//       Access: Public, Virtual
//  Description: Called when the thread is activated (resumes
//               execution).  This is intended to provide a callback
//               hook for PStats to assign time to individual threads
//               properly, particularly in the SIMPLE_THREADS case.
////////////////////////////////////////////////////////////////////
void Thread::PStatsCallback::
activate_hook(Thread *) {
}
