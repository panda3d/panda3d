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
//  Description: Atomically increments the indicated variable and
//               returns the new value.
////////////////////////////////////////////////////////////////////
INLINE int AtomicAdjustWin32Impl::
inc(int &var) {
  nassertr(sizeof(int) == sizeof(LONG), ++var);
  return InterlockedIncrement((LONG *)&var);
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustWin32Impl::dec
//       Access: Public, Static
//  Description: Atomically decrements the indicated variable and
//               returns the new value.
////////////////////////////////////////////////////////////////////
INLINE int AtomicAdjustWin32Impl::
dec(int &var) {
  nassertr(sizeof(int) == sizeof(LONG), --var);
  return InterlockedDecrement((LONG *)&var);
}

////////////////////////////////////////////////////////////////////
//     Function: AtomicAdjustWin32Impl::set
//       Access: Public, Static
//  Description: Atomically changes the indicated variable and
//               returns the original value.
////////////////////////////////////////////////////////////////////
INLINE int AtomicAdjustWin32Impl::
set(int &var, int new_value) {
#ifndef NDEBUG
  nassertd(sizeof(int) == sizeof(LONG)) {
    int temp = var;
    var = new_value;
    return temp;
  }
#endif
  return InterlockedExchange((LONG *)&var, new_value);
}
