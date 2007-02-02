// Filename: pnmBrush.I
// Created by:  drose (01Feb07)
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
//     Function: PNMBrush::Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PNMBrush::
PNMBrush(double xc, double yc) : _xc(xc), _yc(yc) {
}

////////////////////////////////////////////////////////////////////
//     Function: PNMBrush::get_xc
//       Access: Protected
//  Description: Returns the coordinates of the brush's center
//               pixel.  For a one-pixel brush, this will be (0.5,
//               0.5); for a centered two-pixel brush, this will be
//               (1.0, 1.0); for a centered three-pixel brush, this
//               will be (1.5, 1.5); and so on.
////////////////////////////////////////////////////////////////////
INLINE double PNMBrush::
get_xc() const {
  return _xc;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMBrush::get_xc
//       Access: Protected
//  Description: Returns the coordinates of the brush's center
//               pixel.  For a one-pixel brush, this will be (0.5,
//               0.5); for a centered two-pixel brush, this will be
//               (1.0, 1.0); for a centered three-pixel brush, this
//               will be (1.5, 1.5); and so on.
////////////////////////////////////////////////////////////////////
INLINE double PNMBrush::
get_yc() const {
  return _yc;
}
