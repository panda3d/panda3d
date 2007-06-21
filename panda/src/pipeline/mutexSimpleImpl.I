// Filename: mutexSimpleImpl.I
// Created by:  drose (19Jun07)
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
//     Function: MutexSimpleImpl::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MutexSimpleImpl::
MutexSimpleImpl() {
}

////////////////////////////////////////////////////////////////////
//     Function: MutexSimpleImpl::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MutexSimpleImpl::
~MutexSimpleImpl() {
}

////////////////////////////////////////////////////////////////////
//     Function: MutexSimpleImpl::lock
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void MutexSimpleImpl::
lock() {
  if (!try_lock()) {
    do_lock();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MutexSimpleImpl::try_lock
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool MutexSimpleImpl::
try_lock() {
  ThreadSimpleImpl::consider_yield();
  if ((_flags & F_lock_count) != 0) {
    return false;
  }
  _flags |= F_lock_count;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MutexSimpleImpl::release
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void MutexSimpleImpl::
release() {
  nassertv((_flags & F_lock_count) != 0);
  _flags &= ~F_lock_count;

  if (_flags & F_has_waiters) {
    do_release();
  }
}
