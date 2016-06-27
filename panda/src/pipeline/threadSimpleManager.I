/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file threadSimpleManager.I
 * @author drose
 * @date 2007-06-19
 */

/**
 *
 */
INLINE ThreadSimpleImpl *ThreadSimpleManager::
get_current_thread() {
  return _current_thread;
}

/**
 * Returns true if we are still running within the same OS-level thread we
 * think we should be running in, or false if this appears to be running in a
 * different thread.
 */
INLINE bool ThreadSimpleManager::
is_same_system_thread() const {
  return _current_thread->is_same_system_thread();
}

/**
 *
 */
INLINE ThreadSimpleManager *ThreadSimpleManager::
get_global_ptr() {
  if (!_pointers_initialized) {
    init_pointers();
  }
  return _global_ptr;
}

/**
 * STL function object to sort the priority queue of sleeping threads.
 */
INLINE bool ThreadSimpleManager::CompareStartTime::
operator ()(ThreadSimpleImpl *a, ThreadSimpleImpl *b) const {
  return a->get_wake_time() > b->get_wake_time();
}
