// Filename: trueClock.I
// Created by:  drose (04Jul00)
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
//     Function: TrueClock::get_ptr
//       Access: Public
//  Description: Returns a pointer to the one TrueClock object in
//               the world.
////////////////////////////////////////////////////////////////////
INLINE TrueClock *TrueClock::
get_ptr() {
  if (_global_ptr == (TrueClock *)NULL) {
    _global_ptr = new TrueClock;
  }
  return _global_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: TrueClock::Destructor
//       Access: Protected
//  Description: A protected destructor because no one should try to
//               delete the global TrueClock.
////////////////////////////////////////////////////////////////////
INLINE TrueClock::
~TrueClock() {
}
