// Filename: billboardTransition.I
// Created by:  drose (24Mar00)
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
//     Function: BillboardTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BillboardTransition::
BillboardTransition() {
  _up_vector = LVector3f::up();
  _eye_relative = false;
  _axial_rotate = true;
  _offset = 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardTransition::axis
//       Access: Public, Static
//  Description: This named constructor returns a BillboardTransition
//               set up to rotate about whatever up axis is
//               appropriate to the given coordinate system.
////////////////////////////////////////////////////////////////////
INLINE BillboardTransition BillboardTransition::
axis(float offset, CoordinateSystem cs) {
  BillboardTransition t;
  t.set_up_vector(LVector3f::up(cs));
  t.set_eye_relative(false);
  t.set_axial_rotate(true);
  t.set_offset(offset);
  return t;
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardTransition::point_eye
//       Access: Public, Static
//  Description: This named constructor returns a BillboardTransition
//               set up to rotate about the indicated point in eye
//               (camera) coordinates.
////////////////////////////////////////////////////////////////////
INLINE BillboardTransition BillboardTransition::
point_eye(float offset, CoordinateSystem cs) {
  BillboardTransition t;
  t.set_up_vector(LVector3f::up(cs));
  t.set_eye_relative(true);
  t.set_axial_rotate(false);
  t.set_offset(offset);
  return t;
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardTransition::point_world
//       Access: Public, Static
//  Description: This named constructor returns a BillboardTransition
//               set up to rotate about the indicated point in world
//               coordinates.
////////////////////////////////////////////////////////////////////
INLINE BillboardTransition BillboardTransition::
point_world(float offset, CoordinateSystem cs) {
  BillboardTransition t;
  t.set_up_vector(LVector3f::up(cs));
  t.set_eye_relative(false);
  t.set_axial_rotate(false);
  t.set_offset(offset);
  return t;
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardTransition::set_up_vector
//       Access: Public
//  Description: Sets the vector that determines the "up" direction
//               for the billboard.  In axial_rotate mode, this is the
//               vector around which the billboard is constrained to
//               rotate.  When axial_rotate mode is off (i.e. in point
//               rotate mode), this vector describes the direction
//               that the billboard tries to keep facing upward as it
//               rotates around the point.
//
//               This vector may be either world-relative or
//               camera-relative, according to the eye_relative flag.
////////////////////////////////////////////////////////////////////
INLINE void BillboardTransition::
set_up_vector(const LVector3f &up_vector) {
  nassertv(!up_vector.almost_equal(LVector3f(0.0, 0.0, 0.0)));
  _up_vector = up_vector;
  state_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardTransition::get_up_vector
//       Access: Public
//  Description: See set_up_vector.
////////////////////////////////////////////////////////////////////
INLINE LVector3f BillboardTransition::
get_up_vector() const {
  return _up_vector;
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardTransition::set_eye_relative
//       Access: Public
//  Description: Sets the flag indicating whether the billboard's up
//               vector should be considered to be in camera space
//               (true) or world space (false).
////////////////////////////////////////////////////////////////////
INLINE void BillboardTransition::
set_eye_relative(bool eye_relative) {
  _eye_relative = eye_relative;
  state_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardTransition::get_eye_relative
//       Access: Public
//  Description: See set_eye_relative.
////////////////////////////////////////////////////////////////////
INLINE bool BillboardTransition::
get_eye_relative() const {
  return _eye_relative;
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardTransition::set_axial_rotate
//       Access: Public
//  Description: When this flag is true, the billboard will be
//               constrained to rotate in two dimensions about its up
//               vector only.  When false, the billboard will rotate
//               in all three dimensions as necessary to keep itself
//               facing the camera directly.
////////////////////////////////////////////////////////////////////
INLINE void BillboardTransition::
set_axial_rotate(bool axial_rotate) {
  _axial_rotate = axial_rotate;
  state_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardTransition::get_axial_rotate
//       Access: Public
//  Description: See set_axial_rotate.
////////////////////////////////////////////////////////////////////
INLINE bool BillboardTransition::
get_axial_rotate() const {
  return _axial_rotate;
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardTransition::set_offset
//       Access: Public
//  Description: Sets a linear offset in the billboard.  This is
//               particularly useful when eye_relative is true (which
//               means the billboard rotates towards the camera plane,
//               instead of towards the camera point).
//
//               The billboard geometry is first rotated
//               appropriately, and then translated along the linear
//               offset directly towards the camera.
//
//               This can be used to allow billboarding geometry to
//               appear to float in front of an object, while not
//               drifting away from the object as the object goes
//               off-axis.  Another way to achieve the same effect is
//               to simply translate the billboarding geometry by some
//               linear distance along the z axis before making it a
//               billboard, but this does not work in eye_relative
//               mode.
////////////////////////////////////////////////////////////////////
INLINE void BillboardTransition::
set_offset(float offset) {
  _offset = offset;
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardTransition::get_axial_rotate
//       Access: Public
//  Description: See set_offset().
////////////////////////////////////////////////////////////////////
INLINE float BillboardTransition::
get_offset() const {
  return _offset;
}
