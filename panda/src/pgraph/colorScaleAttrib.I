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
//       Access: Private
//  Description: Use ColorScaleAttrib::make() to construct a new
//               ColorScaleAttrib object.
////////////////////////////////////////////////////////////////////
INLINE ColorScaleAttrib::
ColorScaleAttrib(const LVecBase4f &scale) :
  _scale(scale)
{
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
