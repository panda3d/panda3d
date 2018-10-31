/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file threadSimpleImpl.I
 * @author drose
 * @date 2007-06-18
 */

/**
 *
 */
INLINE Thread *ThreadSimpleImpl::
get_current_thread() {
  return ThreadSimpleManager::get_global_ptr()->get_current_thread()->_parent_obj;
}

/**
 * Returns true if we are still running within the same OS-level thread that
 * this thread begin in, or false if this appears to be running in a different
 * thread.
 */
INLINE bool ThreadSimpleImpl::
is_same_system_thread() const {
#ifdef HAVE_POSIX_THREADS
  return pthread_equal(_posix_system_thread_id, pthread_self());
#endif
#ifdef WIN32
  return (_win32_system_thread_id == GetCurrentThreadId());
#endif
  return true;
}

/**
 * Associates the indicated Thread object with the currently-executing thread.
 * You should not call this directly; use Thread::bind_thread() instead.
 */
INLINE void ThreadSimpleImpl::
bind_thread(Thread *) {
}

/**
 *
 */
INLINE bool ThreadSimpleImpl::
is_threading_supported() {
  return true;
}

/**
 *
 */
INLINE bool ThreadSimpleImpl::
is_simple_threads() {
  return true;
}

/**
 *
 */
INLINE void ThreadSimpleImpl::
sleep(double seconds) {
  ThreadSimpleManager *manager = ThreadSimpleManager::get_global_ptr();
  if (manager->is_same_system_thread()) {
    ThreadSimpleImpl *thread = manager->get_current_thread();
    thread->sleep_this(seconds);
  } else {
    manager->system_sleep(seconds);
  }
}

/**
 *
 */
INLINE void ThreadSimpleImpl::
yield() {
  ThreadSimpleManager *manager = ThreadSimpleManager::get_global_ptr();
  if (manager->is_same_system_thread()) {
    ThreadSimpleImpl *thread = manager->get_current_thread();
    thread->yield_this(true);
  } else {
    manager->system_yield();
  }
}

/**
 *
 */
INLINE void ThreadSimpleImpl::
consider_yield() {
  ThreadSimpleManager *manager = ThreadSimpleManager::get_global_ptr();
  if (manager->is_same_system_thread()) {
    ThreadSimpleImpl *thread = manager->get_current_thread();
    thread->consider_yield_this();
  }
}

/**
 *
 */
INLINE void ThreadSimpleImpl::
consider_yield_this() {
  double now = _manager->get_current_time();
  if (now >= _stop_time) {
    yield_this(false);
  }
}

/**
 *
 */
INLINE double ThreadSimpleImpl::
get_wake_time() const {
  return _wake_time;
}

/**
 * Writes a list of threads running and threads blocked.
 */
void ThreadSimpleImpl::
write_status(std::ostream &out) {
  ThreadSimpleManager *manager = ThreadSimpleManager::get_global_ptr();
  manager->write_status(out);
}
