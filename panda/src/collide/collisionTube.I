// Filename: collisionTube.I
// Created by:  drose (25Sep03)
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
//     Function: CollisionTube::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionTube::
CollisionTube(const LPoint3 &a, const LPoint3 &b, PN_stdfloat radius) :
  _a(a), _b(b), _radius(radius)
{
  recalc_internals();
  nassertv(_radius >= 0.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionTube::
CollisionTube(PN_stdfloat ax, PN_stdfloat ay, PN_stdfloat az,
              PN_stdfloat bx, PN_stdfloat by, PN_stdfloat bz,
              PN_stdfloat radius) :
  _a(ax, ay, az), _b(bx, by, bz), _radius(radius)
{
  recalc_internals();
  nassertv(_radius >= 0.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::Default constructor
//       Access: Private
//  Description: Creates an invalid tube.  Only used when reading
//               from a bam file.
////////////////////////////////////////////////////////////////////
INLINE CollisionTube::
CollisionTube() {
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionTube::
CollisionTube(const CollisionTube &copy) :
  CollisionSolid(copy),
  _a(copy._a),
  _b(copy._b),
  _radius(copy._radius)
{
  recalc_internals();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::flush_level
//       Access: Public, Static
//  Description: Flushes the PStatCollectors used during traversal.
////////////////////////////////////////////////////////////////////
INLINE void CollisionTube::
flush_level() {
  _volume_pcollector.flush_level();
  _test_pcollector.flush_level();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::set_point_a
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionTube::
set_point_a(const LPoint3 &a) {
  _a = a;
  recalc_internals();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::set_point_a
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionTube::
set_point_a(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  set_point_a(LPoint3(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::get_point_a
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const LPoint3 &CollisionTube::
get_point_a() const {
  return _a;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::set_point_b
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionTube::
set_point_b(const LPoint3 &b) {
  _b = b;
  recalc_internals();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::set_point_b
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionTube::
set_point_b(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  set_point_b(LPoint3(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::get_point_b
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const LPoint3 &CollisionTube::
get_point_b() const {
  return _b;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::set_radius
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionTube::
set_radius(PN_stdfloat radius) {
  nassertv(radius >= 0.0f);
  _radius = radius;

  // We don't need to call recalc_internals(), since the radius
  // doesn't change either of those properties.
  mark_internal_bounds_stale();
  mark_viz_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::get_radius
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat CollisionTube::
get_radius() const {
  return _radius;
}

