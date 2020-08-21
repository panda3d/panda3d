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
#include "eventParameter.h"
#include "atomicAdjust.h"

class AsyncTaskManager;
class AsyncTask;

/**
 * This class represents a thread-safe handle to a promised future result of
 * an asynchronous operation, providing methods to query its status and result
 * as well as register callbacks for this future's completion.
 *
 * An AsyncFuture can be awaited from within a coroutine or task.  It keeps
 * track of tasks waiting for this future and automatically reactivates them
 * upon this future's completion.
 *
 * A task itself is also a subclass of AsyncFuture.  Other subclasses are
 * not generally necessary, except to override the function of `cancel()`.
 *
 * Until the future is done, it is "owned" by the resolver thread, though it's
 * still legal for other threads to query its state.  When the resolver thread
 * resolves this future using `set_result()`, or any thread calls `cancel()`,
 * it instantly enters the "done" state, after which the result becomes a
 * read-only field that all threads can access.
 *
 * When the future returns true for done(), a thread can use cancelled() to
 * determine whether the future was cancelled or get_result() to access the
 * result of the operation.  Not all operations define a meaningful result
 * value, so some will always return nullptr.
 *
 * In Python, the `cancelled()`, `wait()` and `get_result()` methods are
 * wrapped up into a single `result()` method which waits for the future to
 * complete before either returning the result or throwing an exception if the
 * future was cancelled.
 * However, it is preferable to use the `await` keyword when running from a
 * coroutine, which only suspends the current task and not the entire thread.
 *
 * This API aims to mirror and be compatible with Python's Future class.
 *
 * @since 1.10.0
 */
class EXPCL_PANDA_EVENT AsyncFuture : public TypedReferenceCount {
PUBLISHED:
  INLINE AsyncFuture();
  virtual ~AsyncFuture();

  EXTENSION(static PyObject *__await__(PyObject *self));
  EXTENSION(static PyObject *__iter__(PyObject *self));

  INLINE bool done() const;
  INLINE bool cancelled() const;
  EXTENSION(PyObject *result(PyObject *timeout = Py_None) const);

  virtual bool cancel();

  INLINE void set_done_event(const std::string &done_event);
  INLINE const std::string &get_done_event() const;
  MAKE_PROPERTY(done_event, get_done_event, set_done_event);

  EXTENSION(PyObject *add_done_callback(PyObject *self, PyObject *fn));

  EXTENSION(static PyObject *gather(PyObject *args));

  virtual void output(std::ostream &out) const;

  BLOCKING void wait();
  BLOCKING void wait(double timeout);

  INLINE void set_result(std::nullptr_t);
  INLINE void set_result(TypedObject *result);
  INLINE void set_result(TypedReferenceCount *result);
  INLINE void set_result(TypedWritableReferenceCount *result);
  INLINE void set_result(const EventParameter &result);

public:
  void set_result(TypedObject *ptr, ReferenceCount *ref_ptr);

  INLINE TypedObject *get_result() const;
  INLINE void get_result(TypedObject *&ptr, ReferenceCount *&ref_ptr) const;

  typedef pvector<PT(AsyncFuture)> Futures;
  INLINE static AsyncFuture *gather(Futures futures);

  virtual bool is_task() const {return false;}

  void notify_done(bool clean_exit);
  bool add_waiting_task(AsyncTask *task);

private:
  void wake_task(AsyncTask *task);

protected:
  enum FutureState {
    // Pending states
    FS_pending,
    FS_locked_pending,

    // Done states
    FS_finished,
    FS_cancelled,
  };
  INLINE bool try_lock_pending();
  INLINE void unlock(FutureState new_state = FS_pending);
  INLINE bool set_future_state(FutureState state);

  AsyncTaskManager *_manager;
  TypedObject *_result;
  PT(ReferenceCount) _result_ref;
  AtomicAdjust::Integer _future_state;

  std::string _done_event;

  // Tasks and gathering futures waiting for this one to complete.
  Futures _waiting;

  friend class AsyncGatheringFuture;
  friend class AsyncTaskChain;
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

INLINE std::ostream &operator << (std::ostream &out, const AsyncFuture &fut) {
  fut.output(out);
  return out;
};

/**
 * Specific future that collects the results of several futures.
 */
class EXPCL_PANDA_EVENT AsyncGatheringFuture final : public AsyncFuture {
private:
  AsyncGatheringFuture(AsyncFuture::Futures futures);

public:
  virtual bool cancel() override;

  INLINE size_t get_num_futures() const;
  INLINE AsyncFuture *get_future(size_t i) const;
  INLINE TypedObject *get_result(size_t i) const;

private:
  const Futures _futures;
  AtomicAdjust::Integer _num_pending;

  friend class AsyncFuture;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AsyncFuture::init_type();
    register_type(_type_handle, "AsyncGatheringFuture",
                  AsyncFuture::get_class_type());
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() override {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "asyncFuture.I"

#endif
