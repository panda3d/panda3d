// Filename: mutexSimpleImpl.I
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
//     Function: MutexSimpleImpl::acquire
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void MutexSimpleImpl::
acquire() {
  if (!try_acquire()) {
    do_acquire();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MutexSimpleImpl::try_acquire
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool MutexSimpleImpl::
try_acquire() {
  if ((_flags & F_lock_count) != 0) {
    return false;
  }
  _flags |= F_lock_count;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MutexSimpleImpl::release
//       Access: Public
//  Description: Releases the mutex.  An immediate context switch
//               might occur if there were waiters on the mutex.
////////////////////////////////////////////////////////////////////
INLINE void MutexSimpleImpl::
release() {
  nassertv((_flags & F_lock_count) != 0);
  _flags &= ~F_lock_count;

  if (_flags & F_has_waiters) {
    do_release();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MutexSimpleImpl::release_quietly
//       Access: Public
//  Description: Releases the mutex, without allowing a context switch
//               to occur.
////////////////////////////////////////////////////////////////////
INLINE void MutexSimpleImpl::
release_quietly() {
  nassertv((_flags & F_lock_count) != 0);
  _flags &= ~F_lock_count;

  if (_flags & F_has_waiters) {
    do_release_quietly();
  }
}
