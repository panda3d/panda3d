/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pythonTask.cxx
 * @author drose
 * @date 2008-09-16
 */

#include "pythonTask.h"
#include "pnotify.h"
#include "config_event.h"

#ifdef HAVE_PYTHON
#include "py_panda.h"

#include "pythonThread.h"
#include "asyncTaskManager.h"
#include "asyncFuture_ext.h"

TypeHandle PythonTask::_type_handle;

#ifndef CPPPARSER
extern struct Dtool_PyTypedObject Dtool_TypedReferenceCount;
extern struct Dtool_PyTypedObject Dtool_AsyncFuture;
extern struct Dtool_PyTypedObject Dtool_PythonTask;
#endif

/**
 *
 */
PythonTask::
PythonTask(PyObject *func_or_coro, const std::string &name) :
  AsyncTask(name),
  _function(nullptr),
  _args(nullptr),
  _upon_death(nullptr),
  _owner(nullptr),
  _exception(nullptr),
  _exc_value(nullptr),
  _exc_traceback(nullptr),
  _generator(nullptr),
  _fut_waiter(nullptr),
  _ignore_return(false),
  _registered_to_owner(false),
  _retrieved_exception(false) {

  nassertv(func_or_coro != nullptr);
  if (func_or_coro == Py_None || PyCallable_Check(func_or_coro)) {
    _function = Py_NewRef(func_or_coro);
  }
  else if (PyCoro_CheckExact(func_or_coro)) {
    // We also allow passing in a coroutine, because why not.
    _generator = Py_NewRef(func_or_coro);
  }
  else if (PyGen_CheckExact(func_or_coro)) {
    // Something emulating a coroutine.
    _generator = Py_NewRef(func_or_coro);
  }
  else {
    nassert_raise("Invalid function passed to PythonTask");
  }

  set_args(Py_None, true);
  set_upon_death(Py_None);
  set_owner(Py_None);

  __dict__ = PyDict_New();

#if !defined(SIMPLE_THREADS) && defined(WITH_THREAD) && PY_VERSION_HEX < 0x03090000
  // Ensure that the Python threading system is initialized and ready to go.
  // WITH_THREAD symbol defined within Python.h
  // PyEval_InitThreads is now a deprecated no-op in Python 3.9+
  PyEval_InitThreads();
#endif
}

/**
 *
 */
PythonTask::
~PythonTask() {
  // If the coroutine threw an exception, and there was no opportunity to
  // handle it, let the user know.
  if (_exception != nullptr && !_retrieved_exception) {
    task_cat.error()
      << *this << " exception was never retrieved:\n";
    PyErr_Restore(_exception, _exc_value, _exc_traceback);
    PyErr_Print();
    PyErr_Restore(nullptr, nullptr, nullptr);
    _exception = nullptr;
    _exc_value = nullptr;
    _exc_traceback = nullptr;
  }

  PyObject *self = __self__;
  if (self != nullptr) {
    PyObject_GC_UnTrack(self);
    __self__ = nullptr;
    Py_DECREF(self);
  }

  // All of these may have already been cleared by __clear__.
  Py_XDECREF(_function);
  Py_XDECREF(_args);
  Py_XDECREF(__dict__);
  Py_XDECREF(_exception);
  Py_XDECREF(_exc_value);
  Py_XDECREF(_exc_traceback);
  Py_XDECREF(_generator);
  Py_XDECREF(_owner);
  Py_XDECREF(_upon_death);
}

/**
 * Replaces the function that is called when the task runs.  The parameter
 * should be a Python callable object.
 */
void PythonTask::
set_function(PyObject *function) {
  Py_XSETREF(_function, Py_NewRef(function));

  if (_function != Py_None && !PyCallable_Check(_function)) {
    nassert_raise("Invalid function passed to PythonTask");
  }
}

/**
 * Replaces the argument list that is passed to the task function.  The
 * parameter should be a tuple or list of arguments, or None to indicate the
 * empty list.
 */
