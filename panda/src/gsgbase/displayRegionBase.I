// Filename: displayRegionBase.I
// Created by:  drose (20Feb09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: DisplayRegionBase::Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE DisplayRegionBase::
DisplayRegionBase() {
}

INLINE ostream &
operator << (ostream &out, const DisplayRegionBase &dr) {
  dr.output(out);
  return out;
}
