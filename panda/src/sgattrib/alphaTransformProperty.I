// Filename: alphaTransformProperty.I
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
//     Function: AlphaTransformProperty::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE AlphaTransformProperty::
AlphaTransformProperty() :
  _offset(0), _scale(1)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AlphaTransformProperty::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE AlphaTransformProperty::
AlphaTransformProperty(float offset, float scale) :
  _offset(offset), _scale(scale)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AlphaTransformProperty::set_offset
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void AlphaTransformProperty::
set_offset(float offset) {
  _offset = offset;
}

////////////////////////////////////////////////////////////////////
//     Function: AlphaTransformProperty::get_offset
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float AlphaTransformProperty::
get_offset() const {
  return _offset;
}

////////////////////////////////////////////////////////////////////
//     Function: AlphaTransformProperty::set_scale
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void AlphaTransformProperty::
set_scale(float scale) {
  _scale = scale;
}

////////////////////////////////////////////////////////////////////
//     Function: AlphaTransformProperty::get_scale
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float AlphaTransformProperty::
get_scale() const {
  return _scale;
}
