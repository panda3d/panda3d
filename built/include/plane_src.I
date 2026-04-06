/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file plane_src.I
 * @author mike
 * @date 1997-01-09
 */

/**
 * Creates a default plane.  This plane happens to intersect the origin,
 * perpendicular to the Z axis.  It's not clear how useful a default plane is.
 */
INLINE_MATHUTIL FLOATNAME(LPlane)::
FLOATNAME(LPlane)() {
  _v(0) = 0.0f;
  _v(1) = 0.0f;
  _v(2) = 1.0f;
  _v(3) = 0.0f;
}

/**
 *
 */
INLINE_MATHUTIL FLOATNAME(LPlane)::
FLOATNAME(LPlane)(const FLOATNAME(LVecBase4) &copy) :
  FLOATNAME(LVecBase4)(copy)
{
}

/**
 * Constructs a plane given three counter-clockwise points, as seen from the
 * front of the plane (that is, viewed from the end of the normal vector,
 * looking down).
 */
INLINE_MATHUTIL FLOATNAME(LPlane)::
FLOATNAME(LPlane)(const FLOATNAME(LPoint3) &a, const FLOATNAME(LPoint3) &b,
                 const FLOATNAME(LPoint3) &c) {
  FLOATNAME(LVector3) u = b - a;
  FLOATNAME(LVector3) v = c - a;
  FLOATNAME(LVector3) p = ::normalize(cross(u, v));

  _v(0) = p[0];
  _v(1) = p[1];
  _v(2) = p[2];
  _v(3) = -::dot(p, a);
}

/**
 * Constructs a plane given a surface normal vector and a point within the
 * plane.
 */
INLINE_MATHUTIL FLOATNAME(LPlane)::
FLOATNAME(LPlane)(const FLOATNAME(LVector3) &normal,
                 const FLOATNAME(LPoint3) &point) {
  FLOATNAME(LVector3) p = ::normalize(normal);

  _v(0) = p[0];
  _v(1) = p[1];
  _v(2) = p[2];
  _v(3) = -::dot(p, point);
}

/**
 * Constructs a plane given the four terms of the plane equation.
 */
INLINE_MATHUTIL FLOATNAME(LPlane)::
FLOATNAME(LPlane)(FLOATTYPE a, FLOATTYPE b, FLOATTYPE c, FLOATTYPE d) :
  FLOATNAME(LVecBase4)(a, b, c, d)
{
}

/**
 * Transforms the plane by the indicated matrix.
 */
INLINE_MATHUTIL FLOATNAME(LPlane) FLOATNAME(LPlane)::
operator * (const FLOATNAME(LMatrix3) &mat) const {
  FLOATNAME(LVector3) new_normal = mat.xform(get_normal());
  return FLOATNAME(LPlane)(new_normal, get_point());
}

/**
 * Transforms the plane by the indicated matrix.
 */
INLINE_MATHUTIL FLOATNAME(LPlane) FLOATNAME(LPlane)::
operator * (const FLOATNAME(LMatrix4) &mat) const {
  FLOATNAME(LVector3) new_normal = mat.xform_vec_general(get_normal());
  FLOATNAME(LPoint3) new_point = get_point() * mat;
  return FLOATNAME(LPlane)(new_normal, new_point);
}

/**
 * Transforms the plane by the indicated matrix.
 */
INLINE_MATHUTIL void FLOATNAME(LPlane)::
operator *= (const FLOATNAME(LMatrix4) &mat) {
  (*this) = (*this) * mat;
}

/**
 * Transforms the plane by the indicated matrix.
 */
INLINE_MATHUTIL void FLOATNAME(LPlane)::
xform(const FLOATNAME(LMatrix4) &mat) {
  (*this) = (*this) * mat;
}

/**
 * Returns the same plane facing the opposite direction.
 */
INLINE_MATHUTIL FLOATNAME(LPlane) FLOATNAME(LPlane)::
operator - () const {
  return FLOATNAME(LPlane)(-_v(0), -_v(1), -_v(2), -_v(3));
}

/**
 * Returns the surface normal of the plane.
 */
INLINE_MATHUTIL FLOATNAME(LVector3) FLOATNAME(LPlane)::
get_normal() const {
  return FLOATNAME(LVector3)(_v(0), _v(1), _v(2));
}

