// Filename: scissorAttrib.I
// Created by:  drose (29Jul08)
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
//     Function: ScissorAttrib::make
//       Access: Published, Static
//  Description: Constructs a ScissorAttrib that restricts rendering
//               to the indicated frame within the current
//               DisplayRegion.  (0,0) is the lower-left corner of the
//               DisplayRegion, and (1,1) is the upper-right corner.
////////////////////////////////////////////////////////////////////
INLINE CPT(RenderAttrib) ScissorAttrib::
make(PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top) {
  return make(LVecBase4(left, right, bottom, top));
}

////////////////////////////////////////////////////////////////////
//     Function: ScissorAttrib::is_off
//       Access: Published
//  Description: Returns true if the ScissorAttrib is an 'off'
//               ScissorAttrib, indicating that scissor testing is
//               disabled.
////////////////////////////////////////////////////////////////////
INLINE bool ScissorAttrib::
is_off() const {
  return _off;
}

////////////////////////////////////////////////////////////////////
//     Function: ScissorAttrib::get_frame
//       Access: Published
//  Description: Returns the left, right, bottom, top coordinates of
//               the scissor frame.  This defines a frame within the
//               current DisplayRegion, where 0,0 is the lower-left
//               corner of the DisplayRegion, and 1,1 is the
//               upper-right corner.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase4 &ScissorAttrib::
get_frame() const {
  return _frame;
}
