// Filename: threadSimpleManager.I
// Created by:  drose (19Jun07)
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
//     Function: ThreadSimpleManager::get_current_thread
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ThreadSimpleImpl *ThreadSimpleManager::
get_current_thread() {
  return _current_thread;
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleManager::is_same_system_thread
//       Access: Public
//  Description: Returns true if we are still running within the same
//               OS-level thread that created the ThreadSimpleManager,
//               or false if this appears to be running in a different
//               thread.
////////////////////////////////////////////////////////////////////
INLINE bool ThreadSimpleManager::
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
//     Function: ThreadSimpleManager::get_current_time
//       Access: Public
//  Description: Returns elapsed time in seconds from some undefined
//               epoch, via whatever clock the manager is using for
//               all thread timing.
////////////////////////////////////////////////////////////////////
INLINE double ThreadSimpleManager::
get_current_time() const {
  return _clock->get_short_raw_time();
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleManager::get_global_ptr
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ThreadSimpleManager *ThreadSimpleManager::
get_global_ptr() {
  if (!_pointers_initialized) {
    init_pointers();
  }
  return _global_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleManager::CompareStartTime::operator()
//       Access: Public
//  Description: STL function object to sort the priority queue of
//               sleeping threads.
////////////////////////////////////////////////////////////////////
INLINE bool ThreadSimpleManager::CompareStartTime::
operator ()(ThreadSimpleImpl *a, ThreadSimpleImpl *b) const {
  return a->get_wake_time() > b->get_wake_time();
}
