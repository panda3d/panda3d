// Filename: threadNsprImpl.I
// Created by:  drose (08Aug02)
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
//     Function: ThreadNsprImpl::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ThreadNsprImpl::
ThreadNsprImpl(Thread *parent_obj) :
  _parent_obj(parent_obj)
{
  _thread = (PRThread *)NULL;
  _joinable = false;
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadNsprImpl::prepare_for_exit
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ThreadNsprImpl::
prepare_for_exit() {
  PR_Cleanup();
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadNsprImpl::Constructor
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE Thread *ThreadNsprImpl::
get_current_thread() {
  if (!_got_pt_ptr_index) {
    return (Thread *)NULL;
  }
  return (Thread *)PR_GetThreadPrivate(_pt_ptr_index);
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadNsprImpl::is_threading_supported
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool ThreadNsprImpl::
is_threading_supported() {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadNsprImpl::sleep
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ThreadNsprImpl::
sleep(double seconds) {
  PR_Sleep((int)(PR_TicksPerSecond() * seconds));
}
