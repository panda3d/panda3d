/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionBox.I
 * @author amith tudur
 * @date 2009-07-31
 */

/**
 * Create the Box by giving a Center and distances of each of the sides of
 * box from the Center.
 */
INLINE CollisionBox::
CollisionBox(const LPoint3 &center, PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) :
  _center(center)
{
  _min = LPoint3(_center.get_x() - x, _center.get_y() - y, _center.get_z() - z);
  _max = LPoint3(_center.get_x() + x, _center.get_y() + y, _center.get_z() + z);
  for (int p = 0; p < 6; ++p) {
    _planes[p] = set_plane(p);
  }
}

/**
 * Create the Box by Specifying the Diagonal Points
 */
INLINE CollisionBox::
CollisionBox(const LPoint3 &min, const LPoint3 &max) :
  _center((min + max) / 2), _min(min), _max(max)
{
  for (int p = 0; p < 6; ++p) {
    _planes[p] = set_plane(p);
  }
}

/**
 * Creates an invalid Box.  Only used when reading from a bam file.
 */
INLINE CollisionBox::
CollisionBox() {
}

/**
 *
 */
INLINE CollisionBox::
CollisionBox(const CollisionBox &copy) :
  CollisionSolid(copy),
  _center(copy._center),
  _min(copy._min),
  _max(copy._max)
{
  for (int p = 0; p < 6; ++p) {
    _planes[p] = copy._planes[p];
  }
}

/**
 * Flushes the PStatCollectors used during traversal.
 */
INLINE void CollisionBox::
flush_level() {
  _volume_pcollector.flush_level();
  _test_pcollector.flush_level();
}

/**
 *
 */
INLINE void CollisionBox::
set_center(const LPoint3 &center) {
  _center = center;
  mark_internal_bounds_stale();
  mark_viz_stale();
}

/**
 *
 */
INLINE void CollisionBox::
set_center(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  set_center(LPoint3(x, y, z));
}

/**
 *
 */
INLINE const LPoint3 &CollisionBox::
get_center() const {
  return _center;
}

/**
 *
 */
INLINE const LPoint3 &CollisionBox::
get_min() const {
  return _min;
}

/**
 *
 */
INLINE const LPoint3 &CollisionBox::
get_max() const {
  return _max;
}

/**
 *
 */
INLINE LVector3 CollisionBox::
get_dimensions() const {
  return _max - _min;
}

/**
 * Returns 8: the number of vertices of a rectangular solid.
 */
INLINE int CollisionBox::
get_num_points() const {
  return 8;
}

/**
 * Returns the nth vertex of the OBB.
 */
INLINE LPoint3 CollisionBox::
get_point(int n) const {
  return get_point_aabb(n);
}

/**
 * Returns the nth vertex of the Axis Aligned Bounding Box.
 */
INLINE LPoint3 CollisionBox::
get_point_aabb(int n) const {
  nassertr(n >= 0 && n < 8, LPoint3::zero());

  // We do some trickery assuming that _min and _max are consecutive in
  // memory.
  const LPoint3 *a = &_min;
  return LPoint3(a[(n>>2)&1][0], a[(n>>1)&1][1], a[(n)&1][2]);
}

/**
 * Returns 6: the number of faces of a rectangular solid.
 */
INLINE int CollisionBox::
get_num_planes() const {
  return 6;
}

/**
 * Returns the nth face of the rectangular solid.
 */
INLINE LPlane CollisionBox::
get_plane(int n) const {
  nassertr(n >= 0 && n < 6, LPlane());
  return _planes[n];
}

/**
 * Creates the nth face of the rectangular solid.
 *
 * @deprecated Same as get_plane().
 */
INLINE LPlane CollisionBox::
set_plane(int n) const {
  nassertr(n >= 0 && n < 6, LPlane());
  return LPlane(get_point(plane_def[n][0]),
                get_point(plane_def[n][1]),
                get_point(plane_def[n][2]));
}
