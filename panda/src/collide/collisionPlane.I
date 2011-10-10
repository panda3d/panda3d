// Filename: collisionPlane.I
// Created by:  drose (25Apr00)
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
//     Function: CollisionPlane::Default Constructor
//       Access: Protected
//  Description: This is only for the convenience of CollisionPolygon.
//               Normally, you should not attempt to create an
//               uninitialized CollisionPlane.
////////////////////////////////////////////////////////////////////
INLINE CollisionPlane::
CollisionPlane() {
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPlane::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionPlane::
CollisionPlane(const LPlane &plane) :
  _plane(plane)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPlane::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionPlane::
CollisionPlane(const CollisionPlane &copy) :
  CollisionSolid(copy),
  _plane(copy._plane)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPlane::flush_level
//       Access: Public, Static
//  Description: Flushes the PStatCollectors used during traversal.
////////////////////////////////////////////////////////////////////
INLINE void CollisionPlane::
flush_level() {
  _volume_pcollector.flush_level();
  _test_pcollector.flush_level();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPlane::get_normal
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LVector3 CollisionPlane::
get_normal() const {
  return _plane.get_normal();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPlane::dist_to_plane
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat CollisionPlane::
dist_to_plane(const LPoint3 &point) const {
  return _plane.dist_to_plane(point);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPlane::set_plane
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionPlane::
set_plane(const LPlane &plane) {
  _plane = plane;
  mark_internal_bounds_stale();
  mark_viz_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPlane::get_plane
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const LPlane &CollisionPlane::
get_plane() const {
  return _plane;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPlane::flip
//       Access: Published
//  Description: Convenience method to flip the plane in-place.
////////////////////////////////////////////////////////////////////
INLINE void CollisionPlane::
flip() {
  _plane.flip();
}
