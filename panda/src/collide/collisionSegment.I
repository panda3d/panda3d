/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionSegment.I
 * @author drose
 * @date 2001-01-30
 */

/**
 * Creates an invalid segment.  This isn't terribly useful; it's expected that
 * the user will subsequently adjust the segment via
 * set_origin()/set_direction() or set_from_lens().
 */
INLINE CollisionSegment::
CollisionSegment() :
  _a(LPoint3(0.0, 0.0, 0.0)),
  _b(LPoint3(0.0, 0.0, 0.0))
{
}

/**
 *
 */
INLINE CollisionSegment::
CollisionSegment(const LPoint3 &a, const LPoint3 &b) :
  _a(a), _b(b)
{
  nassertv(_a != _b);
}

/**
 *
 */
INLINE CollisionSegment::
CollisionSegment(PN_stdfloat ax, PN_stdfloat ay, PN_stdfloat az,
                 PN_stdfloat bx, PN_stdfloat by, PN_stdfloat bz) :
  _a(ax, ay, az), _b(bx, by, bz)
{
  nassertv(_a != _b);
}

/**
 *
 */
INLINE CollisionSegment::
CollisionSegment(const CollisionSegment &copy) :
  CollisionSolid(copy),
  _a(copy._a),
  _b(copy._b)
{
}

/**
 *
 */
INLINE void CollisionSegment::
set_point_a(const LPoint3 &a) {
  _a = a;
  mark_internal_bounds_stale();
  mark_viz_stale();
  // We don't assert here that a != b, on the assumption that you might be
  // about to change both at once, and you'll probably start by changing a
  // first.
}

/**
 *
 */
INLINE void CollisionSegment::
set_point_a(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  set_point_a(LPoint3(x, y, z));
}

/**
 *
 */
INLINE const LPoint3 &CollisionSegment::
get_point_a() const {
  return _a;
}

/**
 *
 */
INLINE void CollisionSegment::
set_point_b(const LPoint3 &b) {
  _b = b;
  mark_internal_bounds_stale();
  mark_viz_stale();
  nassertv(_a != _b);
}

/**
 *
 */
INLINE void CollisionSegment::
set_point_b(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  set_point_b(LPoint3(x, y, z));
}

/**
 *
 */
INLINE const LPoint3 &CollisionSegment::
get_point_b() const {
  return _b;
}

/**
 * Accepts a LensNode and a 2-d point in the range [-1,1].  Sets the
 * CollisionSegment so that it begins at the LensNode's near plane and extends
 * to the far plane, making it suitable for picking objects from the screen
 * given a camera and a mouse location.
 */
INLINE bool CollisionSegment::
set_from_lens(LensNode *camera, PN_stdfloat px, PN_stdfloat py) {
  return set_from_lens(camera, LPoint2(px, py));
}
