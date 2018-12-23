/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionCapsule.I
 * @author drose
 * @date 2003-09-25
 */

/**
 *
 */
INLINE CollisionCapsule::
CollisionCapsule(const LPoint3 &a, const LPoint3 &b, PN_stdfloat radius) :
  _a(a), _b(b), _radius(radius)
{
  recalc_internals();
  nassertv(_radius >= 0.0f);
}

/**
 *
 */
INLINE CollisionCapsule::
CollisionCapsule(PN_stdfloat ax, PN_stdfloat ay, PN_stdfloat az,
              PN_stdfloat bx, PN_stdfloat by, PN_stdfloat bz,
              PN_stdfloat radius) :
  _a(ax, ay, az), _b(bx, by, bz), _radius(radius)
{
  recalc_internals();
  nassertv(_radius >= 0.0f);
}

/**
 * Creates an invalid capsule.  Only used when reading from a bam file.
 */
INLINE CollisionCapsule::
CollisionCapsule() {
}

/**
 *
 */
INLINE CollisionCapsule::
CollisionCapsule(const CollisionCapsule &copy) :
  CollisionSolid(copy),
  _a(copy._a),
  _b(copy._b),
  _radius(copy._radius)
{
  recalc_internals();
}

/**
 * Flushes the PStatCollectors used during traversal.
 */
INLINE void CollisionCapsule::
flush_level() {
  _volume_pcollector.flush_level();
  _test_pcollector.flush_level();
}

/**
 *
 */
INLINE void CollisionCapsule::
set_point_a(const LPoint3 &a) {
  _a = a;
  recalc_internals();
}

/**
 *
 */
INLINE void CollisionCapsule::
set_point_a(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  set_point_a(LPoint3(x, y, z));
}

/**
 *
 */
INLINE const LPoint3 &CollisionCapsule::
get_point_a() const {
  return _a;
}

/**
 *
 */
INLINE void CollisionCapsule::
set_point_b(const LPoint3 &b) {
  _b = b;
  recalc_internals();
}

/**
 *
 */
INLINE void CollisionCapsule::
set_point_b(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  set_point_b(LPoint3(x, y, z));
}

/**
 *
 */
INLINE const LPoint3 &CollisionCapsule::
get_point_b() const {
  return _b;
}

/**
 *
 */
INLINE void CollisionCapsule::
set_radius(PN_stdfloat radius) {
  nassertv(radius >= 0.0f);
  _radius = radius;

  // We don't need to call recalc_internals(), since the radius doesn't change
  // either of those properties.
  mark_internal_bounds_stale();
  mark_viz_stale();
}

/**
 *
 */
INLINE PN_stdfloat CollisionCapsule::
get_radius() const {
  return _radius;
}
