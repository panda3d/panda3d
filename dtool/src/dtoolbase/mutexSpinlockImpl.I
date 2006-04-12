// Filename: mutexSpinlockImpl.I
// Created by:  drose (11Apr06)
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
//     Function: MutexSpinlockImpl::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MutexSpinlockImpl::
MutexSpinlockImpl() {
  _lock = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: MutexSpinlockImpl::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MutexSpinlockImpl::
~MutexSpinlockImpl() {
}

////////////////////////////////////////////////////////////////////
//     Function: MutexSpinlockImpl::lock
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void MutexSpinlockImpl::
lock() {
  if (!try_lock()) {
    do_lock();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MutexSpinlockImpl::try_lock
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool MutexSpinlockImpl::
try_lock() {
  return (AtomicAdjust::compare_and_exchange(_lock, 0, 1) == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: MutexSpinlockImpl::release
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void MutexSpinlockImpl::
release() {
  AtomicAdjust::set(_lock, 0);
}
