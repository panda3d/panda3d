/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mutexSimpleImpl.cxx
 * @author drose
 * @date 2007-06-19
 */

#include "selectThreadImpl.h"

#ifdef THREAD_SIMPLE_IMPL

#include "mutexSimpleImpl.h"
#include "threadSimpleImpl.h"
#include "threadSimpleManager.h"

/**
 *
 */
void MutexSimpleImpl::
do_lock() {
  // By the time we get here, we already know that someone else is holding the
  // lock: (_flags & F_lock_count) != 0.
  ThreadSimpleManager *manager = ThreadSimpleManager::get_global_ptr();
  ThreadSimpleImpl *thread = manager->get_current_thread();

  while ((_flags & F_lock_count) != 0) {
    manager->enqueue_block(thread, this);
    manager->next_context();
  }

  _flags |= F_lock_count;
}

/**
 *
 */
void MutexSimpleImpl::
do_unlock() {
  // By the time we get here, we already know that someone else is blocked on
  // this mutex: (_flags & F_waiters) != 0.
  ThreadSimpleManager *manager = ThreadSimpleManager::get_global_ptr();
  if (manager->unblock_one(this)) {
    // There had been a thread waiting on this mutex.  Switch contexts
    // immediately, to make fairness more likely.
    ThreadSimpleImpl *thread = manager->get_current_thread();
    manager->enqueue_ready(thread, false);
    manager->next_context();
  }
}

/**
 *
 */
void MutexSimpleImpl::
do_unlock_quietly() {
  ThreadSimpleManager *manager = ThreadSimpleManager::get_global_ptr();
  manager->unblock_one(this);
}

#endif  // THREAD_SIMPLE_IMPL
