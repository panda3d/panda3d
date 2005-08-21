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
//     Function: TrueClock::get_error_count
//       Access: Public
//  Description: Returns the number of clock errors that have
//               been detected.  Each time a clock error is detected,
//               in which the value returned by either of the above
//               methods is suspect, the value returned by this method
//               will be incremented.  Applications can monitor this
//               value and react, for instance, by resynchronizing
//               their clocks each time this value changes.
////////////////////////////////////////////////////////////////////
INLINE int TrueClock::
get_error_count() const {
  return _error_count;
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
