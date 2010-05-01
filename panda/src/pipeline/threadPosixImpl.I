// Filename: threadPosixImpl.I
// Created by:  drose (09Feb06)
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
//     Function: ThreadPosixImpl::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ThreadPosixImpl::
ThreadPosixImpl(Thread *parent_obj) :
  _parent_obj(parent_obj)
{
  _joinable = false;
  _detached = false;
  _status = S_new;
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadPosixImpl::preempt
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ThreadPosixImpl::
preempt() {
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadPosixImpl::prepare_for_exit
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ThreadPosixImpl::
prepare_for_exit() {
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadPosixImpl::get_current_thread
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE Thread *ThreadPosixImpl::
get_current_thread() {
  TAU_PROFILE("Thread *ThreadPosixImpl::get_current_thread()", " ", TAU_USER);
  if (!_got_pt_ptr_index) {
    init_pt_ptr_index();
  }
  return (Thread *)pthread_getspecific(_pt_ptr_index);
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadPosixImpl::bind_thread
//       Access: Public, Static
//  Description: Associates the indicated Thread object with the
//               currently-executing thread.  You should not call this
//               directly; use Thread::bind_thread() instead.
////////////////////////////////////////////////////////////////////
INLINE void ThreadPosixImpl::
bind_thread(Thread *thread) {
  if (!_got_pt_ptr_index) {
    init_pt_ptr_index();
  }
  int result = pthread_setspecific(_pt_ptr_index, thread);
  nassertv(result == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadPosixImpl::is_threading_supported
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool ThreadPosixImpl::
is_threading_supported() {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadPosixImpl::is_true_threads
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool ThreadPosixImpl::
is_true_threads() {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadPosixImpl::is_simple_threads
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool ThreadPosixImpl::
is_simple_threads() {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadPosixImpl::sleep
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ThreadPosixImpl::
sleep(double seconds) {
  TAU_PROFILE("void ThreadPosixImpl::sleep(double)", " ", TAU_USER);
  struct timespec rqtp;
  rqtp.tv_sec = time_t(seconds);
  rqtp.tv_nsec = long((seconds - (double)rqtp.tv_sec) * 1000000000.0);
  nanosleep(&rqtp, NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadPosixImpl::yield
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ThreadPosixImpl::
yield() {
  sleep(0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadPosixImpl::consider_yield
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ThreadPosixImpl::
consider_yield() {
}
