/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file asyncFuture.cxx
 * @author rdb
 * @date 2017-11-28
 */

#include "asyncFuture.h"
#include "asyncTask.h"
#include "asyncTaskManager.h"
#include "config_event.h"
#include "pStatTimer.h"
#include "throw_event.h"
#include "trueClock.h"

TypeHandle AsyncFuture::_type_handle;
TypeHandle AsyncGatheringFuture::_type_handle;

/**
 * Destroys the future.  Assumes notify_done() has already been called.
 */
AsyncFuture::
~AsyncFuture() {
  // If this triggers, the future destroyed before it was cancelled, which is
  // not valid.  Unless we should simply call cancel() here?
  nassertv(_waiting.empty());

  // This is an attempt to work around what appears to be a compiler bug in
  // MSVC when compiling with optimizations and having an EventStoreInt stored
  // in this field.  It crashes when we delete via the ReferenceCount base
  // instead of via the TypedObject.  I haven't been able to find out why;
  // just that it doesn't happen with ParamString. ~rdb
  ReferenceCount *result_ref = _result_ref.p();
  if (result_ref != nullptr) {
    _result_ref.cheat() = nullptr;
    if (!result_ref->unref()) {
      patomic_thread_fence(std::memory_order_acquire);
      delete _result;
    }
    _result = nullptr;
  }
}

/**
 * Cancels the future.  Returns true if it was cancelled, or false if the
 * future was already done.  Either way, done() will return true after this
 * call returns.
 *
 * Please note that calling this is not a guarantee that the operation
 * corresponding this future does not run.  It could already be in the process
 * of running, or perhaps not respond to a cancel signal.  All this guarantees
 * is that the future is marked as done when this call returns.
 *
 * In the case of a task, this is equivalent to remove().
 */
bool AsyncFuture::
cancel() {
  if (set_future_state(FS_cancelled)) {
    // The compare-swap operation succeeded, so schedule the callbacks.
    notify_done(false);
    return true;
  } else {
    // It's already done.
    return false;
  }
}

/**
 *
 */
void AsyncFuture::
output(std::ostream &out) const {
  out << get_type();
  switch (get_future_state()) {
  case FS_pending:
  case FS_locked_pending:
    out << " (pending)";
    break;
  case FS_finished:
    out << " (finished)";
    break;
  case FS_cancelled:
    out << " (cancelled)";
    break;
  default:
    out << " (**INVALID**)";
    break;
  }
}

/**
 * Waits until the future is done.
 */
void AsyncFuture::
wait() {
  if (!done()) {
    PStatTimer timer(AsyncTaskChain::_wait_pcollector);
    if (task_cat.is_debug()) {
      task_cat.debug()
        << "Waiting for future " << *this << "\n";
    }

    FutureState state = get_future_state();
    while (state < FS_finished) {
      if (state == FS_locked_pending) {
        // If it's locked, someone is probably about to finish it, so don't
        // go to sleep.
        do {
          Thread::relax();
          state = get_future_state();
        }
        while (state == FS_locked_pending);

        if (state >= FS_finished) {
          return;
        }
      }

      // Go to sleep.
      _future_state.wait(state, std::memory_order_relaxed);
      state = get_future_state();
    }
  }

  // Let's make wait() an acquire op, so that people can use a future for
  // synchronization.
  patomic_thread_fence(std::memory_order_acquire);
}

/**
 * Waits until the future is done, or until the timeout is reached.  Note that
 * this can be considerably less efficient than wait() without a timeout, so
 * it's generally not a good idea to use this unless you really need to.
 */
void AsyncFuture::
wait(double timeout) {
  if (!done()) {
    PStatTimer timer(AsyncTaskChain::_wait_pcollector);
    if (task_cat.is_debug()) {
      task_cat.debug()
        << "Waiting up to " << timeout << " seconds for future " << *this << "\n";
    }

    // Continue to yield while the future isn't done.  It may be more efficient
    // to use a condition variable, but let's not add the extra complexity
    // unless we're sure that we need it.
    TrueClock *clock = TrueClock::get_global_ptr();
    double end = clock->get_short_time() + timeout;
    do {
      Thread::relax();
    }
    while (!done() && clock->get_short_time() < end);

    patomic_thread_fence(std::memory_order_acquire);
  }
}

/**
 * Schedules the done callbacks.  Called after the future has just entered the
 * 'done' state.
 * @param clean_exit true if finished successfully, false if cancelled.
 */
