// Filename: asyncUtility.I
// Created by:  mike (09Jan97)
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
//     Function: AsyncUtility::set_frequency
//       Access: Public
//  Description: Fraction of a second.
////////////////////////////////////////////////////////////////////
INLINE void AsyncUtility::
set_frequency(float frequency) {
  _frequency = frequency;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncUtility::get_frequency
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float AsyncUtility::
get_frequency(void) const {
  return _frequency;
}
