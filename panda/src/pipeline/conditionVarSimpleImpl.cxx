/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file conditionVarSimpleImpl.cxx
 * @author drose
 * @date 2007-06-19
 */

#include "selectThreadImpl.h"

#ifdef THREAD_SIMPLE_IMPL

#include "conditionVarSimpleImpl.h"
#include "threadSimpleImpl.h"

/**
 *
 */
void ConditionVarSimpleImpl::
wait() {
  _mutex.unlock_quietly();

  ThreadSimpleManager *manager = ThreadSimpleManager::get_global_ptr();
  ThreadSimpleImpl *thread = manager->get_current_thread();
  manager->enqueue_block(thread, this);
  manager->next_context();

  _mutex.lock();
}

/**
 *
 */
void ConditionVarSimpleImpl::
wait(double timeout) {
  _mutex.unlock_quietly();

  // TODO.  For now this will release every frame, since we don't have an
  // interface yet on ThreadSimpleManager to do a timed wait.  Maybe that's
  // good enough forever (it does satisfy the condition variable semantics,
  // after all).
  ThreadSimpleManager *manager = ThreadSimpleManager::get_global_ptr();
  ThreadSimpleImpl *thread = manager->get_current_thread();
  manager->enqueue_ready(thread, true);
  manager->next_context();

  _mutex.lock();
}

/**
 *
 */
void ConditionVarSimpleImpl::
do_notify() {
  ThreadSimpleManager *manager = ThreadSimpleManager::get_global_ptr();
  if (manager->unblock_one(this)) {
    // There had been a thread waiting on this condition variable.  Switch
    // contexts immediately, to make fairness more likely.
    ThreadSimpleImpl *thread = manager->get_current_thread();
    manager->enqueue_ready(thread, false);
    manager->next_context();
  }
}

/**
 *
 */
void ConditionVarSimpleImpl::
do_notify_all() {
  ThreadSimpleManager *manager = ThreadSimpleManager::get_global_ptr();
  if (manager->unblock_all(this)) {
    // There had been a thread waiting on this condition variable.  Switch
    // contexts immediately, to make fairness more likely.
    ThreadSimpleImpl *thread = manager->get_current_thread();
    manager->enqueue_ready(thread, false);
    manager->next_context();
  }
}

#endif  // THREAD_SIMPLE_IMPL