void PythonTask::
set_args(PyObject *args, bool append_task) {
  Py_XDECREF(_args);
  _args = nullptr;

  if (args == Py_None) {
    // None means no arguments; create an empty tuple.
    _args = PyTuple_New(0);
  } else {
    if (PySequence_Check(args)) {
      _args = PySequence_Tuple(args);
    }
  }

  if (_args == nullptr) {
    nassert_raise("Invalid args passed to PythonTask");
    _args = PyTuple_New(0);
  }

  _append_task = append_task;
}

/**
 * Returns the argument list that is passed to the task function.
 */
PyObject *PythonTask::
get_args() {
  if (_append_task) {
    // If we want to append the task, we have to create a new tuple with space
    // for one more at the end.  We have to do this dynamically each time, to
    // avoid storing the task itself in its own arguments list, and thereby
    // creating a cyclical reference.

    int num_args = PyTuple_GET_SIZE(_args);
    PyObject *with_task = PyTuple_New(num_args + 1);
    for (int i = 0; i < num_args; ++i) {
      PyObject *item = PyTuple_GET_ITEM(_args, i);
      PyTuple_SET_ITEM(with_task, i, Py_NewRef(item));
    }

    // Check whether we have a Python wrapper.  This is not the case if the
    // object has been created by C++ and never been exposed to Python code.
    if (__self__ == nullptr) {
      // A __self__ instance does not exist, let's create one now.
      this->ref();
      __self__ = DTool_CreatePyInstance(this, Dtool_PythonTask, true, false);
    }

    PyTuple_SET_ITEM(with_task, num_args, Py_NewRef(__self__));
    return with_task;
  }
  else {
    return Py_NewRef(_args);
  }
}

/**
 * Replaces the function that is called when the task finishes.  The parameter
 * should be a Python callable object.
 */
void PythonTask::
set_upon_death(PyObject *upon_death) {
  Py_XSETREF(_upon_death, Py_NewRef(upon_death));

  if (_upon_death != Py_None && !PyCallable_Check(_upon_death)) {
    nassert_raise("Invalid upon_death function passed to PythonTask");
  }
}

/**
 * Specifies a Python object that serves as the "owner" for the task.  This
 * owner object must have two methods: _addTask() and _clearTask(), which will
 * be called with one parameter, the task object.
 *
 * owner._addTask() is called when the task is added into the active task
 * list, and owner._clearTask() is called when it is removed.
 */
void PythonTask::
set_owner(PyObject *owner) {
#ifndef NDEBUG
  if (owner != Py_None) {
    PyObject *add = PyObject_GetAttrString(owner, "_addTask");
    PyErr_Clear();
    PyObject *clear = PyObject_GetAttrString(owner, "_clearTask");
    PyErr_Clear();

    bool valid_add = false;
    if (add != nullptr) {
      valid_add = PyCallable_Check(add);
      Py_DECREF(add);
    }
    bool valid_clear = false;
    if (clear != nullptr) {
      valid_clear = PyCallable_Check(clear);
      Py_DECREF(clear);
    }

    if (!valid_add || !valid_clear) {
      Dtool_Raise_TypeError("owner object should have _addTask and _clearTask methods");
      return;
    }
  }
#endif

  if (_owner != nullptr && _owner != Py_None && _state != S_inactive) {
    unregister_from_owner();
  }

  Py_XSETREF(_owner, Py_NewRef(owner));

  if (_owner != Py_None && _state != S_inactive) {
    register_to_owner();
  }
}

/**
 * Returns the result of this task's execution, as set by set_result() within
 * the task or returned from a coroutine added to the task manager.  If an
 * exception occurred within this task, it is raised instead.
 */
PyObject *PythonTask::
get_result() const {
  nassertr(done(), nullptr);

  if (_exception == nullptr) {
    // The result of the call is stored in _exc_value.
    return Py_XNewRef(_exc_value);
  }
  else {
    _retrieved_exception = true;
    PyErr_Restore(Py_NewRef(_exception), Py_XNewRef(_exc_value), Py_XNewRef(_exc_traceback));
    return nullptr;
  }
}

/**
 * If an exception occurred during execution of this task, returns it.  This
 * is only set if this task returned a coroutine or generator.
 */
