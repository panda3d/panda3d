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
#include "conditionVarFull.h"
#include "config_event.h"
#include "pStatTimer.h"
#include "throw_event.h"

TypeHandle AsyncFuture::_type_handle;

/**
 * Destroys the future.  Assumes notify_done() has already been called.
 */
AsyncFuture::
~AsyncFuture() {
  delete _cvar;
  nassertv(_waiting_tasks.empty());
}

/**
 * Cancels the future.  Returns true if it was cancelled, or false if the
 * future was already done.
 * In the case of a task, this is equivalent to remove().
 */
bool AsyncFuture::
cancel() {
  if (!done()) {
    if (_manager == nullptr) {
      _manager = AsyncTaskManager::get_global_ptr();
    }
    MutexHolder holder(_manager->_lock);
    notify_done(false);
    return true;
  } else {
    return false;
  }
}

/**
 *
 */
void AsyncFuture::
output(ostream &out) const {
  out << get_type();
}

/**
 * Waits until the future is done.
 */
void AsyncFuture::
wait() {
  if (done()) {
    return;
  }

  // If we don't have a manager, use the global one.
  if (_manager == nullptr) {
    _manager = AsyncTaskManager::get_global_ptr();
  }

  MutexHolder holder(_manager->_lock);
  if (!done()) {
    if (_cvar == nullptr) {
      _cvar = new ConditionVarFull(_manager->_lock);
    }
    if (task_cat.is_debug()) {
      task_cat.debug()
        << "Waiting for future " << *this << "\n";
    }
    PStatTimer timer(AsyncTaskChain::_wait_pcollector);
    while (!done()) {
      _cvar->wait();
    }
  }
}

/**
 * Waits until the future is done, or until the timeout is reached.
 */
void AsyncFuture::
wait(double timeout) {
  if (done()) {
    return;
  }

  // If we don't have a manager, use the global one.
  if (_manager == nullptr) {
    _manager = AsyncTaskManager::get_global_ptr();
  }

  MutexHolder holder(_manager->_lock);
  if (!done()) {
    if (_cvar == nullptr) {
      _cvar = new ConditionVarFull(_manager->_lock);
    }
    if (task_cat.is_debug()) {
      task_cat.debug()
        << "Waiting up to " << timeout << " seconds for future " << *this << "\n";
    }
    PStatTimer timer(AsyncTaskChain::_wait_pcollector);
    _cvar->wait(timeout);
  }
}

/**
 * Schedules the done callbacks.  Assumes the manager's lock is held, and that
 * the future is currently in the 'pending' state.
 * @param clean_exit true if finished successfully, false if cancelled.
 */
void AsyncFuture::
notify_done(bool clean_exit) {
  nassertv(_manager != nullptr);
  nassertv(_manager->_lock.debug_is_locked());
  nassertv(_future_state == FS_pending);

  pvector<AsyncTask *>::iterator it;
  for (it = _waiting_tasks.begin(); it != _waiting_tasks.end(); ++it) {
    AsyncTask *task = *it;
    nassertd(task->_manager == _manager) continue;
    task->_state = AsyncTask::S_active;
    task->_chain->_active.push_back(task);
    --task->_chain->_num_awaiting_tasks;
    unref_delete(task);
  }
  _waiting_tasks.clear();

  // For historical reasons, we don't send the "done event" if the future was
  // cancelled.
  if (clean_exit && !_done_event.empty()) {
    PT_Event event = new Event(_done_event);
    event->add_parameter(EventParameter(this));
    throw_event(move(event));
  }

  nassertv_always(FS_pending ==
    (FutureState)AtomicAdjust::compare_and_exchange(
      _future_state,
      (AtomicAdjust::Integer)FS_pending,
      (AtomicAdjust::Integer)(clean_exit ? FS_finished : FS_cancelled)));

  // Finally, notify any threads that may be waiting.
  if (_cvar != nullptr) {
    _cvar->notify_all();
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
  nassertv(!done());
  // If we don't have a manager, use the global one.
  if (_manager == nullptr) {
    _manager = AsyncTaskManager::get_global_ptr();
  }
  MutexHolder holder(_manager->_lock);
  _result = ptr;
  _result_ref = ref_ptr;
  notify_done(true);
}

/**
 * Indicates that the given task is waiting for this future to complete.  When
 * the future is done, it will reactivate the given task.
 * Assumes the manager's lock is held.
 */
void AsyncFuture::
add_waiting_task(AsyncTask *task) {
  nassertv(!done());
  nassertv(_manager != nullptr);
  nassertv(_manager->_lock.debug_is_locked());
  task->ref();
  _waiting_tasks.push_back(task);
  nassertv(task->_manager == _manager);
}
