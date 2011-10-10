// Filename: scissorEffect.I
// Created by:  drose (30Jul08)
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
//     Function: ScissorEffect::is_screen
//       Access: Published
//  Description: Returns true if the ScissorEffect is a screen-based
//               effect, meaning get_frame() has a meaningful value,
//               but get_a() and get_b() do not.
////////////////////////////////////////////////////////////////////
INLINE bool ScissorEffect::
is_screen() const {
  return _screen;
}

////////////////////////////////////////////////////////////////////
//     Function: ScissorEffect::get_frame
//       Access: Published
//  Description: If is_screen() returns true, this method may be
//               called to query the screen-based scissor frame.  This
//               is a series of left, right, bottom, top, representing
//               the scissor frame relative to the current
//               DisplayRegion.  See ScissorAttrib.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase4 &ScissorEffect::
get_frame() const {
  return _frame;
}

////////////////////////////////////////////////////////////////////
//     Function: ScissorEffect::get_num_points
//       Access: Published
//  Description: Returns the number of node-based scissor points.  See
//               get_point().
////////////////////////////////////////////////////////////////////
INLINE int ScissorEffect::
get_num_points() const {
  return (int)_points.size();
}

////////////////////////////////////////////////////////////////////
//     Function: ScissorEffect::get_point
//       Access: Published
//  Description: If is_screen() returns false, then get_num_points() and
//               get_point() may be called to query the node-based scissor
//               frame.  These return n points (at least two), which
//               are understood to be in the space of this node, and
//               which define any opposite corners of the scissor
//               frame.
////////////////////////////////////////////////////////////////////
INLINE const LPoint3 &ScissorEffect::
get_point(int n) const {
  nassertr(n >= 0 && n < (int)_points.size(), LPoint3::zero());
  return _points[n]._p;
}

////////////////////////////////////////////////////////////////////
//     Function: ScissorEffect::get_node
//       Access: Published
//  Description: Returns the node to which the nth point is relative,
//               or empty NodePath to indicate the current node.
////////////////////////////////////////////////////////////////////
INLINE NodePath ScissorEffect::
get_node(int n) const {
  nassertr(n >= 0 && n < (int)_points.size(), NodePath());
  return _points[n]._node;
}

////////////////////////////////////////////////////////////////////
//     Function: ScissorEffect::get_clip
//       Access: Published
//  Description: Returns true if this ScissorEffect actually enables
//               scissoring, or false if it culls only.
////////////////////////////////////////////////////////////////////
INLINE bool ScissorEffect::
get_clip() const {
  return _clip;
}
