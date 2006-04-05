// Filename: atomicAdjustPosixImpl.I
// Created by:  drose (10Feb06)
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
//     Function: AtomicAdjustPosixImpl::inc
//       Access: Public, Static
//  Description: Atomically increments the indicated variable.
////////////////////////////////////////////////////////////////////
INLINE void AtomicAdjustPosixImpl::
inc(PN_int32 &var) {
  pthread_mutex_lock(&_mutex);
  ++var;
  pthread_mutex_unlock(&_mutex);
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustPosixImpl::dec
//       Access: Public, Static
//  Description: Atomically decrements the indicated variable and
//               returns true if the new value is nonzero, false if it
//               is zero.
////////////////////////////////////////////////////////////////////
INLINE bool AtomicAdjustPosixImpl::
dec(PN_int32 &var) {
  pthread_mutex_lock(&_mutex);
  PN_int32 result = --var;
  pthread_mutex_unlock(&_mutex);
  return (result != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustPosixImpl::set
//       Access: Public, Static
//  Description: Atomically changes the indicated variable and
//               returns the original value.
////////////////////////////////////////////////////////////////////
INLINE PN_int32 AtomicAdjustPosixImpl::
set(PN_int32 &var, PN_int32 new_value) {
  pthread_mutex_lock(&_mutex);
  PN_int32 orig_value = var;
  var = new_value;
  pthread_mutex_unlock(&_mutex);
  return orig_value;
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustPosixImpl::get
//       Access: Public, Static
//  Description: Atomically retrieves the snapshot value of the
//               indicated variable.  This is the only guaranteed safe
//               way to retrieve the value that other threads might be
//               asynchronously setting, incrementing, or decrementing
//               (via other AtomicAjust methods).
////////////////////////////////////////////////////////////////////
INLINE PN_int32 AtomicAdjustPosixImpl::
get(const PN_int32 &var) {
  pthread_mutex_lock(&_mutex);
  PN_int32 orig_value = var;
  pthread_mutex_unlock(&_mutex);
  return orig_value;
}
