/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionPolygon.I
 * @author drose
 * @date 2000-04-25
 */

/**
 *
 */
INLINE CollisionPolygon::
CollisionPolygon(const LVecBase3 &a, const LVecBase3 &b,
                 const LVecBase3 &c) {
  LPoint3 array[3];
  array[0] = a;
  array[1] = b;
  array[2] = c;
  setup_points(array, array + 3);
}

/**
 *
 */
INLINE CollisionPolygon::
CollisionPolygon(const LVecBase3 &a, const LVecBase3 &b,
                 const LVecBase3 &c, const LVecBase3 &d) {
  LPoint3 array[4];
  array[0] = a;
  array[1] = b;
  array[2] = c;
  array[3] = d;
  setup_points(array, array + 4);
}


/**
 *
 */
INLINE CollisionPolygon::
CollisionPolygon(const LPoint3 *begin, const LPoint3 *end) {
  setup_points(begin, end);
}

/**
 * Creates an invalid polygon.  Only used when reading from a bam file.
 */
INLINE CollisionPolygon::
CollisionPolygon() {
}

/**
 * Returns the number of vertices of the CollisionPolygon.
 */
INLINE size_t CollisionPolygon::
get_num_points() const {
  return _points.size();
}

/**
 * Returns the nth vertex of the CollisionPolygon, expressed in 3-D space.
 */
INLINE LPoint3 CollisionPolygon::
get_point(size_t n) const {
  nassertr(n < _points.size(), LPoint3::zero());
  LMatrix4 to_3d_mat;
  rederive_to_3d_mat(to_3d_mat);
  return to_3d(_points[n]._p, to_3d_mat);
}

/**
 * Verifies that the indicated set of points will define a valid
 * CollisionPolygon: that is, at least three non-collinear points, with no
 * points repeated.
 */
INLINE bool CollisionPolygon::
verify_points(const LPoint3 &a, const LPoint3 &b,
              const LPoint3 &c) {
  LPoint3 array[3];
  array[0] = a;
  array[1] = b;
  array[2] = c;
  return verify_points(array, array + 3);
}

/**
 * Verifies that the indicated set of points will define a valid
 * CollisionPolygon: that is, at least three non-collinear points, with no
 * points repeated.
 */
INLINE bool CollisionPolygon::
verify_points(const LPoint3 &a, const LPoint3 &b,
              const LPoint3 &c, const LPoint3 &d) {
  LPoint3 array[4];
  array[0] = a;
  array[1] = b;
  array[2] = c;
  array[3] = d;
  return verify_points(array, array + 4);
}

/**
 * Flushes the PStatCollectors used during traversal.
 */
INLINE void CollisionPolygon::
flush_level() {
  _volume_pcollector.flush_level();
  _test_pcollector.flush_level();
}

/**
 * Returns true if the 2-d v1 is to the right of v2.
 */
INLINE bool CollisionPolygon::
is_right(const LVector2 &v1, const LVector2 &v2) {
  return (v1[0] * v2[1] - v1[1] * v2[0]) > 1.0e-6f;
}

/**
 * Returns the linear distance of p to the line defined by f and f+v, where v
 * is a normalized vector.  The result is negative if p is left of the line,
 * positive if it is right of the line.
 */
INLINE PN_stdfloat CollisionPolygon::
dist_to_line(const LPoint2 &p,
             const LPoint2 &f, const LVector2 &v) {
  LVector2 v1 = (p - f);
  return (v1[0] * v[1] - v1[1] * v[0]);
}

/**
 * Assuming the indicated point in 3-d space lies within the polygon's plane,
 * returns the corresponding point in the polygon's 2-d definition space.
 */
INLINE LPoint2 CollisionPolygon::
to_2d(const LVecBase3 &point3d) const {
  LPoint3 point = LPoint3(point3d) * _to_2d_mat;
  return LPoint2(point[0], point[2]);
}

/**
 * Fills the indicated matrix with the appropriate rotation transform to move
 * points from the 2-d plane into the 3-d (X, 0, Z) plane.
 */
INLINE void CollisionPolygon::
calc_to_3d_mat(LMatrix4 &to_3d_mat) const {
  // We have to be explicit about the coordinate system--we specifically mean
  // CS_zup_right, because that points the forward vector down the Y axis and
  // moves the coords in (X, 0, Z).  We want this effect regardless of the
  // user's coordinate system of choice.

  // The up vector, on the other hand, is completely arbitrary.

  look_at(to_3d_mat, -get_plane().get_normal(),
          LVector3(0.0f, 0.0f, 1.0f), CS_zup_right);
  to_3d_mat.set_row(3, get_plane().get_point());
}

/**
 * Fills the indicated matrix with the appropriate rotation transform to move
 * points from the 2-d plane into the 3-d (X, 0, Z) plane.
 *
 * This is essentially similar to calc_to_3d_mat, except that the matrix is
 * rederived from whatever is stored in _to_2d_mat, guaranteeing that it will
 * match whatever algorithm produced that one, even if it was produced on a
 * different machine with different numerical precision.
 */
INLINE void CollisionPolygon::
rederive_to_3d_mat(LMatrix4 &to_3d_mat) const {
  to_3d_mat.invert_from(_to_2d_mat);
}

/**
 * Extrude the indicated point in the polygon's 2-d definition space back into
 * 3-d coordinates.
 */
INLINE LPoint3 CollisionPolygon::
to_3d(const LVecBase2 &point2d, const LMatrix4 &to_3d_mat) {
  return LPoint3(point2d[0], 0.0f, point2d[1]) * to_3d_mat;
}

/**
 *
 */
INLINE CollisionPolygon::PointDef::
PointDef(const LPoint2 &p, const LVector2 &v) : _p(p), _v(v) {
}

/**
 *
 */
INLINE CollisionPolygon::PointDef::
PointDef(PN_stdfloat x, PN_stdfloat y) : _p(x, y), _v(0.0f, 0.0f) {
}

/**
 *
 */
INLINE CollisionPolygon::PointDef::
PointDef(const CollisionPolygon::PointDef &copy) : _p(copy._p), _v(copy._v) {
}

/**
 *
 */
INLINE void CollisionPolygon::PointDef::
operator = (const CollisionPolygon::PointDef &copy) {
  _p = copy._p;
  _v = copy._v;
}
