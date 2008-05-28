// Filename: collisionDSSolid.I
// Created by:  Dave Schuyler (05Apr06)
// Based on collision tube by:  drose
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
//     Function: Collision::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionDSSolid::
CollisionDSSolid(
    const LPoint3f &center_a, float radius_a,
    const LPoint3f &center_b, float radius_b,
    const Planef &plane_a, const Planef &plane_b) :
  _center_a(center_a), _radius_a(radius_a),
  _center_b(center_b), _radius_b(radius_b),
  _plane_a(plane_a), _plane_b(plane_b),
  _lens_radius(0.0f)
{
  recalc_internals();
  nassertv(_radius_a >= 0.0f);
  nassertv(_radius_b >= 0.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionDSSolid::
CollisionDSSolid(
    float ax, float ay, float az,
    float radius_a,
    float bx, float by, float bz,
    float radius_b,
    const Planef &plane_a, const Planef &plane_b) :
  _center_a(ax, ay, az), _radius_a(radius_a),
  _center_b(bx, by, bz), _radius_b(radius_b),
  _plane_a(plane_a), _plane_b(plane_b),
  _lens_radius(0.0f)
{
  recalc_internals();
  nassertv(_radius_a >= 0.0f);
  nassertv(_radius_b >= 0.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::Default constructor
//       Access: Private
//  Description: Creates an invalid tube.  Only used when reading
//               from a bam file.
////////////////////////////////////////////////////////////////////
INLINE CollisionDSSolid::
CollisionDSSolid() {
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionDSSolid::
CollisionDSSolid(const CollisionDSSolid &copy) :
  CollisionSolid(copy),
  _center_a(copy._center_a),
  _radius_a(copy._radius_a),
  _center_b(copy._center_b),
  _radius_b(copy._radius_b),
  _plane_a(copy._plane_a),
  _plane_b(copy._plane_b)
{
  recalc_internals();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::set_center_a
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionDSSolid::
set_center_a(const LPoint3f &a) {
  _center_a = a;
  recalc_internals();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::set_center_a
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionDSSolid::
set_center_a(float x, float y, float z) {
  set_center_a(LPoint3f(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::get_center_a
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const LPoint3f &CollisionDSSolid::
get_center_a() const {
  return _center_a;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::set_center_b
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionDSSolid::
set_center_b(const LPoint3f &b) {
  _center_b = b;
  recalc_internals();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::set_center_b
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionDSSolid::
set_center_b(float x, float y, float z) {
  set_center_b(LPoint3f(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::get_center_b
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const LPoint3f &CollisionDSSolid::
get_center_b() const {
  return _center_b;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::set_radius_a
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionDSSolid::
set_radius_a(float radius) {
  nassertv(radius >= 0.0f);
  _radius_a = radius;

  // We don't need to call recalc_internals(), since the radius
  // doesn't change either of those properties.
  mark_internal_bounds_stale();
  mark_viz_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::get_radius_a
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float CollisionDSSolid::
get_radius_a() const {
  return _radius_a;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::set_radius_b
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionDSSolid::
set_radius_b(float radius) {
  nassertv(radius >= 0.0f);
  _radius_b = radius;

  // We don't need to call recalc_internals(), since the radius
  // doesn't change either of those properties.
  mark_internal_bounds_stale();
  mark_viz_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::get_radius_b
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float CollisionDSSolid::
get_radius_b() const {
  return _radius_b;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::get_normal_a
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LVector3f CollisionDSSolid::
get_normal_a() const {
  return _plane_a.get_normal();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::dist_to_plane_a
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float CollisionDSSolid::
dist_to_plane_a(const LPoint3f &point) const {
  return _plane_a.dist_to_plane(point);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::set_plane_a
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionDSSolid::
set_plane_a(const Planef &plane) {
  _plane_a = plane;
  mark_internal_bounds_stale();
  mark_viz_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::get_plane_a
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const Planef &CollisionDSSolid::
get_plane_a() const {
  return _plane_a;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::get_normal_b
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LVector3f CollisionDSSolid::
get_normal_b() const {
  return _plane_b.get_normal();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::dist_to_plane_b
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float CollisionDSSolid::
dist_to_plane_b(const LPoint3f &point) const {
  return _plane_b.dist_to_plane(point);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::set_plane_b
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionDSSolid::
set_plane_b(const Planef &plane) {
  _plane_b = plane;
  mark_internal_bounds_stale();
  mark_viz_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::get_plane_b
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const Planef &CollisionDSSolid::
get_plane_b() const {
  return _plane_b;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::get_lens_radius
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float CollisionDSSolid::
get_lens_radius() const {
  return _lens_radius;
}