/*PyObject *PythonTask::
exception() const {
  if (_exception == nullptr) {
    return Py_NewRef(Py_None);
  }
  else if (_exc_value == nullptr || _exc_value == Py_None) {
    return PyObject_CallNoArgs(_exception);
  }
  else if (PyTuple_Check(_exc_value)) {
    return PyObject_Call(_exception, _exc_value, nullptr);
  }
  else {
    return PyObject_CallOneArg(_exception, _exc_value);
  }
}*/

/**
 * Maps from an expression like "task.attr_name = v". This is customized here
 * so we can support some traditional task interfaces that supported directly
 * assigning certain values.  We also support adding arbitrary data to the
 * Task object.
 */
int PythonTask::
__setattr__(PyObject *self, PyObject *attr, PyObject *v) {
  if (!PyUnicode_Check(attr)) {
    PyErr_Format(PyExc_TypeError,
                 "attribute name must be string, not '%.200s'",
                 Py_TYPE(attr)->tp_name);
    return -1;
  }

  PyObject *descr = _PyType_Lookup(Py_TYPE(self), attr);
  if (descr != nullptr) {
    descrsetfunc f = Py_TYPE(descr)->tp_descr_set;
    if (f != nullptr) {
      return f(descr, self, v);
    }
  }

  if (task_cat.is_debug()) {
    PyObject *str = PyObject_Repr(v);
    task_cat.debug()
      << *this << ": task."
      << PyUnicode_AsUTF8(attr) << " = "
      << PyUnicode_AsUTF8(str) << "\n";
    Py_DECREF(str);
  }

  return PyDict_SetItem(__dict__, attr, v);
}

/**
 * Maps from an expression like "del task.attr_name". This is customized here
 * so we can support some traditional task interfaces that supported directly
 * assigning certain values.  We also support adding arbitrary data to the
 * Task object.
 */
int PythonTask::
__delattr__(PyObject *self, PyObject *attr) {
  if (PyObject_GenericSetAttr(self, attr, nullptr) == 0) {
    return 0;
  }

  if (!PyErr_ExceptionMatches(PyExc_AttributeError)) {
    return -1;
  }

  PyErr_Clear();

  if (PyDict_DelItem(__dict__, attr) == -1) {
    // PyDict_DelItem does not raise an exception.
    PyErr_Format(PyExc_AttributeError,
                 "'PythonTask' object has no attribute '%U'",
                 attr);
    return -1;
  }

  return 0;
}

/**
 * Maps from an expression like "task.attr_name". This is customized here so
 * we can support some traditional task interfaces that supported directly
 * querying certain values.  We also support adding arbitrary data to the Task
 * object.
 */
PyObject *PythonTask::
__getattribute__(PyObject *self, PyObject *attr) const {
  // We consult the instance dict first, since the user may have overridden a
  // method or something.
  PyObject *item;
  if (PyDict_GetItemRef(__dict__, attr, &item) > 0) {
    return item;
  }

  return PyObject_GenericGetAttr(self, attr);
}

/**
 * Called by Python to implement cycle detection.
 */
int PythonTask::
__traverse__(visitproc visit, void *arg) {
  Py_VISIT(__self__);
  Py_VISIT(_function);
  Py_VISIT(_args);
  Py_VISIT(_upon_death);
  Py_VISIT(_owner);
  Py_VISIT(__dict__);
  Py_VISIT(_generator);
  return 0;
}

/**
 * Called by Python to implement cycle breaking.
 */
int PythonTask::
__clear__() {
  Py_CLEAR(_function);
  Py_CLEAR(_args);
  Py_CLEAR(_upon_death);
  Py_CLEAR(_owner);
  Py_CLEAR(__dict__);
  Py_CLEAR(_generator);

  Py_CLEAR(__self__);
  return 0;
}

/**
 *
 */
