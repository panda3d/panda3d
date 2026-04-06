/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionSphere.I
 * @author drose
 * @date 2000-04-24
 */

/**
 *
 */
INLINE CollisionSphere::
CollisionSphere(const LPoint3 &center, PN_stdfloat radius) :
  _center(center), _radius(radius)
{
  nassertv(_radius >= 0.0f);
}

/**
 *
 */
INLINE CollisionSphere::
CollisionSphere(PN_stdfloat cx, PN_stdfloat cy, PN_stdfloat cz, PN_stdfloat radius) :
  _center(cx, cy, cz), _radius(radius)
{
  nassertv(_radius >= 0.0f);
}

/**
 * Creates an invalid sphere.  Only used when reading from a bam file.
 */
INLINE CollisionSphere::
CollisionSphere() {
}

/**
 *
 */
INLINE CollisionSphere::
CollisionSphere(const CollisionSphere &copy) :
  CollisionSolid(copy),
  _center(copy._center),
  _radius(copy._radius)
{
}

/**
 * Flushes the PStatCollectors used during traversal.
 */
INLINE void CollisionSphere::
flush_level() {
  _volume_pcollector.flush_level();
  _test_pcollector.flush_level();
}

/**
 *
 */
INLINE void CollisionSphere::
set_center(const LPoint3 &center) {
  _center = center;
  mark_internal_bounds_stale();
  mark_viz_stale();
}

/**
 *
 */
INLINE void CollisionSphere::
set_center(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  set_center(LPoint3(x, y, z));
}

/**
 *
 */
INLINE const LPoint3 &CollisionSphere::
get_center() const {
  return _center;
}

/**
 *
 */
INLINE void CollisionSphere::
set_radius(PN_stdfloat radius) {
  nassertv(radius >= 0.0f);
  _radius = radius;
  mark_internal_bounds_stale();
  mark_viz_stale();
}

/**
 *
 */
INLINE PN_stdfloat CollisionSphere::
get_radius() const {
  return _radius;
}