void AsyncFuture::
notify_done(bool clean_exit) {
  patomic_thread_fence(std::memory_order_acquire);
  nassertv(done());

  // Let any calls to wait() know that we're done.
  _future_state.notify_all();

  // This will only be called by the thread that managed to set the
  // _future_state away from the "pending" state, so this is thread safe.

  // Go through the futures that are waiting for this to finish.
  for (AsyncFuture *fut : _waiting) {
    if (fut->is_task()) {
      // It's a task.  Make it active again.
      wake_task((AsyncTask *)fut);
    }
    else if (fut->get_type() == AsyncGatheringFuture::get_class_type()) {
      // It's a gathering future.  Decrease the pending count on it, and if
      // we're the last one, call notify_done() on it.
      AsyncGatheringFuture *gather = (AsyncGatheringFuture *)fut;
      if (gather->_num_pending.fetch_sub(1, std::memory_order_relaxed) == 1) {
        if (gather->set_future_state(FS_finished)) {
          gather->notify_done(true);
        }
      }
    }
    else {
      // It's a shielding future.  The shielding only protects the inner future
      // when the outer is cancelled, not the other way around, so we have to
      // propagate any cancellation here as well.
      if (clean_exit && _result != nullptr) {
        // Propagate the result, if any.
        if (fut->try_lock_pending()) {
          fut->_result = _result;
          fut->_result_ref = _result_ref;
          fut->unlock(FS_finished);
          fut->notify_done(true);
        }
      }
      else if (fut->set_future_state(clean_exit ? FS_finished : FS_cancelled)) {
        fut->notify_done(clean_exit);
      }
    }
  }
  _waiting.clear();

  // For historical reasons, we don't send the "done event" if the future was
  // cancelled.
  if (clean_exit && !_done_event.empty()) {
    PT_Event event = new Event(_done_event);
    event->add_parameter(EventParameter(this));
    throw_event(std::move(event));
  }
}

/**
 * Sets this future's result as a generic TypedObject.
 */
void AsyncFuture::
set_result(TypedObject *result) {
  if (result->is_of_type(TypedWritableReferenceCount::get_class_type())) {
    set_result((TypedWritableReferenceCount *)result);
  }
  else if (result->is_of_type(TypedReferenceCount::get_class_type())) {
    set_result((TypedReferenceCount *)result);
  }
  else {
    set_result(result, nullptr);
  }
}

/**
 * Sets this future's result.  Can only be done while the future is not done.
 * Calling this marks the future as done and schedules the done callbacks.
 *
 * This variant takes two pointers; the second one is only set if this object
 * inherits from ReferenceCount, so that a reference can be held.
 *
 * Assumes the manager's lock is *not* held.
 */
void AsyncFuture::
set_result(TypedObject *ptr, ReferenceCount *ref_ptr) {
  // We don't strictly need to lock the future since only one thread is
  // allowed to call set_result(), but we might as well.
  patomic_unsigned_lock_free::value_type orig_state = FS_pending;
  if (!_future_state.compare_exchange_strong(orig_state, FS_locked_pending,
                                             std::memory_order_relaxed)) {
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
    while (orig_state == FS_locked_pending) {
      Thread::relax();
      orig_state = FS_pending;
      _future_state.compare_exchange_weak(orig_state, FS_locked_pending,
                                          std::memory_order_relaxed);
    }
#else
    // We can't lose control between now and calling unlock() if we're using a
    // cooperative threading model.
    nassertv(false);
#endif
  }

  if (orig_state == FS_pending) {
    _result = ptr;
    _result_ref = ref_ptr;
    unlock(FS_finished);

    // OK, now our thread owns the _waiting vector et al.
    notify_done(true);
  }
  else if (orig_state == FS_cancelled) {
    // This was originally illegal, but there is a chance that the future was
    // cancelled while another thread was setting the result.  So, we drop
    // this, but we can issue a warning.
    task_cat.warning()
      << "Ignoring set_result() called on cancelled " << *this << "\n";
  }
  else {
    task_cat.error()
      << "set_result() was called on finished " << *this << "\n";
  }
}

/**
 * Indicates that the given task is waiting for this future to complete.  When
 * the future is done, it will reactivate the given task.  If this is called
 * while the future is already done, schedules the task immediately.
 * Assumes the manager's lock is not held.
 * @returns true if the future was pending, false if it was already done.
 */
bool AsyncFuture::
add_waiting_task(AsyncTask *task) {
  nassertr(task->is_runnable(), false);

  // We have to make sure we're not going to change state while we're in the
  // process of adding the task.
  if (try_lock_pending()) {
    if (_manager == nullptr) {
      _manager = task->_manager;
    }

    _waiting.push_back(task);

    // Unlock the state.
    unlock();
    nassertr(task->_manager == nullptr || task->_manager == _manager, true);
    return true;
  } else {
    // It's already done.  Wake the task immediately.
    wake_task(task);
    return false;
  }
}

