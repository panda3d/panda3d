// Filename: colorScaleAttrib.I
// Created by:  drose (14Mar02)
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
//     Function: ColorScaleAttrib::Constructor
//       Access: Protected
//  Description: Use ColorScaleAttrib::make() to construct a new
//               ColorScaleAttrib object.
////////////////////////////////////////////////////////////////////
INLINE ColorScaleAttrib::
ColorScaleAttrib(bool off, const LVecBase4f &scale) :
  _off(off),
  _scale(scale)
{
  _has_scale = !_scale.almost_equal(LVecBase4f(1.0f, 1.0f, 1.0f, 1.0f));
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::Copy Constructor
//       Access: Protected
//  Description: Use ColorScaleAttrib::make() to construct a new
//               ColorScaleAttrib object.
////////////////////////////////////////////////////////////////////
INLINE ColorScaleAttrib::
ColorScaleAttrib(const ColorScaleAttrib &copy) :
  _off(copy._off),
  _has_scale(copy._has_scale),
  _scale(copy._scale)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::is_off
//       Access: Published
//  Description: Returns true if the ColorScaleAttrib will ignore any
//               color scales inherited from above, false otherwise.
//               This is not the same thing as !has_scale(); a
//               ColorScaleAttrib may have the "off" flag set and also
//               have another scale specified.
////////////////////////////////////////////////////////////////////
INLINE bool ColorScaleAttrib::
is_off() const {
  return _off;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::has_scale
//       Access: Published
//  Description: Returns true if the ColorScaleAttrib has a
//               non-identity scale, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool ColorScaleAttrib::
has_scale() const {
  return _has_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::get_scale
//       Access: Published
//  Description: Returns the scale to be applied to colors.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase4f &ColorScaleAttrib::
get_scale() const {
  return _scale;
}
