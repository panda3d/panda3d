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
  _cvar(nullptr),
  _future_state(FS_pending),
  _result(nullptr) {
}

/**
 * Returns true if the future is done or has been cancelled.  It is always
 * safe to call this.
 */
INLINE bool AsyncFuture::
done() const {
  return (FutureState)AtomicAdjust::get(_future_state) != FS_pending;
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
set_done_event(const string &done_event) {
  _done_event = done_event;
}

/**
 * Returns the event name that will be triggered when the future finishes.
 * See set_done_event().
 */
INLINE const string &AsyncFuture::
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
set_result(nullptr_t) {
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
