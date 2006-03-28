// Filename: atomicAdjustNsprImpl.I
// Created by:  drose (09Aug02)
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
//     Function: AtomicAdjustNsprImpl::inc
//       Access: Public, Static
//  Description: Atomically increments the indicated variable and
//               returns the new value.
////////////////////////////////////////////////////////////////////
INLINE PN_int32 AtomicAdjustNsprImpl::
inc(PN_int32 &var) {
  return PR_AtomicIncrement(&var);
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustNsprImpl::dec
//       Access: Public, Static
//  Description: Atomically decrements the indicated variable and
//               returns the new value.
////////////////////////////////////////////////////////////////////
INLINE PN_int32 AtomicAdjustNsprImpl::
dec(PN_int32 &var) {
  return PR_AtomicDecrement(&var);
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustNsprImpl::set
//       Access: Public, Static
//  Description: Atomically changes the indicated variable and
//               returns the original value.
////////////////////////////////////////////////////////////////////
INLINE PN_int32 AtomicAdjustNsprImpl::
set(PN_int32 &var, PN_int32 new_value) {
  return PR_AtomicSet(&var, new_value);
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustNsprImpl::get
//       Access: Public, Static
//  Description: Atomically retrieves the snapshot value of the
//               indicated variable.  This is the only guaranteed safe
//               way to retrieve the value that other threads might be
//               asynchronously setting, incrementing, or decrementing
//               (via other AtomicAjust methods).
////////////////////////////////////////////////////////////////////
INLINE PN_int32 AtomicAdjustNsprImpl::
get(const PN_int32 &var) {
  return var;
}
