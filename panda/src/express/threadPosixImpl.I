// Filename: threadPosixImpl.I
// Created by:  drose (09Feb06)
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
//     Function: ThreadPosixImpl::sleep
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ThreadPosixImpl::
sleep(double seconds) {
  struct timespec rqtp;
  rqtp.tv_sec = time_t(seconds);
  rqtp.tv_nsec = long((seconds - (double)rqtp.tv_sec) * 1000000000.0);
  nanosleep(&rqtp, NULL);
}