bool PythonTask::
unref() const {
  if (!AsyncTask::unref()) {
    // It was cleaned up by the Python garbage collector.
    return false;
  }

  // If the last reference to the object is the one being held by Python,
  // check whether the Python wrapper itself is also at a refcount of 1.
  bool result = true;
  if (get_ref_count() == 1) {
    PyGILState_STATE gstate = PyGILState_Ensure();

    // Check whether we have a Python wrapper.  This is not the case if the
    // object has been created by C++ and never been exposed to Python code.
    PyObject *self = __self__;
    if (self != nullptr) {
      int ref_count = Py_REFCNT(self);
      assert(ref_count > 0);
      if (ref_count == 1) {
        // The last reference to the Python wrapper is being held by us.
        // Break the reference cycle and allow the object to go away.
        if (!AsyncTask::unref()) {
          PyObject_GC_UnTrack(self);
          ((Dtool_PyInstDef *)self)->_memory_rules = false;
          __self__ = nullptr;
          Py_DECREF(self);

          // Let the caller destroy the object.
          result = false;
        }
      }
    }
    PyGILState_Release(gstate);
  }
  return result;
}

/**
 * Cancels this task.  This is equivalent to remove(), except for coroutines,
 * for which it will throw an exception into any currently pending await.
 */
bool PythonTask::
cancel() {
  AsyncTaskManager *manager = _manager;
  if (manager != nullptr) {
    nassertr(_chain->_manager == manager, false);
    if (task_cat.is_debug()) {
      task_cat.debug()
        << "Cancelling " << *this << "\n";
    }

    bool must_cancel = true;
    if (_fut_waiter != nullptr) {
      // Cancel the future that this task is waiting on.  Note that we do this
      // before grabbing the lock, since this operation may also grab it.  This
      // means that _fut_waiter is only protected by the GIL.
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
      // Use PyGILState to protect this asynchronous call.
      PyGILState_STATE gstate;
      gstate = PyGILState_Ensure();
#endif

      // Shortcut for unextended AsyncFuture.
      if (Py_IS_TYPE(_fut_waiter, Dtool_GetPyTypeObject(&Dtool_AsyncFuture))) {
        AsyncFuture *fut = (AsyncFuture *)DtoolInstance_VOID_PTR(_fut_waiter);
        if (!fut->done()) {
          fut->cancel();
        }
        if (fut->done()) {
          // We don't need this anymore.
          Py_DECREF(_fut_waiter);
          _fut_waiter = nullptr;
        }
      }
      else {
        PyObject *result = PyObject_CallMethod(_fut_waiter, "cancel", nullptr);
        Py_XDECREF(result);
      }

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
      PyGILState_Release(gstate);
#endif
      // Keep _fut_waiter in any case, because we may need to cancel it again
      // later if it ignores the cancellation.
    }

    MutexHolder holder(manager->_lock);
    if (_state == S_awaiting) {
      // Reactivate it so that it can receive a CancelledException.
      if (must_cancel) {
        _must_cancel = true;
      }
      _state = AsyncTask::S_active;
      _chain->_active.push_back(this);
      --_chain->_num_awaiting_tasks;
      return true;
    }
    else if (must_cancel || _fut_waiter != nullptr) {
      // We may be polling an external future, so we still need to throw a
      // CancelledException and allow it to be caught.
      if (must_cancel) {
        _must_cancel = true;
      }
      return true;
    }
    else if (_chain->do_remove(this, true)) {
      return true;
    }
    else {
      if (task_cat.is_debug()) {
        task_cat.debug()
          << "  (unable to cancel " << *this << ")\n";
      }
      return false;
    }
  }

  return false;
}

/**
 * Override this function to return true if the task can be successfully
 * executed, false if it cannot.  Mainly intended as a sanity check when
 * attempting to add the task to a task manager.
 *
 * This function is called with the lock held.
 */
bool PythonTask::
is_runnable() {
  return _function != Py_None;
}

/**
 * Override this function to do something useful for the task.
 *
 * This function is called with the lock *not* held.
 */
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

/**
 * The Python calls that implement do_task().  This function is separate so we
 * can acquire the Python interpretor lock while it runs.
 */
