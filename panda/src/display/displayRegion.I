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
//     Function: DisplayRegion::get_dimensions
//       Access: Public
//  Description: Retrieves the coordinates of the DisplayRegion's
//               rectangle within its GraphicsLayer.  These numbers
//               will be in the range [0..1].
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegion::
get_dimensions(float &l, float &r, float &b, float &t) const {
  l = _l;
  r = _r;
  b = _b;
  t = _t;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_left
//       Access: Public
//  Description: Retrieves the x coordinate of the left edge of the
//               rectangle within its GraphicsLayer.  This number
//               will be in the range [0..1].
////////////////////////////////////////////////////////////////////
INLINE float DisplayRegion::
get_left() const {
  return _l;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_right
//       Access: Public
//  Description: Retrieves the x coordinate of the right edge of the
//               rectangle within its GraphicsLayer.  This number
//               will be in the range [0..1].
////////////////////////////////////////////////////////////////////
INLINE float DisplayRegion::
get_right() const {
  return _r;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_bottom
//       Access: Public
//  Description: Retrieves the y coordinate of the bottom edge of 
//               the rectangle within its GraphicsLayer.  This 
//               number will be in the range [0..1].
////////////////////////////////////////////////////////////////////
INLINE float DisplayRegion::
get_bottom() const {
  return _b;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_top
//       Access: Public
//  Description: Retrieves the y coordinate of the top edge of the
//               rectangle within its GraphicsLayer.  This number
//               will be in the range [0..1].
////////////////////////////////////////////////////////////////////
INLINE float DisplayRegion::
get_top() const {
  return _t;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_layer
//       Access: Public
//  Description: Returns the layer associated with this particular
//               DisplayRegion, or NULL if no layer is associated
//               (or if the layer was deleted).
////////////////////////////////////////////////////////////////////
INLINE GraphicsLayer *DisplayRegion::
get_layer() const {
  return _layer;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_camera
//       Access: Public
//  Description: Returns the camera associated with this
//               DisplayRegion, or NULL if no camera is associated.
////////////////////////////////////////////////////////////////////
INLINE const NodePath &DisplayRegion::
get_camera() const {
  return _camera;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::set_active
//       Access: Public
//  Description: Sets the active flag associated with the
//               DisplayRegion.  If the DisplayRegion is marked
//               inactive, nothing is rendered.
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegion::
set_active(bool active) {
  if (active != _active) {
    _active = active;
    win_display_regions_changed();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::is_active
//       Access: Public
//  Description: Returns the active flag associated with the
//               DisplayRegion.
////////////////////////////////////////////////////////////////////
INLINE bool DisplayRegion::
is_active() const {
  return _active;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::compute_pixels
//       Access: Public
//  Description: Computes the pixel locations of the DisplayRegion
//               within its layer, given the size of the layer in
//               pixels.
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegion::
compute_pixels(const int x, const int y) {
  _pl = int((_l * x) + 0.5);
  _pr = int((_r * x) + 0.5);
  _pb = int((_b * y) + 0.5);
  _pt = int((_t * y) + 0.5);
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_pixels
//       Access: Public
//  Description: Retrieves the coordinates of the DisplayRegion within
//               its layer, in pixels.
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegion::
get_pixels(int &pl, int &pr, int &pb, int &pt) const {
  pl = _pl;
  pr = _pr;
  pb = _pb;
  pt = _pt;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_region_pixels
//       Access: Public
//  Description: Retrieves the coordinates of the DisplayRegion within
//               its layer, as the pixel location of its bottom-left
//               corner, along with a pixel width and height.
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegion::
get_region_pixels(int &xo, int &yo, int &w, int &h) const {
  xo = _pl;
  yo = _pb;
  w = _pr - _pl;
  h = _pt - _pb;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_pixel_width
//       Access: Public
//  Description: Returns the width of the DisplayRegion in pixels.
////////////////////////////////////////////////////////////////////
INLINE int DisplayRegion::
get_pixel_width() const {
  return _pr - _pl;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_pixel_height
//       Access: Public
//  Description: Returns the height of the DisplayRegion in pixels.
////////////////////////////////////////////////////////////////////
INLINE int DisplayRegion::
get_pixel_height() const {
  return _pt - _pb;
}


INLINE ostream &operator << (ostream &out, const DisplayRegion &dr) {
  dr.output(out);
  return out;
}
