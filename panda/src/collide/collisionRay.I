/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionRay.I
 * @author drose
 * @date 2000-06-22
 */

/**
 * Creates an invalid ray.  This isn't terribly useful; it's expected that the
 * user will subsequently adjust the ray via set_origin()/set_direction() or
 * set_from_lens().
 */
INLINE CollisionRay::
CollisionRay() :
  _origin(LPoint3(0.0, 0.0, 0.0)),
  _direction(LVector3(0.0, 0.0, 0.0))
{
}

/**
 *
 */
INLINE CollisionRay::
CollisionRay(const LPoint3 &origin, const LVector3 &direction) :
  _origin(origin), _direction(direction)
{
  nassertv(_direction != LPoint3::zero());
}

/**
 *
 */
INLINE CollisionRay::
CollisionRay(PN_stdfloat ox, PN_stdfloat oy, PN_stdfloat oz,
             PN_stdfloat dx, PN_stdfloat dy, PN_stdfloat dz) :
  _origin(ox, oy, oz), _direction(dx, dy, dz)
{
  nassertv(_direction != LPoint3::zero());
}

/**
 *
 */
INLINE CollisionRay::
CollisionRay(const CollisionRay &copy) :
  CollisionSolid(copy),
  _origin(copy._origin),
  _direction(copy._direction)
{
}

/**
 *
 */
INLINE void CollisionRay::
set_origin(const LPoint3 &origin) {
  _origin = origin;
  mark_internal_bounds_stale();
  mark_viz_stale();
}

/**
 *
 */
INLINE void CollisionRay::
set_origin(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  set_origin(LPoint3(x, y, z));
}

/**
 *
 */
INLINE const LPoint3 &CollisionRay::
get_origin() const {
  return _origin;
}

/**
 *
 */
INLINE void CollisionRay::
set_direction(const LVector3 &direction) {
  _direction = direction;
  mark_internal_bounds_stale();
  mark_viz_stale();
  nassertv(_direction != LPoint3::zero());
}

/**
 *
 */
INLINE void CollisionRay::
set_direction(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  set_direction(LVector3(x, y, z));
}

/**
 *
 */
INLINE const LVector3 &CollisionRay::
get_direction() const {
  return _direction;
}

/**
 * Accepts a LensNode and a 2-d point in the range [-1,1].  Sets the
 * CollisionRay so that it begins at the LensNode's near plane and extends to
 * infinity, making it suitable for picking objects from the screen given a
 * camera and a mouse location.
 */
INLINE bool CollisionRay::
set_from_lens(LensNode *camera, PN_stdfloat px, PN_stdfloat py) {
  return set_from_lens(camera, LPoint2(px, py));
}