AsyncTask::DoneStatus PythonTask::
do_python_task() {
  PyObject *result = nullptr;

  // Are we waiting for a future to finish?  Short-circuit all the logic below
  // by simply calling done().
  {
    PyObject *fut_waiter = _fut_waiter;
    if (fut_waiter != nullptr) {
      PyObject *is_done = PyObject_CallMethod(fut_waiter, "done", nullptr);
      if (is_done == nullptr) {
        return DS_interrupt;
      }
      if (!PyObject_IsTrue(is_done)) {
        // Nope, ask again next frame.
        Py_DECREF(is_done);
        return DS_cont;
      }
      Py_DECREF(is_done);
      Py_DECREF(fut_waiter);
      _fut_waiter = nullptr;
    }
  }

  if (_generator == nullptr) {
    // We are calling the function directly.
    nassertr(_function != nullptr, DS_interrupt);

    PyObject *args = get_args();
    result = PythonThread::call_python_func(_function, args);
    Py_DECREF(args);

    if (result != nullptr && PyGen_Check(result)) {
      // The function has yielded a generator.  We will call into that
      // henceforth, instead of calling the function from the top again.
      if (task_cat.is_debug()) {
        PyObject *str = PyObject_ASCII(_function);
        task_cat.debug()
          << PyUnicode_AsUTF8(str) << " in " << *this
          << " yielded a generator.\n";
        Py_DECREF(str);
      }
      _generator = result;
      result = nullptr;

    } else if (result != nullptr && Py_TYPE(result)->tp_as_async != nullptr) {
      // The function yielded a coroutine, or something of the sort.
      if (task_cat.is_debug()) {
        PyObject *str = PyObject_ASCII(_function);
        PyObject *str2 = PyObject_ASCII(result);
        task_cat.debug()
          << PyUnicode_AsUTF8(str) << " in " << *this
          << " yielded an awaitable: " << PyUnicode_AsUTF8(str2) << "\n";
        Py_DECREF(str);
        Py_DECREF(str2);
      }
      if (PyCoro_CheckExact(result)) {
        // If a coroutine, am_await is possible but senseless, since we can
        // just call send(None) on the coroutine itself.
        _generator = result;
      } else {
        unaryfunc await = Py_TYPE(result)->tp_as_async->am_await;
        _generator = await(result);
        Py_DECREF(result);
      }
      result = nullptr;
    }
  }

  if (_generator != nullptr) {
    if (!_must_cancel) {
      // We are calling a generator.  Use "send" rather than PyIter_Next since
      // we need to be able to read the value from a StopIteration exception.
      PyObject *func = PyObject_GetAttrString(_generator, "send");
      nassertr(func != nullptr, DS_interrupt);
      result = PyObject_CallOneArg(func, Py_None);
      Py_DECREF(func);
    } else {
      // Throw a CancelledError into the generator.
      _must_cancel = false;
      PyObject *exc = PyObject_CallNoArgs(Extension<AsyncFuture>::get_cancelled_error_type());
      PyObject *func = PyObject_GetAttrString(_generator, "throw");
      result = PyObject_CallFunctionObjArgs(func, exc, nullptr);
      Py_DECREF(func);
      Py_DECREF(exc);
    }

    if (result == nullptr) {
      // An error happened.  If StopIteration, that indicates the task has
      // returned.  Otherwise, we need to save it so that it can be re-raised
      // in the function that awaited this task.
      Py_DECREF(_generator);
      _generator = nullptr;

#if PY_VERSION_HEX >= 0x030D0000 // Python 3.13
      // Python 3.13 does not support _PyGen_FetchStopIterationValue anymore.
      if (PyErr_ExceptionMatches(PyExc_StopIteration)) {
        PyObject *exc = PyErr_GetRaisedException();
        result = ((PyStopIterationObject *)exc)->value;
        if (result == nullptr) {
          result = Py_None;
        }
        result = Py_NewRef(result);
        Py_DECREF(exc);
#else
      if (_PyGen_FetchStopIterationValue(&result) == 0) {
#endif
        PyErr_Clear();

        if (_must_cancel) {
          // Task was cancelled right before finishing.  Make sure it is not
          // getting rerun or marked as successfully completed.
          _state = S_servicing_removed;
        }

        // If we passed a coroutine into the task, eg. something like:
        //   taskMgr.add(my_async_function())
        // then we cannot rerun the task, so the return value is always
        // assumed to be DS_done.  Instead, we pass the return value to the
        // result of the `await` expression.
        if (_function == nullptr) {
          if (task_cat.is_debug()) {
            task_cat.debug()
              << *this << " received StopIteration from coroutine.\n";
          }
          // Store the result in _exc_value because that's not used anyway.
          Py_XDECREF(_exc_value);
          _exc_value = result;
          return DS_done;
        }

      } else if (PyErr_ExceptionMatches(Extension<AsyncFuture>::get_cancelled_error_type())) {
        // Someone cancelled the coroutine, and it did not bother to handle it,
        // so we should consider it cancelled.
        if (task_cat.is_debug()) {
          task_cat.debug()
            << *this << " was cancelled and did not catch CancelledError.\n";
        }
        _state = S_servicing_removed;
        PyErr_Clear();
        return DS_done;

      } else if (_function == nullptr) {
        // We got an exception.  If this is a scheduled coroutine, we will
        // keep it and instead throw it into whatever 'awaits' this task.
        // Otherwise, fall through and handle it the regular way.
        Py_XDECREF(_exception);
        Py_XDECREF(_exc_value);
        Py_XDECREF(_exc_traceback);
        PyErr_Fetch(&_exception, &_exc_value, &_exc_traceback);
        _retrieved_exception = false;

        if (task_cat.is_debug()) {
          if (_exception != nullptr && Py_IS_TYPE(_exception, &PyType_Type)) {
            task_cat.debug()
              << *this << " received " << ((PyTypeObject *)_exception)->tp_name << " from coroutine.\n";
          } else {
            task_cat.debug()
              << *this << " received exception from coroutine.\n";
          }
        }

        // Tell the task chain we want to kill ourselves.  We indicate this is
        // a "clean exit" because we still want to run the done callbacks on
        // exception.
        return DS_done;
      }

    } else if (result == Py_None) {
      // Bare yield means to continue next frame.
      Py_DECREF(result);
      return DS_cont;

    } else if (DtoolInstance_Check(result)) {
      // We are waiting for an AsyncFuture (eg. other task) to finish.
      AsyncFuture *fut = (AsyncFuture *)DtoolInstance_UPCAST(result, Dtool_AsyncFuture);
      if (fut != nullptr) {
        // Suspend execution of this task until this other task has completed.
        if (fut != (AsyncFuture *)this && !fut->done()) {
          if (fut->is_task()) {
            // This is actually a task, do we need to schedule it with the
            // manager?  This allows doing something like
            //   await Task.pause(1.0)
            // directly instead of having to do:
            //   await taskMgr.add(Task.pause(1.0))
            AsyncTask *task = (AsyncTask *)fut;
            if (!task->is_alive()) {
              _manager->add(task);
            }
          }
          if (fut->add_waiting_task(this)) {
            if (task_cat.is_debug()) {
              task_cat.debug()
                << *this << " is now awaiting <" << *fut << ">.\n";
            }
          } else {
            // The task is already done.  Continue at next opportunity.
            if (task_cat.is_debug()) {
              task_cat.debug()
                << *this << " would await <" << *fut << ">, were it not already done.\n";
            }
            Py_DECREF(result);
            return DS_cont;
          }
        } else {
          // This is an error.  If we wanted to be fancier we could also
          // detect deeper circular dependencies.
          task_cat.error()
            << *this << " cannot await itself\n";
        }
        // Store the Python object in case we need to cancel it (it may be a
        // subclass of AsyncFuture that overrides cancel() from Python)
        _fut_waiter = result;
        return DS_await;
      }
    } else {
      // We are waiting for a non-Panda future to finish.  We currently
      // implement this by checking every frame whether the future is done.
      PyObject *check = PyObject_GetAttrString(result, "_asyncio_future_blocking");
      if (check != nullptr && check != Py_None) {
        Py_DECREF(check);
        // Next frame, check whether this future is done.
        PyObject *fut_done = PyObject_GetAttrString(result, "done");
        if (fut_done == nullptr || !PyCallable_Check(fut_done)) {
          Py_XDECREF(fut_done);
          task_cat.error()
            << "future.done is not callable\n";
          return DS_interrupt;
        }
        if (task_cat.is_debug()) {
          PyObject *str = PyObject_ASCII(result);
          task_cat.debug()
            << *this << " is now polling " << PyUnicode_AsUTF8(str) << ".done()\n";
          Py_DECREF(str);
        }
        Py_DECREF(fut_done);
        _fut_waiter = result;
        return DS_cont;
      }
      PyErr_Clear();
      Py_XDECREF(check);
    }
  }

  if (result == nullptr) {
    if (PyErr_Occurred() && PyErr_ExceptionMatches(PyExc_SystemExit)) {
      // Don't print an error message for SystemExit.  Or rather, make it a
      // debug message.
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

  if (result == Py_None || _ignore_return) {
    Py_DECREF(result);
    return DS_done;
  }

  if (PyLong_Check(result)) {
    long retval = PyLong_AS_LONG(result);

    switch (retval) {
    case DS_again:
      Py_XDECREF(_generator);
      _generator = nullptr;
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

  // This is unfortunate, but some are returning task.done, which nowadays
  // conflicts with the AsyncFuture method.  Check if that is being returned.
  PyMethodDef *meth = nullptr;
  if (PyCFunction_Check(result)) {
    meth = ((PyCFunctionObject *)result)->m_ml;
  } else if (Py_IS_TYPE(result, &PyMethodDescr_Type)) {
    meth = ((PyMethodDescrObject *)result)->d_method;
  }

  if (meth != nullptr && strcmp(meth->ml_name, "done") == 0) {
    Py_DECREF(result);
    return DS_done;
  }

  std::ostringstream strm;
  PyObject *str = PyObject_ASCII(result);
  if (str == nullptr) {
    str = PyUnicode_FromString("<repr error>");
  }
  strm
    << *this << " returned " << PyUnicode_AsUTF8(str);
  Py_DECREF(str);
  Py_DECREF(result);
  std::string message = strm.str();
  nassert_raise(message);

  return DS_interrupt;
}

/**
 * Override this function to do something useful when the task has been added
 * to the active queue.
 *
 * This function is called with the lock *not* held.
 */
void PythonTask::
upon_birth(AsyncTaskManager *manager) {
  AsyncTask::upon_birth(manager);
  register_to_owner();
}

/**
 * Override this function to do something useful when the task has been
 * removed from the active queue.  The parameter clean_exit is true if the
 * task has been removed because it exited normally (returning DS_done), or
 * false if it was removed for some other reason (e.g.
 * AsyncTaskManager::remove()).  By the time this method is called, _manager
 * has been cleared, so the parameter manager indicates the original
 * AsyncTaskManager that owned this task.
 *
 * The normal behavior is to throw the done_event only if clean_exit is true.
 *
 * This function is called with the lock *not* held.
 */
void PythonTask::
upon_death(AsyncTaskManager *manager, bool clean_exit) {
  AsyncTask::upon_death(manager, clean_exit);

  // If we were polling something when we were removed, get rid of it.
  //TODO: should we call cancel() on it?
  if (_fut_waiter != nullptr) {
    Py_DECREF(_fut_waiter);
    _fut_waiter = nullptr;
  }

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

/**
 * Tells the owner we are now his task.
 */
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

/**
 * Tells the owner we are no longer his task.
 */
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

/**
 * Calls the indicated method name on the given object, if defined, passing in
 * the task object as the only parameter.
 */
void PythonTask::
call_owner_method(const char *method_name) {
  if (_owner != Py_None) {
    PyObject *func = PyObject_GetAttrString(_owner, (char *)method_name);
    if (func == nullptr) {
      task_cat.error()
        << "Owner object added to " << *this << " has no method "
        << method_name << "().\n";

    } else {
      call_function(func);
      Py_DECREF(func);
    }
  }
}

/**
 * Calls the indicated Python function, passing in the task object as the only
 * parameter.
 */
void PythonTask::
call_function(PyObject *function) {
  if (function != Py_None) {
    // Check whether we have a Python wrapper.  This is not the case if the
    // object has been created by C++ and never been exposed to Python code.
    if (__self__ == nullptr) {
      // A __self__ instance does not exist, let's create one now.
      this->ref();
      __self__ = DTool_CreatePyInstance(this, Dtool_PythonTask, true, false);
    }

    PyObject *result = PyObject_CallOneArg(function, __self__);
    Py_XDECREF(result);
  }
}

#endif  // HAVE_PYTHON
