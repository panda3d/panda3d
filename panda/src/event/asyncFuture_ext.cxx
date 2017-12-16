/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file asyncFuture_ext.h
 * @author rdb
 * @date 2017-10-29
 */

#include "asyncFuture_ext.h"
#include "pythonTask.h"

#ifdef HAVE_PYTHON

#ifndef CPPPARSER
extern struct Dtool_PyTypedObject Dtool_AsyncFuture;
extern struct Dtool_PyTypedObject Dtool_TypedObject;
#endif

/**
 * Get the result of a future, which may be a PythonTask.  Assumes that the
 * future is already done.
 */
static PyObject *get_done_result(const AsyncFuture *future) {
  if (!future->cancelled()) {
    if (future->is_of_type(PythonTask::get_class_type())) {
      // If it's a PythonTask, defer to its get_result(), since it may store
      // any PyObject value or raise an exception.
      const PythonTask *task = (const PythonTask *)future;
      return task->get_result();
    } else {
      ReferenceCount *ref_ptr;
      TypedObject *ptr;
      future->get_result(ptr, ref_ptr);

      if (ptr == nullptr) {
        Py_INCREF(Py_None);
        return Py_None;
      }

      if (ref_ptr != nullptr) {
        ref_ptr->ref();
      }

      return DTool_CreatePyInstanceTyped
        ((void *)ptr, Dtool_TypedObject, (ref_ptr != nullptr), false,
         ptr->get_type_index());
    }
  } else {
    // If the future was cancelled, we should raise an exception.
    static PyObject *exc_type = nullptr;
    if (exc_type == nullptr) {
      // Get the CancelledError that asyncio uses, too.
      PyObject *module = PyImport_ImportModule("concurrent.futures._base");
      if (module != nullptr) {
        exc_type = PyObject_GetAttrString(module, "CancelledError");
        Py_DECREF(module);
      }
      // If we can't get that, we should pretend and make our own.
      if (exc_type == nullptr) {
        exc_type = PyErr_NewExceptionWithDoc("concurrent.futures._base.CancelledError",
                                             "The Future was cancelled.",
                                             nullptr, nullptr);
      }
    }
    Py_INCREF(exc_type);
    PyErr_Restore(exc_type, nullptr, nullptr);
    return nullptr;
  }
}

/**
 * Yields continuously until the task has finished.
 */
static PyObject *gen_next(PyObject *self) {
  const AsyncFuture *future = nullptr;
  if (!Dtool_Call_ExtractThisPointer(self, Dtool_AsyncFuture, (void **)&future)) {
    return nullptr;
  }

  if (!future->done()) {
    // Continue awaiting the result.
    Py_INCREF(self);
    return self;
  } else {
    PyObject *result = get_done_result(future);
    if (result != nullptr) {
      Py_INCREF(PyExc_StopIteration);
      PyErr_Restore(PyExc_StopIteration, result, nullptr);
    }
    return nullptr;
  }
}

/**
 * Returns a generator that continuously yields an awaitable until the task
 * has finished.  This allows syntax like `model = await loader.load...` to be
 * used in a Python coroutine.
 */
PyObject *Extension<AsyncFuture>::
__await__(PyObject *self) {
  Dtool_GeneratorWrapper *gen;
  gen = (Dtool_GeneratorWrapper *)PyType_GenericAlloc(&Dtool_GeneratorWrapper_Type, 0);
  if (gen != nullptr) {
    Py_INCREF(self);
    gen->_base._self = self;
    gen->_iternext_func = &gen_next;
  }
  return (PyObject *)gen;
}

/**
 * Returns the result of this future, unless it was cancelled, in which case
 * it returns CancelledError.
 * If the future is not yet done, waits until the result is available.  If a
 * timeout is passed and the future is not done within the given timeout,
 * raises TimeoutError.
 */
PyObject *Extension<AsyncFuture>::
result(double timeout) const {
  if (!_this->done()) {
    // Not yet done?  Wait until it is done, or until a timeout occurs.  But
    // first check to make sure we're not trying to deadlock the thread.
    Thread *current_thread = Thread::get_current_thread();
    if (_this == (const AsyncFuture *)current_thread->get_current_task()) {
      PyErr_SetString(PyExc_RuntimeError, "cannot call task.result() from within the task");
      return nullptr;
    }

    // Release the GIL for the duration.
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
    PyThreadState *_save;
    Py_UNBLOCK_THREADS
#endif
    //TODO: check why gcc and clang don't like infinity timeout.
    if (cinf(timeout) || timeout < 0.0) {
      _this->wait();
    } else {
      _this->wait(timeout);
    }
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
    Py_BLOCK_THREADS
#endif

    if (!_this->done()) {
      // It timed out.  Raise an exception.
      static PyObject *exc_type = nullptr;
      if (exc_type == nullptr) {
        // Get the TimeoutError that asyncio uses, too.
        PyObject *module = PyImport_ImportModule("concurrent.futures._base");
        if (module != nullptr) {
          exc_type = PyObject_GetAttrString(module, "TimeoutError");
          Py_DECREF(module);
        }
        // If we can't get that, we should pretend and make our own.
        if (exc_type == nullptr) {
          exc_type = PyErr_NewExceptionWithDoc("concurrent.futures._base.TimeoutError",
                                               "The operation exceeded the given deadline.",
                                               nullptr, nullptr);
        }
      }
      Py_INCREF(exc_type);
      PyErr_Restore(exc_type, nullptr, nullptr);
      return nullptr;
    }
  }

  return get_done_result(_this);
}

#endif
