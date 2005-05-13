// Filename: mouseSubregion.I
// Created by:  drose (13May05)
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
//     Function: MouseSubregion::get_left
//       Access: Published
//  Description: Retrieves the x coordinate of the left edge of the
//               rectangle within the window.  This number will be in
//               the range [0..1].
////////////////////////////////////////////////////////////////////
float MouseSubregion::
get_left() const {
  return _l;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseSubregion::get_right
//       Access: Published
//  Description: Retrieves the x coordinate of the right edge of the
//               rectangle within the window.  This number will be in
//               the range [0..1].
////////////////////////////////////////////////////////////////////
float MouseSubregion::
get_right() const {
  return _r;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseSubregion::get_bottom
//       Access: Published
//  Description: Retrieves the y coordinate of the bottom edge of 
//               the rectangle within the window.  This number will be
//               in the range [0..1].
////////////////////////////////////////////////////////////////////
float MouseSubregion::
get_bottom() const {
  return _b;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseSubregion::get_top
//       Access: Published
//  Description: Retrieves the y coordinate of the top edge of the
//               rectangle within the window.  This number will be in
//               the range [0..1].
////////////////////////////////////////////////////////////////////
float MouseSubregion::
get_top() const {
  return _t;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseSubregion::set_dimensions
//       Access: Published
//  Description: Changes the region of the window in which the mouse
//               is considered to be active.  The parameters are
//               identical to those for a DisplayRegion: they range
//               from 0 to 1, where 0,0 is the lower left corner and
//               1,1 is the upper right; (0, 1, 0, 1) represents the
//               whole window.
////////////////////////////////////////////////////////////////////
void MouseSubregion::
set_dimensions(float l, float r, float b, float t) {
  _l = l;
  _r = r;
  _b = b;
  _t = t;

  _minx = l * 2.0f - 1.0f;
  _miny = b * 2.0f - 1.0f;

  float maxx = r * 2.0f - 1.0f;
  float maxy = t * 2.0f - 1.0f;

  _scalex = 2.0f / (maxx - _minx);
  _scaley = 2.0f / (maxy - _miny);
}
