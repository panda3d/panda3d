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
#include "paramValue.h"
#include "paramPyObject.h"
#include "pythonTask.h"
#include "asyncTaskManager.h"
#include "config_event.h"

#ifdef HAVE_PYTHON

#ifndef CPPPARSER
extern struct Dtool_PyTypedObject Dtool_AsyncFuture;
extern struct Dtool_PyTypedObject Dtool_ParamValueBase;
extern struct Dtool_PyTypedObject Dtool_TypedObject;
extern struct Dtool_PyTypedObject Dtool_TypedReferenceCount;
extern struct Dtool_PyTypedObject Dtool_TypedWritableReferenceCount;
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
        return Py_NewRef(Py_None);
      }

      TypeHandle type = ptr->get_type();
      if (type.is_derived_from(ParamValueBase::get_class_type())) {
        // If this is a ParamValueBase, return the 'value' property.
        // EventStoreInt and Double are not exposed to Python for some reason.
        if (type == EventStoreInt::get_class_type()) {
          return Dtool_WrapValue(((EventStoreInt *)ptr)->get_value());
        }
        else if (type == EventStoreDouble::get_class_type()) {
          return Dtool_WrapValue(((EventStoreDouble *)ptr)->get_value());
        }
        else if (type == ParamPyObject::get_class_type()) {
          return ((ParamPyObject *)ptr)->get_value();
        }

        ParamValueBase *value = (ParamValueBase *)ptr;
        PyObject *wrap = DTool_CreatePyInstanceTyped
          ((void *)value, Dtool_ParamValueBase, false, false, type.get_index());
        if (wrap != nullptr) {
          PyObject *value = PyObject_GetAttrString(wrap, "value");
          Py_DECREF(wrap);
          if (value != nullptr) {
            return value;
          }
          PyErr_Clear();
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
    PyErr_SetNone(Extension<AsyncFuture>::get_cancelled_error_type());
    return nullptr;
  }
}

/**
 * Yields continuously until the task has finished.
 */
