// Filename: atomicAdjustWin32Impl.I
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
//     Function: AtomicAdjustWin32Impl::inc
//       Access: Public, Static
//  Description: Atomically increments the indicated variable.
////////////////////////////////////////////////////////////////////
INLINE void AtomicAdjustWin32Impl::
inc(PN_int32 &var) {
  InterlockedIncrement((LONG *)&var);
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustWin32Impl::dec
//       Access: Public, Static
//  Description: Atomically decrements the indicated variable and
//               returns true if the new value is nonzero, false if it
//               is zero.
////////////////////////////////////////////////////////////////////
INLINE bool AtomicAdjustWin32Impl::
dec(PN_int32 &var) {
  return (InterlockedDecrement((LONG *)&var) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustWin32Impl::set
//       Access: Public, Static
//  Description: Atomically changes the indicated variable and
//               returns the original value.
////////////////////////////////////////////////////////////////////
INLINE PN_int32 AtomicAdjustWin32Impl::
set(PN_int32 &var, PN_int32 new_value) {
  return InterlockedExchange((LONG *)&var, new_value);
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustWin32Impl::get
//       Access: Public, Static
//  Description: Atomically retrieves the snapshot value of the
//               indicated variable.  This is the only guaranteed safe
//               way to retrieve the value that other threads might be
//               asynchronously setting, incrementing, or decrementing
//               (via other AtomicAjust methods).
////////////////////////////////////////////////////////////////////
INLINE PN_int32 AtomicAdjustWin32Impl::
get(const PN_int32 &var) {
  return var;
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustWin32Impl::set_ptr
//       Access: Public, Static
//  Description: Atomically changes the indicated variable and
//               returns the original value.
////////////////////////////////////////////////////////////////////
INLINE void *AtomicAdjustWin32Impl::
set_ptr(void *&var, void *new_value) {
  void *orig_value = var;
  var = new_value;
  return orig_value;
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustWin32Impl::get_ptr
//       Access: Public, Static
//  Description: Atomically retrieves the snapshot value of the
//               indicated variable.  This is the only guaranteed safe
//               way to retrieve the value that other threads might be
//               asynchronously setting, incrementing, or decrementing
//               (via other AtomicAjust methods).
////////////////////////////////////////////////////////////////////
INLINE void *AtomicAdjustWin32Impl::
get_ptr(void * const &var) {
  return var;
}
