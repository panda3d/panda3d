// Filename: collisionSphere.I
// Created by:  drose (24Apr00)
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
//     Function: CollisionSphere::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionSphere::
CollisionSphere(const LPoint3f &center, float radius) :
  _center(center), _radius(radius)
{
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
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::Default constructor
//       Access: Private
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
//     Function: CollisionSphere::set_center
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionSphere::
set_center(const LPoint3f &center) {
  _center = center;
  mark_bound_stale();
  mark_viz_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::set_center
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionSphere::
set_center(float x, float y, float z) {
  set_center(LPoint3f(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::get_center
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const LPoint3f &CollisionSphere::
get_center() const {
  return _center;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::set_radius
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionSphere::
set_radius(float radius) {
  _radius = radius;
  mark_bound_stale();
  mark_viz_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::get_radius
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float CollisionSphere::
get_radius() const {
  return _radius;
}

