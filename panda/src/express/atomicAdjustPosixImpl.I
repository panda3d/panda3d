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
//  Description: Atomically increments the indicated variable and
//               returns the new value.
////////////////////////////////////////////////////////////////////
INLINE PN_int32 AtomicAdjustPosixImpl::
inc(PN_int32 &var) {
  pthread_mutex_lock(&_mutex);
  PN_int32 result = ++var;
  pthread_mutex_unlock(&_mutex);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustPosixImpl::dec
//       Access: Public, Static
//  Description: Atomically decrements the indicated variable and
//               returns the new value.
////////////////////////////////////////////////////////////////////
INLINE PN_int32 AtomicAdjustPosixImpl::
dec(PN_int32 &var) {
  pthread_mutex_lock(&_mutex);
  PN_int32 result = --var;
  pthread_mutex_unlock(&_mutex);
  return result;
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