static PyObject *gen_next_asyncfuture(PyObject *self) {
  const AsyncFuture *future = nullptr;
  if (!Dtool_Call_ExtractThisPointer(self, Dtool_AsyncFuture, (void **)&future)) {
    return nullptr;
  }

  if (!future->done()) {
    // Continue awaiting the result.
    return Py_NewRef(self);
  }
  else {
    PyObject *result = get_done_result(future);
    if (result != nullptr) {
      PyErr_SetObject(PyExc_StopIteration, result);
      // PyErr_SetObject increased the reference count, so we no longer need our reference.
      Py_DECREF(result);
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
  return Dtool_NewGenerator(self, &gen_next_asyncfuture);
}

/**
 * Sets this future's result.  Can only be called if done() returns false.
 */
void Extension<AsyncFuture>::
set_result(PyObject *result) {
  if (result == Py_None) {
    _this->set_result(nullptr);
    return;
  }
  else if (DtoolInstance_Check(result)) {
    // If this is a Python subclass of a C++ type, fall through to below, since
    // we don't want to lose that extra information.
    if (Py_IS_TYPE(result, Dtool_GetPyTypeObject(DtoolInstance_TYPE(result)))) {
      void *ptr;
      if ((ptr = DtoolInstance_UPCAST(result, Dtool_TypedWritableReferenceCount))) {
        _this->set_result((TypedWritableReferenceCount *)ptr);
        return;
      }
      if ((ptr = DtoolInstance_UPCAST(result, Dtool_TypedReferenceCount))) {
        _this->set_result((TypedReferenceCount *)ptr);
        return;
      }
      if ((ptr = DtoolInstance_UPCAST(result, Dtool_TypedObject))) {
        _this->set_result((TypedObject *)ptr);
        return;
      }
    }
  }
  else if (PyUnicode_Check(result)) {
    Py_ssize_t result_len;
    wchar_t *result_str = PyUnicode_AsWideCharString(result, &result_len);
    _this->set_result(new EventStoreWstring(std::wstring(result_str, result_len)));
    PyMem_Free(result_str);
    return;
  }
  else if (PyLongOrInt_Check(result)) {
    long result_val = PyLongOrInt_AS_LONG(result);
    if (result_val >= INT_MIN && result_val <= INT_MAX) {
      _this->set_result(new EventStoreInt((int)result_val));
      return;
    }
  }
  else if (PyNumber_Check(result)) {
    _this->set_result(new EventStoreDouble(PyFloat_AsDouble(result)));
    return;
  }

  // If we don't recognize the type, store it as a generic PyObject pointer.
  ParamPyObject::init_type();
  _this->set_result(new ParamPyObject(result));
}

/**
 * Returns the result of this future, unless it was cancelled, in which case
 * it returns CancelledError.
 * If the future is not yet done, waits until the result is available.  If a
 * timeout is passed and the future is not done within the given timeout,
 * raises TimeoutError.
 */
PyObject *Extension<AsyncFuture>::
result(PyObject *self, PyObject *timeout) const {
  double timeout_val;
  if (timeout != Py_None) {
    timeout_val = PyFloat_AsDouble(timeout);
    if (timeout_val == -1.0 && PyErr_Occurred()) {
      return nullptr;
    }
  }

  if (!_this->done()) {
    // Not yet done?  Wait until it is done, or until a timeout occurs.  But
    // first check to make sure we're not trying to deadlock the thread.
    Thread *current_thread = Thread::get_current_thread();
    AsyncTask *current_task = (AsyncTask *)current_thread->get_current_task();
    if (_this == current_task) {
      PyErr_SetString(PyExc_RuntimeError, "cannot call task.result() from within the task");
      return nullptr;
    }

    PythonTask *python_task = nullptr;
    if (current_task != nullptr &&
        current_task->is_of_type(PythonTask::get_class_type())) {
      // If we are calling result() inside a coroutine, mark it as awaiting this
      // future.  That makes it possible to cancel() us from another thread.
      python_task = (PythonTask *)current_task;
      nassertr(python_task->_fut_waiter == nullptr, nullptr);
      python_task->_fut_waiter = self;
    }

    // Release the GIL for the duration.
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
    PyThreadState *_save;
    Py_UNBLOCK_THREADS
#endif
    if (timeout == Py_None) {
      _this->wait();
    }
    else {
      _this->wait(timeout_val);
    }
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
    Py_BLOCK_THREADS
#endif

    if (python_task != nullptr) {
      python_task->_fut_waiter = nullptr;
    }

    if (!_this->done()) {
      // It timed out.  Raise an exception.
      static PyObject *exc_type = nullptr;
      if (exc_type == nullptr) {
        // Get the TimeoutError that asyncio uses, too.
        PyObject *module = PyImport_ImportModule("asyncio.exceptions");
        if (module != nullptr) {
          exc_type = PyObject_GetAttrString(module, "TimeoutError");
          Py_DECREF(module);
        }
        else {
          PyErr_Clear();
        }
        // If we can't get that, we should pretend and make our own.
        if (exc_type == nullptr) {
          exc_type = PyErr_NewExceptionWithDoc((char*)"asyncio.exceptions.TimeoutError",
                                               (char*)"The operation exceeded the given deadline.",
                                               nullptr, nullptr);
        }
      }
      PyErr_SetNone(exc_type);
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

  return Py_NewRef(Py_None);
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
    } else if (PyCoro_CheckExact(item)) {
      // We allow passing in a coroutine instead of a future.  This causes it
      // to be scheduled as a task on the current task manager.
      PT(AsyncTask) task = new PythonTask(item);
      Thread *current_thread = Thread::get_current_thread();
      AsyncTask *current_task = (AsyncTask *)current_thread->get_current_task();
      if (current_task != nullptr) {
        task->set_task_chain(current_task->get_task_chain());
        current_task->get_manager()->add(task);
      }
      else {
        event_cat.warning()
          << "gather() with coroutine not called from within a task; not scheduling with task manager.\n";
      }
      futures.push_back(task);
      continue;
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

/**
 * Returns a borrowed reference to the CancelledError exception type.
 */
PyObject *Extension<AsyncFuture>::
get_cancelled_error_type() {
  static PyObject *exc_type = nullptr;
  if (exc_type == nullptr) {
    // This method should not affect the current exception, so stash it.
    PyObject *curexc_type, *curexc_value, *curexc_traceback;
    PyErr_Fetch(&curexc_type, &curexc_value, &curexc_traceback);

    // Get the CancelledError that asyncio uses, too.
    PyObject *module = PyImport_ImportModule("asyncio.exceptions");
    if (module != nullptr) {
      exc_type = PyObject_GetAttrString(module, "CancelledError");
      Py_DECREF(module);
    }

    // If we can't get that, we should pretend and make our own.
    if (exc_type == nullptr) {
      exc_type = PyErr_NewExceptionWithDoc((char *)"asyncio.exceptions.CancelledError",
                                            (char *)"The Future or Task was cancelled.",
                                            PyExc_BaseException, nullptr);
    }

    PyErr_Restore(curexc_type, curexc_value, curexc_traceback);
  }
  return exc_type;
}

#endif
