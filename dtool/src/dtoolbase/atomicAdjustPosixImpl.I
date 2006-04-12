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
inc(TVOLATILE PN_int32 &var) {
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
dec(TVOLATILE PN_int32 &var) {
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
set(TVOLATILE PN_int32 &var, PN_int32 new_value) {
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
get(const TVOLATILE PN_int32 &var) {
  pthread_mutex_lock(&_mutex);
  PN_int32 orig_value = var;
  pthread_mutex_unlock(&_mutex);
  return orig_value;
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustPosixImpl::set_ptr
//       Access: Public, Static
//  Description: Atomically changes the indicated variable and
//               returns the original value.
////////////////////////////////////////////////////////////////////
INLINE void *AtomicAdjustPosixImpl::
set_ptr(void * TVOLATILE &var, void *new_value) {
  pthread_mutex_lock(&_mutex);
  void *orig_value = var;
  var = new_value;
  pthread_mutex_unlock(&_mutex);
  return orig_value;
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustPosixImpl::get_ptr
//       Access: Public, Static
//  Description: Atomically retrieves the snapshot value of the
//               indicated variable.  This is the only guaranteed safe
//               way to retrieve the value that other threads might be
//               asynchronously setting, incrementing, or decrementing
//               (via other AtomicAjust methods).
////////////////////////////////////////////////////////////////////
INLINE void *AtomicAdjustPosixImpl::
get_ptr(void * const TVOLATILE &var) {
  pthread_mutex_lock(&_mutex);
  void *orig_value = var;
  pthread_mutex_unlock(&_mutex);
  return orig_value;
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustPosixImpl::compare_and_exchange
//       Access: Public, Static
//  Description: Atomic compare and exchange.  
//
//               If mem is equal to old_value, store new_value in mem.
//               In either case, return the original value of mem.
//               The caller can test for success by comparing
//               return_value == old_value.
//
//               The atomic function expressed in pseudo-code:
//
//                 orig_value = mem;
//                 if (mem == old_value) {
//                   mem = new_value;
//                 }
//                 return orig_value;
//
////////////////////////////////////////////////////////////////////
INLINE PN_int32 AtomicAdjustPosixImpl::
compare_and_exchange(TVOLATILE PN_int32 &mem, PN_int32 old_value,
                     PN_int32 new_value) {
  pthread_mutex_lock(&_mutex);
  PN_int32 orig_value = mem;
  if (mem == old_value) {
    mem = new_value;
  }
  pthread_mutex_unlock(&_mutex);
  return orig_value;
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustPosixImpl::compare_and_exchange_ptr
//       Access: Public, Static
//  Description: Atomic compare and exchange.  
//
//               As above, but works on pointers instead of integers.
////////////////////////////////////////////////////////////////////
INLINE void *AtomicAdjustPosixImpl::
compare_and_exchange_ptr(void * TVOLATILE &mem, void *old_value,
                         void *new_value) {
  pthread_mutex_lock(&_mutex);
  void *orig_value = mem;
  if (mem == old_value) {
    mem = new_value;
  }
  pthread_mutex_unlock(&_mutex);
  return orig_value;
}
