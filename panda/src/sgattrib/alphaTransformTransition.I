// Filename: alphaTransformTransition.I
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
//     Function: AlphaTransformTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE AlphaTransformTransition::
AlphaTransformTransition(float offset, float scale) :
  _state(offset, scale)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AlphaTransformTransition::set_offset
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void AlphaTransformTransition::
set_offset(float offset) {
  _state.set_offset(offset);
}

////////////////////////////////////////////////////////////////////
//     Function: AlphaTransformTransition::get_offset
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float AlphaTransformTransition::
get_offset() const {
  return _state.get_offset();
}

////////////////////////////////////////////////////////////////////
//     Function: AlphaTransformTransition::set_scale
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void AlphaTransformTransition::
set_scale(float scale) {
  _state.set_scale(scale);
}

////////////////////////////////////////////////////////////////////
//     Function: AlphaTransformTransition::get_scale
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float AlphaTransformTransition::
get_scale() const {
  return _state.get_scale();
}