/**
 * Returns the straight-line shortest distance from the point to the plane.
 * The returned value is positive if the point is in front of the plane (on
 * the side with the normal), or negative in the point is behind the plane (on
 * the opposite side from the normal). It's zero if the point is exactly in
 * the plane.
 */
INLINE_MATHUTIL FLOATTYPE FLOATNAME(LPlane)::
dist_to_plane(const FLOATNAME(LPoint3) &point) const {
  return (_v(0) * point[0] + _v(1) * point[1] + _v(2) * point[2] + _v(3));
}

/**
 * Normalizes the plane in place.  Returns true if the plane was normalized,
 * false if the plane had a zero-length normal vector.
 */
INLINE_MATHUTIL bool FLOATNAME(LPlane)::
normalize() {
  FLOATTYPE l2 = get_normal().length_squared();
  if (l2 == (FLOATTYPE)0.0f) {
    return false;

  } else if (!IS_THRESHOLD_EQUAL(l2, 1.0f, NEARLY_ZERO(FLOATTYPE) * NEARLY_ZERO(FLOATTYPE))) {
    (*this) /= csqrt(l2);
  }

  return true;
}

/**
 * Normalizes the plane and returns the normalized plane as a copy.  If the
 * plane's normal was a zero-length vector, the same plane is returned.
 */
INLINE_MATHUTIL FLOATNAME(LPlane) FLOATNAME(LPlane)::
normalized() const {
  FLOATTYPE l2 = get_normal().length_squared();
  if (l2 != (FLOATTYPE)0.0f) {
    return (*this) / csqrt(l2);
  } else {
    return (*this);
  }
}

/**
 * Returns the point within the plane nearest to the indicated point in space.
 */
INLINE_MATHUTIL FLOATNAME(LPoint3) FLOATNAME(LPlane)::
project(const FLOATNAME(LPoint3) &point) const {
  return point - get_normal() * dist_to_plane(point);
}

/**
 * Convenience method that flips the plane in-place.  This is done by simply
 * flipping the normal vector.
 */
INLINE_MATHUTIL void FLOATNAME(LPlane)::
flip() {
  _v(0) = -_v(0);
  _v(1) = -_v(1);
  _v(2) = -_v(2);
  _v(3) = -_v(3);
}

/**
 * Returns true if the plane intersects the infinite line passing through
 * points p1 and p2, false if the line is parallel.  The points p1 and p2 are
 * used only to define the Euclidean line; they have no other bearing on the
 * intersection test.  If true, sets intersection_point to the point of
 * intersection.
 */
INLINE_MATHUTIL bool FLOATNAME(LPlane)::
intersects_line(FLOATNAME(LPoint3) &intersection_point,
                const FLOATNAME(LPoint3) &p1,
                const FLOATNAME(LPoint3) &p2) const {
  FLOATTYPE t;
  if (!intersects_line(t, p1, p2 - p1)) {
    return false;
  }
  intersection_point = p1 + t * (p2 - p1);
  return true;
}

/**
 * This flavor of intersects_line() returns a bit more information about the
 * nature of the intersecting point.  The line is defined via the parametric
 * equation from + t * delta for all real values of t.
 *
 * If there is no intersection with the plane, the function returns false and
 * leaves t undefined.  If there is an intersection with the plane, the
 * function returns true and sets t to the parametric value that defines the
 * point of intersection.  That is, t == 0.0f implies that the intersection
 * occurred exactly at point from, and t == 1.0f implies at point from +
 * delta, with other values of t accordingly.
 */
INLINE_MATHUTIL bool FLOATNAME(LPlane)::
intersects_line(FLOATTYPE &t,
                const FLOATNAME(LPoint3) &from,
                const FLOATNAME(LVector3) &delta) const {
  FLOATTYPE denom = ::dot(get_normal(), delta);
  if (IS_NEARLY_ZERO(denom)) {
    t = 0.0f;
    return false;
  }

  t = -(dist_to_plane(from) / denom);
  return true;
}

INLINE_MATHUTIL std::ostream &
operator << (std::ostream &out, const FLOATNAME(LPlane) &p) {
  p.output(out);
  return out;
}
