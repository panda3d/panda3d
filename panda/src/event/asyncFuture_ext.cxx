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
#include "asyncTaskSequence.h"
#include "eventParameter.h"
#include "paramValue.h"
#include "pythonTask.h"

#ifdef HAVE_PYTHON

#ifndef CPPPARSER
extern struct Dtool_PyTypedObject Dtool_AsyncFuture;
extern struct Dtool_PyTypedObject Dtool_ParamValueBase;
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

    } else if (future->is_of_type(AsyncTaskSequence::get_class_type())) {
      // If it's an AsyncTaskSequence, get the result for each task.
      const AsyncTaskSequence *task = (const AsyncTaskSequence *)future;
      Py_ssize_t num_tasks = (Py_ssize_t)task->get_num_tasks();
      PyObject *results = PyTuple_New(num_tasks);

      for (Py_ssize_t i = 0; i < num_tasks; ++i) {
        PyObject *result = get_done_result(task->get_task(i));
        if (result != nullptr) {
          // This steals a reference.
          PyTuple_SET_ITEM(results, i, result);
        } else {
          Py_DECREF(results);
          return nullptr;
        }
      }
      return results;

    } else if (future->is_of_type(AsyncGatheringFuture::get_class_type())) {
      // If it's an AsyncGatheringFuture, get the result for each future.
      const AsyncGatheringFuture *gather = (const AsyncGatheringFuture *)future;
      Py_ssize_t num_futures = (Py_ssize_t)gather->get_num_futures();
      PyObject *results = PyTuple_New(num_futures);

      for (Py_ssize_t i = 0; i < num_futures; ++i) {
        PyObject *result = get_done_result(gather->get_future((size_t)i));
        if (result != nullptr) {
          // This steals a reference.
          PyTuple_SET_ITEM(results, i, result);
        } else {
          Py_DECREF(results);
          return nullptr;
        }
      }
      return results;

    } else {
      // It's any other future.
      ReferenceCount *ref_ptr;
      TypedObject *ptr;
      future->get_result(ptr, ref_ptr);

      if (ptr == nullptr) {
        Py_INCREF(Py_None);
        return Py_None;
      }

      TypeHandle type = ptr->get_type();
      if (type.is_derived_from(ParamValueBase::get_class_type())) {
        // If this is a ParamValueBase, return the 'value' property.
        // EventStoreInt and Double are not exposed to Python for some reason.
        if (type == EventStoreInt::get_class_type()) {
          return Dtool_WrapValue(((EventStoreInt *)ptr)->get_value());
        } else if (type == EventStoreDouble::get_class_type()) {
          return Dtool_WrapValue(((EventStoreDouble *)ptr)->get_value());
        }

        ParamValueBase *value = (ParamValueBase *)ptr;
        PyObject *wrap = DTool_CreatePyInstanceTyped
          ((void *)value, Dtool_ParamValueBase, false, false, type.get_index());
        if (wrap != nullptr) {
          PyObject *value = PyObject_GetAttrString(wrap, "value");
          if (value != nullptr) {
            return value;
          }
          PyErr_Restore(nullptr, nullptr, nullptr);
          Py_DECREF(wrap);
        }
      }

      if (ref_ptr != nullptr) {
        ref_ptr->ref();
      }

      return DTool_CreatePyInstanceTyped
        ((void *)ptr, Dtool_TypedObject, (ref_ptr != nullptr), false,
         type.get_index());
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
        exc_type = PyErr_NewExceptionWithDoc((char*)"concurrent.futures._base.CancelledError",
                                             (char*)"The Future was cancelled.",
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
  return Dtool_NewGenerator(self, &gen_next);
}

/**
 * Returns the result of this future, unless it was cancelled, in which case
 * it returns CancelledError.
 * If the future is not yet done, waits until the result is available.  If a
 * timeout is passed and the future is not done within the given timeout,
 * raises TimeoutError.
 */
PyObject *Extension<AsyncFuture>::
result(PyObject *timeout) const {
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
    if (timeout == Py_None) {
      _this->wait();
    } else {
      PyObject *num = PyNumber_Float(timeout);
      if (num != nullptr) {
        _this->wait(PyFloat_AS_DOUBLE(num));
      } else {
        return Dtool_Raise_ArgTypeError(timeout, 0, "result", "float");
      }
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
          exc_type = PyErr_NewExceptionWithDoc((char*)"concurrent.futures._base.TimeoutError",
                                               (char*)"The operation exceeded the given deadline.",
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

/**
 * Schedules the given function to be run as soon as the future is complete.
 * This is also called if the future is cancelled.
 * If the future is already done, the callback is scheduled right away.
 */
PyObject *Extension<AsyncFuture>::
add_done_callback(PyObject *self, PyObject *fn) {
  if (!PyCallable_Check(fn)) {
    return Dtool_Raise_ArgTypeError(fn, 0, "add_done_callback", "callable");
  }

  PythonTask *task = new PythonTask(fn);
  Py_DECREF(task->_args);
  task->_args = PyTuple_Pack(1, self);
  task->_append_task = false;
  task->_ignore_return = true;

  // If this is an AsyncTask, make sure it is scheduled on the same chain.
  if (_this->is_task()) {
    AsyncTask *this_task = (AsyncTask *)_this;
    task->set_task_chain(this_task->get_task_chain());
  }

  _this->add_waiting_task(task);

  Py_INCREF(Py_None);
  return Py_None;
}

/**
 * Creates a new future that returns `done()` when all of the contained
 * futures are done.
 *
 * Calling `cancel()` on the returned future will result in all contained
 * futures that have not yet finished to be cancelled.
 */
PyObject *Extension<AsyncFuture>::
gather(PyObject *args) {
  if (!PyTuple_Check(args)) {
    return Dtool_Raise_TypeError("args is not a tuple");
  }

  Py_ssize_t size = Py_SIZE(args);
  AsyncFuture::Futures futures;
  futures.reserve(size);

  for (Py_ssize_t i = 0; i < size; ++i) {
    PyObject *item = PyTuple_GET_ITEM(args, i);
    if (DtoolInstance_Check(item)) {
      AsyncFuture *fut = (AsyncFuture *)DtoolInstance_UPCAST(item, Dtool_AsyncFuture);
      if (fut != nullptr) {
        futures.push_back(fut);
        continue;
      }
#if PY_VERSION_HEX >= 0x03050000
    } else if (PyCoro_CheckExact(item)) {
      // We allow passing in a coroutine instead of a future.  This causes it
      // to be scheduled as a task.
      futures.push_back(new PythonTask(item));
      continue;
#endif
    }
    return Dtool_Raise_ArgTypeError(item, i, "gather", "coroutine, task or future");
  }

  AsyncFuture *future = AsyncFuture::gather(std::move(futures));
  if (future != nullptr) {
    future->ref();
    return DTool_CreatePyInstanceTyped((void *)future, Dtool_AsyncFuture, true, false, future->get_type_index());
  } else {
    return PyErr_NoMemory();
  }
}

#endif
