// Filename: alphaTransformAttribute.I
// Created by:  jason (01Aug00)
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
//     Function: AlphaTransformAttribute::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE AlphaTransformAttribute::
AlphaTransformAttribute()
{
}

////////////////////////////////////////////////////////////////////
//     Function: AlphaTransformAttribute::set_offset
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void AlphaTransformAttribute::
set_offset(float offset) {
  _state.set_offset(offset);
}

////////////////////////////////////////////////////////////////////
//     Function: AlphaTransformAttribute::get_offset
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float AlphaTransformAttribute::
get_offset() const {
  return _state.get_offset();
}

////////////////////////////////////////////////////////////////////
//     Function: AlphaTransformAttribute::set_scale
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void AlphaTransformAttribute::
set_scale(float scale) {
  _state.set_scale(scale);
}

////////////////////////////////////////////////////////////////////
//     Function: AlphaTransformAttribute::get_scale
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float AlphaTransformAttribute::
get_scale() const {
  return _state.get_scale();
}
