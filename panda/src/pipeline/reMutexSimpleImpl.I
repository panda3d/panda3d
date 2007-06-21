// Filename: reMutexSimpleImpl.I
// Created by:  drose (20Jun07)
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
//     Function: ReMutexSimpleImpl::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ReMutexSimpleImpl::
ReMutexSimpleImpl() {
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexSimpleImpl::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ReMutexSimpleImpl::
~ReMutexSimpleImpl() {
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexSimpleImpl::lock
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ReMutexSimpleImpl::
lock() {
  if (!try_lock()) {
    do_lock();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexSimpleImpl::try_lock
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool ReMutexSimpleImpl::
try_lock() {
  ThreadSimpleManager *manager = ThreadSimpleManager::get_global_ptr();
  ThreadSimpleImpl *thread = manager->get_current_thread();

  thread->consider_yield_this();
  if ((_flags & F_lock_count) != 0) {
    return false;
  }
  ++_flags;
  _locking_thread = thread;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ReMutexSimpleImpl::release
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ReMutexSimpleImpl::
release() {
  nassertv((_flags & F_lock_count) != 0);

  --_flags;
  if ((_flags & F_lock_count) == 0 && (_flags & F_has_waiters)) {
    do_release();
  }
}
