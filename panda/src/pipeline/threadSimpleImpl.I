// Filename: threadSimpleImpl.I
// Created by:  drose (18Jun07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleImpl::get_current_thread
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE Thread *ThreadSimpleImpl::
get_current_thread() {
  return ThreadSimpleManager::get_global_ptr()->get_current_thread()->_parent_obj;
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleImpl::is_same_system_thread
//       Access: Public
//  Description: Returns true if we are still running within the same
//               OS-level thread that this thread begin in, or false
//               if this appears to be running in a different thread.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleImpl::bind_thread
//       Access: Public, Static
//  Description: Associates the indicated Thread object with the
//               currently-executing thread.  You should not call this
//               directly; use Thread::bind_thread() instead.
////////////////////////////////////////////////////////////////////
INLINE void ThreadSimpleImpl::
bind_thread(Thread *) {
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleImpl::is_threading_supported
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool ThreadSimpleImpl::
is_threading_supported() {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleImpl::is_true_threads
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool ThreadSimpleImpl::
is_true_threads() {
  return (is_os_threads != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleImpl::is_simple_threads
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool ThreadSimpleImpl::
is_simple_threads() {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleImpl::sleep
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleImpl::yield
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleImpl::consider_yield
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ThreadSimpleImpl::
consider_yield() {
  ThreadSimpleManager *manager = ThreadSimpleManager::get_global_ptr();
  if (manager->is_same_system_thread()) {
    ThreadSimpleImpl *thread = manager->get_current_thread();
    thread->consider_yield_this();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleImpl::consider_yield_this
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ThreadSimpleImpl::
consider_yield_this() {
  double now = _manager->get_current_time();
  if (now >= _stop_time) {
    yield_this(false);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleImpl::get_wake_time
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE double ThreadSimpleImpl::
get_wake_time() const {
  return _wake_time;
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleImpl::write_status
//       Access: Public, Static
//  Description: Writes a list of threads running and threads blocked.
////////////////////////////////////////////////////////////////////
void ThreadSimpleImpl::
write_status(ostream &out) {
  ThreadSimpleManager *manager = ThreadSimpleManager::get_global_ptr();
  manager->write_status(out);
}
