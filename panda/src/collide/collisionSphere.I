// Filename: collisionSphere.I
// Created by:  drose (24Apr00)
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
//     Function: CollisionSphere::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionSphere::
CollisionSphere(const LPoint3f &center, float radius) :
  _center(center), _radius(radius)
{
  nassertv(_radius >= 0.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionSphere::
CollisionSphere(float cx, float cy, float cz, float radius) :
  _center(cx, cy, cz), _radius(radius)
{
  nassertv(_radius >= 0.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::Default constructor
//       Access: Protected
//  Description: Creates an invalid sphere.  Only used when reading
//               from a bam file.
////////////////////////////////////////////////////////////////////
INLINE CollisionSphere::
CollisionSphere() {
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionSphere::
CollisionSphere(const CollisionSphere &copy) :
  CollisionSolid(copy),
  _center(copy._center),
  _radius(copy._radius)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::flush_level
//       Access: Public, Static
//  Description: Flushes the PStatCollectors used during traversal.
////////////////////////////////////////////////////////////////////
INLINE void CollisionSphere::
flush_level() {
  _volume_pcollector.flush_level();
  _test_pcollector.flush_level();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::set_center
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionSphere::
set_center(const LPoint3f &center) {
  _center = center;
  mark_internal_bounds_stale();
  mark_viz_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::set_center
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionSphere::
set_center(float x, float y, float z) {
  set_center(LPoint3f(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::get_center
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const LPoint3f &CollisionSphere::
get_center() const {
  return _center;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::set_radius
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionSphere::
set_radius(float radius) {
  nassertv(radius >= 0.0f);
  _radius = radius;
  mark_internal_bounds_stale();
  mark_viz_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::get_radius
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float CollisionSphere::
get_radius() const {
  return _radius;
}

