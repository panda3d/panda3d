/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lquaternion_src.h
 * @author frang
 * @date 2000-06-06
 */

/**
 * This is the base quaternion class
 */
class EXPCL_PANDA_LINMATH FLOATNAME(LQuaternion) : public FLOATNAME(LVecBase4) {
PUBLISHED:
  INLINE_LINMATH FLOATNAME(LQuaternion)();
  INLINE_LINMATH FLOATNAME(LQuaternion)(const FLOATNAME(LVecBase4) &copy);
  INLINE_LINMATH FLOATNAME(LQuaternion)(FLOATTYPE r, const FLOATNAME(LVecBase3) &copy);
  INLINE_LINMATH FLOATNAME(LQuaternion)(FLOATTYPE r, FLOATTYPE i, FLOATTYPE j, FLOATTYPE k);

  static FLOATNAME(LQuaternion) pure_imaginary(const FLOATNAME(LVector3) &v);

  INLINE_LINMATH FLOATNAME(LQuaternion) conjugate() const;

  INLINE_LINMATH FLOATNAME(LVecBase3)
    xform(const FLOATNAME(LVecBase3) &v) const;

  INLINE_LINMATH FLOATNAME(LVecBase4)
    xform(const FLOATNAME(LVecBase4) &v) const;

  INLINE_LINMATH FLOATNAME(LQuaternion)
    multiply(const FLOATNAME(LQuaternion) &rhs) const;

  INLINE_LINMATH FLOATNAME(LQuaternion) operator - () const;

  INLINE_LINMATH FLOATNAME(LQuaternion)
  operator + (const FLOATNAME(LQuaternion) &other) const;
  INLINE_LINMATH FLOATNAME(LQuaternion)
  operator - (const FLOATNAME(LQuaternion) &other) const;

  INLINE_LINMATH FLOATTYPE angle_rad(const FLOATNAME(
      LQuaternion) &other) const;
  INLINE_LINMATH FLOATTYPE angle_deg(const FLOATNAME(
      LQuaternion) &other) const;

  INLINE_LINMATH FLOATNAME(LQuaternion) operator * (FLOATTYPE scalar) const;
  INLINE_LINMATH FLOATNAME(LQuaternion) operator / (FLOATTYPE scalar) const;

  INLINE_LINMATH FLOATNAME(LQuaternion) operator *(
      const FLOATNAME(LQuaternion) &) const;
  INLINE_LINMATH FLOATNAME(LQuaternion)& operator *=(
      const FLOATNAME(LQuaternion) &);

  INLINE_LINMATH FLOATNAME(LMatrix3) operator *(const FLOATNAME(LMatrix3) &);
  INLINE_LINMATH FLOATNAME(LMatrix4) operator *(const FLOATNAME(LMatrix4) &);

  FLOATNAME(LQuaternion) __pow__(FLOATTYPE) const;

  INLINE_LINMATH bool almost_equal(
      const FLOATNAME(LQuaternion) &other) const;
  INLINE_LINMATH bool almost_equal(
      const FLOATNAME(LQuaternion) &other, FLOATTYPE threshold) const;
  INLINE_LINMATH bool is_same_direction(
      const FLOATNAME(LQuaternion) &other) const;
  INLINE_LINMATH bool almost_same_direction(
      const FLOATNAME(LQuaternion) &other, FLOATTYPE threshold) const;

  INLINE_LINMATH void output(std::ostream&) const;

  void extract_to_matrix(FLOATNAME(LMatrix3) &m) const;
  void extract_to_matrix(FLOATNAME(LMatrix4) &m) const;

  void set_from_matrix(const FLOATNAME(LMatrix3) &m);
  INLINE_LINMATH void set_from_matrix(const FLOATNAME(LMatrix4) &m);
  void set_hpr(const FLOATNAME(LVecBase3) &hpr, CoordinateSystem cs = CS_default);
  FLOATNAME(LVecBase3) get_hpr(CoordinateSystem cs = CS_default) const;

  INLINE_LINMATH FLOATNAME(LVector3) get_axis() const;
  INLINE_LINMATH FLOATNAME(LVector3) get_axis_normalized() const;
  INLINE_LINMATH FLOATTYPE get_angle_rad() const;
  INLINE_LINMATH FLOATTYPE get_angle() const;

  INLINE_LINMATH void set_from_axis_angle_rad(
      FLOATTYPE angle_rad, const FLOATNAME(LVector3) &axis);
  INLINE_LINMATH void set_from_axis_angle(
      FLOATTYPE angle_deg, const FLOATNAME(LVector3) &axis);

  INLINE_LINMATH FLOATNAME(LVector3) get_up(CoordinateSystem cs = CS_default) const;
  INLINE_LINMATH FLOATNAME(LVector3) get_right(CoordinateSystem cs = CS_default) const;
  INLINE_LINMATH FLOATNAME(LVector3) get_forward(CoordinateSystem cs = CS_default) const;

  INLINE_LINMATH FLOATTYPE get_r() const;
  INLINE_LINMATH FLOATTYPE get_i() const;
  INLINE_LINMATH FLOATTYPE get_j() const;
  INLINE_LINMATH FLOATTYPE get_k() const;

  INLINE_LINMATH void set_r(FLOATTYPE r);
  INLINE_LINMATH void set_i(FLOATTYPE i);
  INLINE_LINMATH void set_j(FLOATTYPE j);
  INLINE_LINMATH void set_k(FLOATTYPE k);

  INLINE_LINMATH bool normalize();

  INLINE_LINMATH bool conjugate_from(const FLOATNAME(LQuaternion) &other);
  INLINE_LINMATH bool conjugate_in_place();

  INLINE_LINMATH bool invert_from(const FLOATNAME(LQuaternion) &other);
  INLINE_LINMATH bool invert_in_place();

  INLINE_LINMATH bool is_identity() const;
  INLINE_LINMATH bool is_almost_identity(FLOATTYPE tolerance) const;
  INLINE_LINMATH static const FLOATNAME(LQuaternion) &ident_quat();

private:
  static const FLOATNAME(LQuaternion) _ident_quat;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();
private:
  static TypeHandle _type_handle;
};


INLINE std::ostream& operator<<(std::ostream& os, const FLOATNAME(LQuaternion)& q) {
  q.output(os);
  return os;
}

BEGIN_PUBLISH
INLINE_LINMATH FLOATNAME(LQuaternion) invert(const FLOATNAME(LQuaternion) &a);
INLINE_LINMATH FLOATNAME(LMatrix3)
operator * (const FLOATNAME(LMatrix3) &m, const FLOATNAME(LQuaternion) &q);
INLINE_LINMATH FLOATNAME(LMatrix4)
operator * (const FLOATNAME(LMatrix4) &m, const FLOATNAME(LQuaternion) &q);
END_PUBLISH

#include "lquaternion_src.I"
