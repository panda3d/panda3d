// Filename: atomicAdjust.I
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
//     Function: AtomicAdjust::inc
//       Access: Public, Static
//  Description: Atomically increments the indicated variable and
//               returns the new value.
////////////////////////////////////////////////////////////////////
INLINE int AtomicAdjust::
inc(int &var) {
  return AtomicAdjustImpl::inc(var);
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjust::dec
//       Access: Public, Static
//  Description: Atomically decrements the indicated variable and
//               returns the new value.
////////////////////////////////////////////////////////////////////
INLINE int AtomicAdjust::
dec(int &var) {
  return AtomicAdjustImpl::dec(var);
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjust::set
//       Access: Public, Static
//  Description: Atomically changes the indicated variable and
//               returns the original value.
////////////////////////////////////////////////////////////////////
INLINE int AtomicAdjust::
set(int &var, int new_value) {
  return AtomicAdjustImpl::set(var, new_value);
}
