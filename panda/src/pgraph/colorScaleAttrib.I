// Filename: colorScaleAttrib.I
// Created by:  drose (14Mar02)
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
//     Function: ColorScaleAttrib::Copy Constructor
//       Access: Protected
//  Description: Use ColorScaleAttrib::make() to construct a new
//               ColorScaleAttrib object.
////////////////////////////////////////////////////////////////////
INLINE ColorScaleAttrib::
ColorScaleAttrib(const ColorScaleAttrib &copy) :
  _off(copy._off),
  _has_scale(copy._has_scale),
  _has_rgb_scale(copy._has_rgb_scale),
  _has_alpha_scale(copy._has_alpha_scale),
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
//     Function: ColorScaleAttrib::is_identity
//       Access: Published
//  Description: Returns true if the ColorScaleAttrib is an identity
//               attrib, false if it is either an off attrib or it has
//               a scale.
////////////////////////////////////////////////////////////////////
INLINE bool ColorScaleAttrib::
is_identity() const {
  return !_off && !_has_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::has_scale
//       Access: Published
//  Description: Returns true if the ColorScaleAttrib has a
//               non-identity scale, false otherwise (in which case it
//               might be an off attrib or an identity attrib).
////////////////////////////////////////////////////////////////////
INLINE bool ColorScaleAttrib::
has_scale() const {
  return _has_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::has_rgb_scale
//       Access: Published
//  Description: Returns true if the ColorScaleAttrib has a
//               non-identity scale in the RGB components (ignoring
//               alpha), or false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool ColorScaleAttrib::
has_rgb_scale() const {
  return _has_rgb_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::has_alpha_scale
//       Access: Published
//  Description: Returns true if the ColorScaleAttrib has a
//               non-identity scale in the alpha component (ignoring
//               RGB), or false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool ColorScaleAttrib::
has_alpha_scale() const {
  return _has_alpha_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::get_scale
//       Access: Published
//  Description: Returns the scale to be applied to colors.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase4 &ColorScaleAttrib::
get_scale() const {
  return _scale;
}
