// Filename: displayRegion.I
// Created by:  frang (07Mar99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::is_active
//       Access: Published
//  Description: Returns the active flag associated with the
//               DisplayRegion.
////////////////////////////////////////////////////////////////////
INLINE bool DisplayRegion::
is_active() const {
  return _active;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::do_compute_pixels
//       Access: Private
//  Description: The private implementation of compute_pixels, this
//               assumes that we already have the lock.
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegion::
do_compute_pixels(int x_size, int y_size) {
  _pl = int((_l * x_size) + 0.5);
  _pr = int((_r * x_size) + 0.5);
  _pb = int((_b * y_size) + 0.5);
  _pt = int((_t * y_size) + 0.5);
  _pbi = int(((1.0f - _b) * y_size) + 0.5);
  _pti = int(((1.0f - _t) * y_size) + 0.5);
}


INLINE ostream &operator << (ostream &out, const DisplayRegion &dr) {
  dr.output(out);
  return out;
}
