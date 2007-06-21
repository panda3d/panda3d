// Filename: threadSimpleImpl.I
// Created by:  drose (18Jun07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
//     Function: ThreadSimpleImpl::sleep
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ThreadSimpleImpl::
sleep(double seconds) {
  ThreadSimpleManager *manager = ThreadSimpleManager::get_global_ptr();
  ThreadSimpleImpl *thread = manager->get_current_thread();
  thread->sleep_this(seconds);
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleImpl::yield
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ThreadSimpleImpl::
yield() {
  ThreadSimpleManager *manager = ThreadSimpleManager::get_global_ptr();
  ThreadSimpleImpl *thread = manager->get_current_thread();
  thread->yield_this();
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleImpl::consider_yield
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ThreadSimpleImpl::
consider_yield() {
  ThreadSimpleManager *manager = ThreadSimpleManager::get_global_ptr();
  ThreadSimpleImpl *thread = manager->get_current_thread();
  thread->consider_yield_this();
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleImpl::consider_yield_this
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ThreadSimpleImpl::
consider_yield_this() {
  double now = _manager->get_current_time();
  if (now - _start_time > _time_per_epoch) {
    yield_this();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSimpleImpl::get_start_time
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE double ThreadSimpleImpl::
get_start_time() const {
  return _start_time;
}
