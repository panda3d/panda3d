/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file asyncFuture.I
 * @author rdb
 * @date 2017-11-28
 */

/**
 * Initializes the future in the pending state.
 */
INLINE AsyncFuture::
AsyncFuture() :
  _manager(nullptr),
  _result(nullptr),
  _future_state(FS_pending) {
}

/**
 * Returns true if the future is done or has been cancelled.  It is always
 * safe to call this.
 */
INLINE bool AsyncFuture::
done() const {
  return (FutureState)AtomicAdjust::get(_future_state) >= FS_finished;
}

/**
 * Returns true if the future was cancelled.  It is always safe to call this.
 */
INLINE bool AsyncFuture::
cancelled() const {
  return (FutureState)AtomicAdjust::get(_future_state) == FS_cancelled;
}

/**
 * Sets the event name that will be triggered when the future finishes.  Will
 * not be triggered if the future is cancelled, but it will be triggered for
 * a coroutine task that exits with an exception.
 */
INLINE void AsyncFuture::
set_done_event(const std::string &done_event) {
  nassertv(!done());
  _done_event = done_event;
}

/**
 * Returns the event name that will be triggered when the future finishes.
 * See set_done_event().
 */
INLINE const std::string &AsyncFuture::
get_done_event() const {
  return _done_event;
}

/**
 * Returns this future's result.  Can only be called if done() returns true.
 */
INLINE TypedObject *AsyncFuture::
get_result() const {
  // This is thread safe, since _result may no longer be modified after the
  // state is changed to "done".
  nassertr_always(done(), nullptr);
  return _result;
}

/**
 * Returns this future's result as a pair of TypedObject, ReferenceCount
 * pointers.  Can only be called if done() returns true.
 */
INLINE void AsyncFuture::
get_result(TypedObject *&ptr, ReferenceCount *&ref_ptr) const {
  // This is thread safe, since _result may no longer be modified after the
  // state is changed to "done".
  nassertd(done()) {
    ptr = nullptr;
    ref_ptr = nullptr;
  }
  ptr = _result;
  ref_ptr = _result_ref.p();
}

/**
 * Sets this future's result.  Can only be called if done() returns false.
 */
INLINE void AsyncFuture::
set_result(std::nullptr_t) {
  set_result(nullptr, nullptr);
}

INLINE void AsyncFuture::
set_result(TypedObject *result) {
  set_result(result, nullptr);
}

INLINE void AsyncFuture::
set_result(TypedReferenceCount *result) {
  set_result(result, result);
}

INLINE void AsyncFuture::
set_result(TypedWritableReferenceCount *result) {
  set_result(result, result);
}

INLINE void AsyncFuture::
set_result(const EventParameter &result) {
  set_result(result.get_ptr(), result.get_ptr());
}

/**
 * Creates a new future that returns `done()` when all of the contained
 * futures are done.
 *
 * Calling `cancel()` on the returned future will result in all contained
 * futures that have not yet finished to be cancelled.
 */
INLINE AsyncFuture *AsyncFuture::
gather(Futures futures) {
  if (futures.empty()) {
    AsyncFuture *fut = new AsyncFuture;
    fut->_future_state = (AtomicAdjust::Integer)FS_finished;
    return fut;
  } else if (futures.size() == 1) {
    return futures[0].p();
  } else {
    return (AsyncFuture *)new AsyncGatheringFuture(std::move(futures));
  }
}

/**
 * Tries to atomically lock the future, assuming it is pending.  Returns false
 * if it is not in the pending state, implying it's either done or about to be
 * cancelled.
 */
INLINE bool AsyncFuture::
try_lock_pending() {
  return set_future_state(FS_locked_pending);
}

/**
 * Should be called after try_lock_pending() returns true.
 */
INLINE void AsyncFuture::
unlock(FutureState new_state) {
  nassertv(new_state != FS_locked_pending);
  FutureState orig_state = (FutureState)AtomicAdjust::set(_future_state, (AtomicAdjust::Integer)new_state);
  nassertv(orig_state == FS_locked_pending);
}

/**
 * Atomically changes the future state from pending to another state.  Returns
 * true if successful, false if the future was already done.
 * Note that once a future is in a "done" state (ie. cancelled or finished) it
 * can never change state again.
 */
INLINE bool AsyncFuture::
set_future_state(FutureState state) {
  FutureState orig_state = (FutureState)
    AtomicAdjust::compare_and_exchange(
      _future_state,
      (AtomicAdjust::Integer)FS_pending,
      (AtomicAdjust::Integer)state);

  while (orig_state == FS_locked_pending) {
    Thread::force_yield();
    orig_state = (FutureState)AtomicAdjust::compare_and_exchange(
      _future_state,
      (AtomicAdjust::Integer)FS_pending,
      (AtomicAdjust::Integer)state);
  }

  return orig_state == FS_pending;
}

/**
 * Returns the number of futures that were passed to the constructor.
 */
INLINE size_t AsyncGatheringFuture::
get_num_futures() const {
  return _futures.size();
}

/**
 * Returns the nth future that was passed into the constructor.
 */
INLINE AsyncFuture *AsyncGatheringFuture::
get_future(size_t i) const {
  nassertr(i < _futures.size(), nullptr);
  return _futures[i].p();
}

/**
 * Returns the result of the nth future that was passed into the constructor.
 */
INLINE TypedObject *AsyncGatheringFuture::
get_result(size_t i) const {
  nassertr(i < _futures.size(), nullptr);
  return _futures[i]->get_result();
}
