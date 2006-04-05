// Filename: threadWin32Impl.I
// Created by:  drose (07Feb06)
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
//     Function: ThreadWin32Impl::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ThreadWin32Impl::
ThreadWin32Impl(Thread *parent_obj) :
  _cv(_mutex),
  _parent_obj(parent_obj)
{
  _thread = 0;
  _joinable = false;
  _status = S_new;
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadWin32Impl::prepare_for_exit
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ThreadWin32Impl::
prepare_for_exit() {
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadWin32Impl::get_current_thread
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE Thread *ThreadWin32Impl::
get_current_thread() {
  if (!_got_pt_ptr_index) {
    init_pt_ptr_index();
  }
  return (Thread *)TlsGetValue(_pt_ptr_index);
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadWin32Impl::bind_thread
//       Access: Public, Static
//  Description: Associates the indicated Thread object with the
//               currently-executing thread.  You should not call this
//               directly; use Thread::bind_thread() instead.
////////////////////////////////////////////////////////////////////
INLINE void ThreadWin32Impl::
bind_thread(Thread *thread) {
  if (!_got_pt_ptr_index) {
    init_pt_ptr_index();
  }
  BOOL result = TlsSetValue(_pt_ptr_index, thread);
  nassertv(result);
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadWin32Impl::is_threading_supported
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool ThreadWin32Impl::
is_threading_supported() {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadWin32Impl::sleep
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ThreadWin32Impl::
sleep(double seconds) {
  Sleep((int)(seconds * 1000));
}
