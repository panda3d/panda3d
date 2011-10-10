// Filename: billboardEffect.I
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
//     Function: BillboardEffect::Constructor
//       Access: Private
//  Description: Use BillboardEffect::make() to construct a new
//               BillboardEffect object.
////////////////////////////////////////////////////////////////////
INLINE BillboardEffect::
BillboardEffect() {
  _off = true;
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::make_axis
//       Access: Published, Static
//  Description: A convenience function to make a typical
//               axis-rotating billboard.
////////////////////////////////////////////////////////////////////
INLINE CPT(RenderEffect) BillboardEffect::
make_axis() {
  return make(LVector3::up(), false, true, 
              0.0f, NodePath(), LPoint3(0.0f, 0.0f, 0.0f));
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::make_point_eye
//       Access: Published, Static
//  Description: A convenience function to make a typical
//               eye-relative point-rotating billboard.
////////////////////////////////////////////////////////////////////
INLINE CPT(RenderEffect) BillboardEffect::
make_point_eye() {
  return make(LVector3::up(), true, false,
              0.0f, NodePath(), LPoint3(0.0f, 0.0f, 0.0f));
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::make_point_world
//       Access: Published, Static
//  Description: A convenience function to make a typical
//               world-relative point-rotating billboard.
////////////////////////////////////////////////////////////////////
INLINE CPT(RenderEffect) BillboardEffect::
make_point_world() {
  return make(LVector3::up(), false, false,
              0.0f, NodePath(), LPoint3(0.0f, 0.0f, 0.0f));
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::is_off
//       Access: Published
//  Description: Returns true if the BillboardEffect is an 'off'
//               BillboardEffect, indicating that it does not enable
//               billboarding.  This kind of BillboardEffect isn't
//               particularly useful and isn't normally created or
//               stored in the graph; it might be implicitly
//               discovered as the result of a
//               NodePath::get_rel_state().
////////////////////////////////////////////////////////////////////
INLINE bool BillboardEffect::
is_off() const {
  return _off;
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::get_up_vector
//       Access: Published
//  Description: Returns the up vector in effect for this billboard.
////////////////////////////////////////////////////////////////////
INLINE const LVector3 &BillboardEffect::
get_up_vector() const {
  return _up_vector;
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::get_eye_relative
//       Access: Published
//  Description: Returns true if this billboard interprets the up
//               vector relative to the camera, or false if it is
//               relative to the world.
////////////////////////////////////////////////////////////////////
INLINE bool BillboardEffect::
get_eye_relative() const {
  return _eye_relative;
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::get_axial_rotate
//       Access: Published
//  Description: Returns true if this billboard rotates only around
//               the axis of the up vector, or false if it rotates
//               freely in three dimensions.
////////////////////////////////////////////////////////////////////
INLINE bool BillboardEffect::
get_axial_rotate() const {
  return _axial_rotate;
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::get_offset
//       Access: Published
//  Description: Returns the distance toward the camera (or the
//               look_at_point) the billboard is moved towards, after
//               rotating.  This can be used to ensure the billboard
//               is not obscured by nearby geometry.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat BillboardEffect::
get_offset() const {
  return _offset;
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::get_look_at
//       Access: Published
//  Description: Returns the node this billboard will rotate to look
//               towards.  If this is empty, it means the billboard
//               will rotate towards the current camera node, wherever
//               that might be.
////////////////////////////////////////////////////////////////////
INLINE const NodePath &BillboardEffect::
get_look_at() const {
  return _look_at;
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::get_look_at_point
//       Access: Published
//  Description: Returns the point, relative to the look_at node,
//               towards which the billboard will rotate.  Normally
//               this is (0, 0, 0).
////////////////////////////////////////////////////////////////////
INLINE const LPoint3 &BillboardEffect::
get_look_at_point() const {
  return _look_at_point;
}
