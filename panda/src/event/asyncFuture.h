/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file asyncFuture.h
 * @author rdb
 * @date 2017-11-28
 */

#ifndef ASYNCFUTURE_H
#define ASYNCFUTURE_H

#include "pandabase.h"
#include "typedReferenceCount.h"
#include "typedWritableReferenceCount.h"
#include "conditionVar.h"
#include "atomicAdjust.h"

class AsyncTaskManager;
class AsyncTask;
class ConditionVarFull;

/**
 * This class represents a thread-safe handle to a promised future result of
 * an asynchronous operation, providing methods to query its status and result
 * as well as register callbacks for this future's completion.
 *
 * An AsyncFuture can be awaited from within a coroutine or task.  It keeps
 * track of a list of tasks waiting for this future so that they are
 * automatically reactivated upon this future's completion.
 *
 * A task itself is also a subclass of AsyncFuture.  Other subclasses are
 * possible, although subclassing is not necessary for most purposes.
 *
 * The `done()` method is used to check whether the future has completed and
 * a result is available (whether it is cancelled or finished).
 *
 * To get the result of the future in C++, use `wait()` and `get_result()`.
 * In Python, the functionality of both of those calls is wrapped into the
 * `result()` method, which waits for the future to complete before either
 * returning the result or throwing an exception if the future was cancelled.
 * However, it is preferable to use the `await` keyword when running from a
 * coroutine.
 *
 * This API aims to mirror and be compatible with Python's Future class.
 */
class EXPCL_PANDA_EVENT AsyncFuture : public TypedReferenceCount {
PUBLISHED:
  INLINE AsyncFuture();
  virtual ~AsyncFuture();

  EXTENSION(static PyObject *__await__(PyObject *self));
  EXTENSION(static PyObject *__iter__(PyObject *self));

  INLINE bool done() const;
  INLINE bool cancelled() const;
  EXTENSION(PyObject *result(double timeout = -1.0) const);

  virtual bool cancel();

  INLINE void set_done_event(const string &done_event);
  INLINE const string &get_done_event() const;
  MAKE_PROPERTY(done_event, get_done_event, set_done_event);

  virtual void output(ostream &out) const;

  void wait();
  void wait(double timeout);

  INLINE void set_result(nullptr_t);
  INLINE void set_result(TypedObject *result);
  INLINE void set_result(TypedReferenceCount *result);
  INLINE void set_result(TypedWritableReferenceCount *result);

public:
  INLINE TypedObject *get_result() const;
  INLINE void get_result(TypedObject *&ptr, ReferenceCount *&ref_ptr) const;

  void notify_done(bool clean_exit);

private:
  void set_result(TypedObject *ptr, ReferenceCount *ref_ptr);
  void add_waiting_task(AsyncTask *task);

protected:
  enum FutureState {
    FS_pending,
    FS_finished,
    FS_cancelled,
  };

  AsyncTaskManager *_manager;
  ConditionVarFull *_cvar;
  TypedObject *_result;
  PT(ReferenceCount) _result_ref;
  AtomicAdjust::Integer _future_state;

  string _done_event;

  // Tasks waiting for this one to complete.  These are reference counted, but
  // we can't store them in a PointerTo for circular dependency reasons.
  pvector<AsyncTask *> _waiting_tasks;

  friend class PythonTask;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "AsyncFuture",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const AsyncFuture &fut) {
  fut.output(out);
  return out;
};

#include "asyncFuture.I"

#endif