/**
 * Reactivates the given task.  Assumes the manager lock is not held.
 */
void AsyncFuture::
wake_task(AsyncTask *task) {
  AsyncTaskManager *manager = task->_manager;
  if (manager == nullptr) {
    // If it's an unscheduled task, schedule it on the same manager as the
    // rest of the waiting tasks.
    manager = _manager;
    if (manager == nullptr) {
      manager = AsyncTaskManager::get_global_ptr();
    }
  }

  MutexHolder holder(manager->_lock);
  switch (task->_state) {
  case AsyncTask::S_servicing_removed:
    nassertv(task->_manager == _manager);
    // Re-adding a self-removed task; this just means clearing the removed
    // flag.
    task->_state = AsyncTask::S_servicing;
    return;

  case AsyncTask::S_active:
    // It could have already been activated, such as by a cancel() which then
    // indirectly caused the awaiting future to be cancelled.  Do nothing.
    return;

  case AsyncTask::S_inactive:
    // Schedule it immediately.
    nassertv(task->_manager == nullptr);

    if (task_cat.is_debug()) {
      task_cat.debug()
        << "Adding " << *task << " (woken by future " << *this << ")\n";
    }

    {
      manager->_lock.unlock();
      task->upon_birth(manager);
      manager->_lock.lock();
      nassertv(task->_manager == nullptr &&
               task->_state == AsyncTask::S_inactive);

      AsyncTaskChain *chain = manager->do_find_task_chain(task->_chain_name);
      if (chain == nullptr) {
        task_cat.warning()
          << "Creating implicit AsyncTaskChain " << task->_chain_name
          << " for " << manager->get_type() << " " << manager->get_name() << "\n";
        chain = manager->do_make_task_chain(task->_chain_name);
      }
      chain->do_add(task);
    }
    return;

  case AsyncTask::S_awaiting:
    nassertv(task->_manager == _manager);
    task->_state = AsyncTask::S_active;
    task->_chain->_active.push_back(task);
    --task->_chain->_num_awaiting_tasks;
    return;

  default:
    nassert_raise("unexpected task state");
    return;
  }
}

/**
 * Internal callback called when a CompletionToken created from this future
 * completes.
 */
void AsyncFuture::
token_callback(Completable::Data *data, bool success) {
  AsyncFuture *future = (AsyncFuture *)data;
  future->set_result(EventParameter(success));
  unref_delete(future);
}

/**
 * @see AsyncFuture::gather
 */
AsyncGatheringFuture::
AsyncGatheringFuture(AsyncFuture::Futures futures) :
  _futures(std::move(futures)),
  _num_pending(0) {

  bool any_pending = false;

  for (AsyncFuture *fut : _futures) {
    // If this returns true, the future is not yet done and we need to
    // register ourselves with it.  This creates a circular reference, but it
    // is resolved when the future is completed or cancelled.
    if (fut->try_lock_pending()) {
      if (_manager == nullptr) {
        _manager = fut->_manager;
      }
      fut->_waiting.push_back((AsyncFuture *)this);
      _num_pending.fetch_add(1, std::memory_order_relaxed);
      fut->unlock();
      any_pending = true;
    }
  }
  if (!any_pending) {
    // Start in the done state if all the futures we were passed are done.
    // Note that it is only safe to set this member in this manner if indeed
    // no other future holds a reference to us.
    _future_state.store(FS_finished, std::memory_order_relaxed);
  }
}

/**
 * Cancels all the futures.  Returns true if any futures were cancelled.
 * Makes sure that all the futures finish before this one is marked done, in
 * order to maintain the guarantee that calling result() is safe when done()
 * returns true.
 */
bool AsyncGatheringFuture::
cancel() {
  if (!done()) {
    if (task_cat.is_debug()) {
      task_cat.debug()
        << "Cancelling AsyncGatheringFuture (" << _futures.size() << " futures)\n";
    }

    // Temporarily increase the pending count so that the notify_done()
    // callbacks won't end up causing it to be set to "finished".
    _num_pending.fetch_add(1, std::memory_order_relaxed);

    bool any_cancelled = false;
    for (AsyncFuture *fut : _futures) {
      if (fut->cancel()) {
        any_cancelled = true;
      }
    }

    // If all the futures were cancelled, change state of this future to
    // "cancelled" and call the notify_done() callbacks.
    if (_num_pending.fetch_sub(1, std::memory_order_relaxed) == 1) {
      if (set_future_state(FS_cancelled)) {
        notify_done(false);
      }
    }

    return any_cancelled;
  } else {
    return false;
  }
}
