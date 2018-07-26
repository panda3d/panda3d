/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file plane_src.h
 * @author mike
 * @date 1997-01-09
 */

/**
 * An abstract mathematical description of a plane.  A plane is defined by the
 * equation Ax + By + Cz + D = 0.
 */
class EXPCL_PANDA_MATHUTIL FLOATNAME(LPlane) : public FLOATNAME(LVecBase4) {
PUBLISHED:
  INLINE_MATHUTIL FLOATNAME(LPlane)();
  INLINE_MATHUTIL FLOATNAME(LPlane)(const FLOATNAME(LVecBase4) &copy);
  INLINE_MATHUTIL FLOATNAME(LPlane)(const FLOATNAME(LPoint3) &a,
                                   const FLOATNAME(LPoint3) &b,
                                   const FLOATNAME(LPoint3) &c);
  INLINE_MATHUTIL FLOATNAME(LPlane)(const FLOATNAME(LVector3) &normal,
                                   const FLOATNAME(LPoint3) &point);
  INLINE_MATHUTIL FLOATNAME(LPlane)(FLOATTYPE a, FLOATTYPE b,
                                   FLOATTYPE c, FLOATTYPE d);

  INLINE_MATHUTIL FLOATNAME(LPlane) operator * (const FLOATNAME(LMatrix3) &mat) const;
  INLINE_MATHUTIL FLOATNAME(LPlane) operator * (const FLOATNAME(LMatrix4) &mat) const;
  INLINE_MATHUTIL void operator *= (const FLOATNAME(LMatrix4) &mat);
  INLINE_MATHUTIL void xform(const FLOATNAME(LMatrix4) &mat);
  INLINE_MATHUTIL FLOATNAME(LPlane) operator - () const;

  FLOATNAME(LMatrix4) get_reflection_mat() const;

  INLINE_MATHUTIL FLOATNAME(LVector3) get_normal() const;
  FLOATNAME(LPoint3) get_point() const;

  INLINE_MATHUTIL FLOATTYPE dist_to_plane(const FLOATNAME(LPoint3) &point) const;

  INLINE_MATHUTIL bool normalize();
  INLINE_MATHUTIL FLOATNAME(LPlane) normalized() const;
  INLINE_MATHUTIL FLOATNAME(LPoint3) project(const FLOATNAME(LPoint3) &point) const;
  INLINE_MATHUTIL void flip();

  INLINE_MATHUTIL bool intersects_line(FLOATNAME(LPoint3) &intersection_point,
                                       const FLOATNAME(LPoint3) &p1,
                                       const FLOATNAME(LPoint3) &p2) const;
  INLINE_MATHUTIL bool intersects_line(FLOATTYPE &t,
                                       const FLOATNAME(LPoint3) &from,
                                       const FLOATNAME(LVector3) &delta) const;

  bool intersects_plane(FLOATNAME(LPoint3) &from,
                        FLOATNAME(LVector3) &delta,
                        const FLOATNAME(LPlane) &other) const;

  bool intersects_parabola(FLOATTYPE &t1, FLOATTYPE &t2,
                           const FLOATNAME(LParabola) &parabola) const;

  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level = 0) const;
};

INLINE_MATHUTIL std::ostream &
operator << (std::ostream &out, const FLOATNAME(LPlane) &p);

#include "plane_src.I"
