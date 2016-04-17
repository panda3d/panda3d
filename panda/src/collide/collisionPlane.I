/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionPlane.I
 * @author drose
 * @date 2000-04-25
 */

/**
 * This is only for the convenience of CollisionPolygon.  Normally, you should
 * not attempt to create an uninitialized CollisionPlane.
 */
INLINE CollisionPlane::
CollisionPlane() {
}

/**
 *
 */
INLINE CollisionPlane::
CollisionPlane(const LPlane &plane) :
  _plane(plane)
{
}

/**
 *
 */
INLINE CollisionPlane::
CollisionPlane(const CollisionPlane &copy) :
  CollisionSolid(copy),
  _plane(copy._plane)
{
}

/**
 * Flushes the PStatCollectors used during traversal.
 */
INLINE void CollisionPlane::
flush_level() {
  _volume_pcollector.flush_level();
  _test_pcollector.flush_level();
}

/**
 *
 */
INLINE LVector3 CollisionPlane::
get_normal() const {
  return _plane.get_normal();
}

/**
 *
 */
INLINE PN_stdfloat CollisionPlane::
dist_to_plane(const LPoint3 &point) const {
  return _plane.dist_to_plane(point);
}

/**
 *
 */
INLINE void CollisionPlane::
set_plane(const LPlane &plane) {
  _plane = plane;
  mark_internal_bounds_stale();
  mark_viz_stale();
}

/**
 *
 */
INLINE const LPlane &CollisionPlane::
get_plane() const {
  return _plane;
}

/**
 * Convenience method to flip the plane in-place.
 */
INLINE void CollisionPlane::
flip() {
  _plane.flip();
}
