// Filename: lquaternion_src.I
// Created by:  frang (06Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::Default Constructor
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LQuaternion)::
FLOATNAME(LQuaternion)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::Copy Constructor
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LQuaternion)::
FLOATNAME(LQuaternion)(const FLOATNAME(LVecBase4) &copy) :
  FLOATNAME(LVecBase4)(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::Copy Constructor
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LQuaternion)::
FLOATNAME(LQuaternion)(FLOATTYPE r, const FLOATNAME(LVecBase3) &copy) {
  set(r, copy[0], copy[1], copy[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::Constructor
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LQuaternion)::
FLOATNAME(LQuaternion)(FLOATTYPE r, FLOATTYPE i, FLOATTYPE j, FLOATTYPE k) {
  set(r, i, j, k);
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::xform
//       Access: Published
//  Description: Transforms a 3-d vector by the indicated rotation
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LQuaternion)::
xform(const FLOATNAME(LVecBase3) &v) const {
  FLOATNAME(LQuaternion) v_quat(0.0f, v[0], v[1], v[2]);

  FLOATNAME(LQuaternion) conjugate(
    _v(0), -_v(1), -_v(2), -_v(3));
  v_quat = conjugate * v_quat * (*this);

  return FLOATNAME(LVecBase3)(v_quat[1], v_quat[2], v_quat[3]);
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::xform
//       Access: Published
//  Description: Transforms a 4-d vector by the indicated rotation
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4) FLOATNAME(LQuaternion)::
xform(const FLOATNAME(LVecBase4) &v) const {
  FLOATNAME(LQuaternion) v_quat(v[0], v[1], v[2], v[3]);

  FLOATNAME(LQuaternion) conjugate(
    _v(0), -_v(1), -_v(2), -_v(3));
  v_quat = conjugate * v_quat * (*this);

  return FLOATNAME(LVecBase4)(v_quat);
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::multiply
//       Access: Published
//  Description: actual multiply call (non virtual)
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LQuaternion) FLOATNAME(LQuaternion)::
multiply(const FLOATNAME(LQuaternion) &rhs) const {
  FLOATTYPE r = (rhs._v(0) * _v(0)) - (rhs._v(1) * _v(1)) - (rhs._v(2) * _v(2)) - (rhs._v(3) * _v(3));
  FLOATTYPE i = (rhs._v(1) * _v(0)) + (rhs._v(0) * _v(1)) - (rhs._v(3) * _v(2)) + (rhs._v(2) * _v(3));
  FLOATTYPE j = (rhs._v(2) * _v(0)) + (rhs._v(3) * _v(1)) + (rhs._v(0) * _v(2)) - (rhs._v(1) * _v(3));
  FLOATTYPE k = (rhs._v(3) * _v(0)) - (rhs._v(2) * _v(1)) + (rhs._v(1) * _v(2)) + (rhs._v(0) * _v(3));

  return FLOATNAME(LQuaternion)(r, i, j, k);
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::unary -
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LQuaternion) FLOATNAME(LQuaternion)::
operator - () const {
  return FLOATNAME(LVecBase4)::operator - ();
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::quaternion + quaternion
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LQuaternion) FLOATNAME(LQuaternion)::
operator + (const FLOATNAME(LQuaternion) &other) const {
  return FLOATNAME(LVecBase4)::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::quaternion - quaternion
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LQuaternion) FLOATNAME(LQuaternion)::
operator - (const FLOATNAME(LQuaternion) &other) const {
  return FLOATNAME(LVecBase4)::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::angle_rad
//       Access: Published
//  Description: Returns the angle between the orientation represented
//               by this quaternion and the other one, expressed in
//               radians.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LQuaternion)::
angle_rad(const FLOATNAME(LQuaternion) &other) const {
  return get_forward().angle_rad(other.get_forward());
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::angle_deg
//       Access: Published
//  Description: Returns the angle between the orientation represented
//               by this quaternion and the other one, expressed in
//               degrees.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LQuaternion)::
angle_deg(const FLOATNAME(LQuaternion) &other) const {
  return rad_2_deg(angle_rad(other));
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::quaternion * scalar
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LQuaternion) FLOATNAME(LQuaternion)::
operator * (FLOATTYPE scalar) const {
  return FLOATNAME(LVecBase4)::operator * (scalar);
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::quaternion / scalar
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LQuaternion) FLOATNAME(LQuaternion)::
operator / (FLOATTYPE scalar) const {
  return FLOATNAME(LVecBase4)::operator / (scalar);
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::Multiply Operator
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LQuaternion) FLOATNAME(LQuaternion)::
operator *(const FLOATNAME(LQuaternion)& c) const {
  return multiply(c);
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::Multiply Assignment Operator
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LQuaternion)& FLOATNAME(LQuaternion)::
operator *=(const FLOATNAME(LQuaternion)& c) {
  (*this) = operator*(c);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::Multiply Operator
//       Access: public
//  Description: Quat * Matrix = matrix
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LQuaternion)::
operator *(const FLOATNAME(LMatrix3) &m) {
  FLOATNAME(LMatrix3) result;
  extract_to_matrix(result);
  return result * m;
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::Multiply Operator
//       Access: public
//  Description: Quat * Matrix = matrix
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4) FLOATNAME(LQuaternion)::
operator *(const FLOATNAME(LMatrix4) &m) {
  FLOATNAME(LMatrix3) m_upper_3 = m.get_upper_3();
  FLOATNAME(LMatrix3) this_quat;
  extract_to_matrix(this_quat);

  FLOATNAME(LMatrix4) result;
  result.set_upper_3(this_quat * m_upper_3);
  result.set_row(3, m.get_row(3));
  result.set_col(3, m.get_col(3));

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::almost_equal
//       Access: public
//  Description: Returns true if two quaternions are memberwise equal
//               within a default tolerance based on the numeric type.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LQuaternion)::
almost_equal(const FLOATNAME(LQuaternion) &other) const {
  return almost_equal(other, NEARLY_ZERO(FLOATTYPE));
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::almost_equal
//       Access: public
//  Description: Returns true if two quaternions are memberwise equal
//               within a specified tolerance.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LQuaternion)::
almost_equal(const FLOATNAME(LQuaternion) &other,
             FLOATTYPE threshold) const {
  return (IS_THRESHOLD_EQUAL(_v(0), other._v(0), threshold) &&
          IS_THRESHOLD_EQUAL(_v(1), other._v(1), threshold) &&
          IS_THRESHOLD_EQUAL(_v(2), other._v(2), threshold) &&
          IS_THRESHOLD_EQUAL(_v(3), other._v(3), threshold));
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::is_same_direction
//       Access: public
//  Description: Returns true if two quaternions represent the same
//               rotation within a default tolerance based on the
//               numeric type.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LQuaternion)::
is_same_direction(const FLOATNAME(LQuaternion) &other) const {
  return almost_same_direction(other, NEARLY_ZERO(FLOATTYPE));
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::almost_same_direction
//       Access: public
//  Description: Returns true if two quaternions represent the same
//               rotation within a specified tolerance.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LQuaternion)::
almost_same_direction(const FLOATNAME(LQuaternion) &other, 
                      FLOATTYPE threshold) const {
  return ((*this) * invert(other)).is_almost_identity(threshold);
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::output
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LQuaternion)::
output(ostream& os) const {
  os << MAYBE_ZERO(_v(0)) << " + "
     << MAYBE_ZERO(_v(1)) << "i + "
     << MAYBE_ZERO(_v(2)) << "j + "
     << MAYBE_ZERO(_v(3)) << "k";
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::set_from_matrix
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LQuaternion)::
set_from_matrix(const FLOATNAME(LMatrix4) &m) {
  set_from_matrix(m.get_upper_3());
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::get_axis
//       Access: Public
//  Description: This, along with get_angle(), returns the rotation
//               represented by the quaternion as an angle about an
//               arbitrary axis.  This returns the axis; it is not
//               normalized.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LQuaternion)::
get_axis() const {
  return FLOATNAME(LVector3)(_v(1), _v(2), _v(3));
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::get_axis_normalized
//       Access: Public
//  Description: This, along with get_angle(), returns the rotation
//               represented by the quaternion as an angle about an
//               arbitrary axis.  This returns the normalized axis.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LQuaternion)::
get_axis_normalized() const {
  FLOATNAME(LVector3) axis = get_axis();
  axis.normalize();
  return axis;
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::get_angle_rad
//       Access: Public
//  Description: This, along with get_axis(), returns the rotation
//               represented by the quaternion as an angle about an
//               arbitrary axis.  This returns the angle, in radians
//               counterclockwise about the axis.
//
//               It is necessary to ensure the quaternion has been
//               normalized (for instance, with a call to normalize())
//               before calling this method.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LQuaternion)::
get_angle_rad() const {
  return acos(_v(0)) * 2.0;
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::get_angle
//       Access: Public
//  Description: This, along with get_axis(), returns the rotation
//               represented by the quaternion as an angle about an
//               arbitrary axis.  This returns the angle, in degrees
//               counterclockwise about the axis.
//
//               It is necessary to ensure the quaternion has been
//               normalized (for instance, with a call to normalize())
//               before calling this method.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LQuaternion)::
get_angle() const {
  return rad_2_deg(get_angle_rad());
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::set_from_axis_angle_rad
//       Access: Public
//  Description: angle_rad is the angle about the axis in radians.
//               axis must be normalized.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LQuaternion)::
set_from_axis_angle_rad(FLOATTYPE angle_rad, const FLOATNAME(LVector3) &axis) {
  nassertv(IS_THRESHOLD_EQUAL(axis.length(), 1.0f, 0.001f));
  FLOATTYPE sinHalfAngle = sin(angle_rad * 0.5);
  _v(0) = cos(angle_rad * 0.5);
  _v(1) = axis[0] * sinHalfAngle;
  _v(2) = axis[1] * sinHalfAngle;
  _v(3) = axis[2] * sinHalfAngle;
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::set_from_axis_angle_deg
//       Access: Public
//  Description: angle_deg is the angle about the axis in degrees.
//               axis must be normalized.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LQuaternion)::
set_from_axis_angle(FLOATTYPE angle_deg, const FLOATNAME(LVector3) &axis) {
  set_from_axis_angle_rad(deg_2_rad(angle_deg), axis);
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::get_up
//       Access: Public
//  Description: Returns the orientation represented by this
//               quaternion, expressed as an up vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LQuaternion)::
get_up(CoordinateSystem cs) const {
  return xform(FLOATNAME(LVector3)::up(cs));
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::get_right
//       Access: Public
//  Description: Returns the orientation represented by this
//               quaternion, expressed as a right vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LQuaternion)::
get_right(CoordinateSystem cs) const {
  return xform(FLOATNAME(LVector3)::right(cs));
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::get_forward
//       Access: Public
//  Description: Returns the orientation represented by this
//               quaternion, expressed as a forward vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVector3) FLOATNAME(LQuaternion)::
get_forward(CoordinateSystem cs) const {
  return xform(FLOATNAME(LVector3)::forward(cs));
}


////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::get_r
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LQuaternion)::
get_r() const {
  return _v(0);
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::get_i
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LQuaternion)::
get_i() const {
  return _v(1);
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::get_j
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LQuaternion)::
get_j() const {
  return _v(2);
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::get_k
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LQuaternion)::
get_k() const {
  return _v(3);
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::set_r
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LQuaternion)::
set_r(FLOATTYPE r) {
  _v(0) = r;
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::set_i
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LQuaternion)::
set_i(FLOATTYPE i) {
  _v(1) = i;
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::set_j
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LQuaternion)::
set_j(FLOATTYPE j) {
  _v(2) = j;
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::set_k
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LQuaternion)::
set_k(FLOATTYPE k) {
  _v(3) = k;
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::normalize
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LQuaternion)::
normalize() {
  FLOATTYPE length_squared = (*this).dot(*this);
  if (length_squared == (FLOATTYPE)0.0f) {
    set(0.0f, 0.0f, 0.0f, 0.0f);
    return false;
  } else if (!IS_THRESHOLD_EQUAL(length_squared, 1.0f, NEARLY_ZERO(FLOATTYPE))) {
    (*this) /= csqrt(length_squared);
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::conjugate
//       Access: Published
//  Description: Returns the complex conjugate of this quat.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LQuaternion) FLOATNAME(LQuaternion)::
conjugate() const {
  return FLOATNAME(LQuaternion)(_v(0), -_v(1), -_v(2), -_v(3));
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::conjugate_from
//       Access: Published
//  Description: Computes the conjugate of the other quat, and stores
//               the result in this quat.  This is a fully general
//               operation and makes no assumptions about the type of
//               transform represented by the quat.
//
//               The other quat must be a different object than this
//               quat.  However, if you need to get a conjugate of a
//               quat in place, see conjugate_in_place.
//
//               The return value is true if the quat was
//               successfully inverted, false if there was a
//               singularity.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LQuaternion)::
conjugate_from(const FLOATNAME(LQuaternion) &other) {
  set(other._v(0), -other._v(1), -other._v(2), -other._v(3));
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::conjugate_in_place
//       Access: Published
//  Description: Sets this to be the conjugate of the current quat.
//               Returns true if the successful, false if the quat
//               was singular.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LQuaternion)::
conjugate_in_place() {
  // _v(0) = _v(0);
  _v(1) = -_v(1);
  _v(2) = -_v(2);
  _v(3) = -_v(3);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::invert_from
//       Access: Public
//  Description: Computes the inverse of the other quat, and stores
//               the result in this quat.  This is a fully general
//               operation and makes no assumptions about the type of
//               transform represented by the quat.
//
//               The other quat must be a different object than this
//               quat.  However, if you need to invert a quat in
//               place, see invert_in_place.
//
//               The return value is true if the quat was
//               successfully inverted, false if there was a
//               singularity.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LQuaternion)::
invert_from(const FLOATNAME(LQuaternion) &other) {
  set(-other._v(0), other._v(1), other._v(2), other._v(3));
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::invert_in_place
//       Access: Public
//  Description: Inverts the current quat.  Returns true if the
//               inverse is successful, false if the quat was
//               singular.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LQuaternion)::
invert_in_place() {
  _v(0) = -_v(0);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::is_identity
//       Access: Public
//  Description: Returns true if this quaternion represents the
//               identity transformation: no rotation.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LQuaternion)::
is_identity() const {
  return is_almost_identity(NEARLY_ZERO(FLOATTYPE));
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::is_almost_identity
//       Access: Public
//  Description: Returns true if this quaternion represents the
//               identity transformation within a given tolerance.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LQuaternion)::
is_almost_identity(FLOATTYPE tolerance) const {
  return (IS_THRESHOLD_EQUAL(_v(0), -1.0f, tolerance) || 
          IS_THRESHOLD_EQUAL(_v(0), 1.0f, tolerance));
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::ident_quat
//       Access: Public, Static
//  Description: Returns an identity quaternion.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LQuaternion) &FLOATNAME(LQuaternion)::
ident_quat() {
  return _ident_quat;
}

////////////////////////////////////////////////////////////////////
//     Function: invert
//  Description: Inverts the given quat and returns it.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LQuaternion)
invert(const FLOATNAME(LQuaternion) &a) {
  FLOATNAME(LQuaternion) result;
  bool nonsingular = result.invert_from(a);
#ifndef NDEBUG
  if (!nonsingular) {
    nassert_raise("Attempt to compute inverse of singular quaternion!");
    return FLOATNAME(LQuaternion)::ident_quat();
  }
#endif
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: operator *(Matrix3, Quat)
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3) operator *(const FLOATNAME(LMatrix3) &m,
                             const FLOATNAME(LQuaternion) &q) {
  FLOATNAME(LMatrix3) q_matrix;
  q.extract_to_matrix(q_matrix);

  return m * q_matrix;
}

////////////////////////////////////////////////////////////////////
//     Function: operator *(Matrix4, Quat)
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4) operator *(const FLOATNAME(LMatrix4) &m,
                             const FLOATNAME(LQuaternion) &q) {
  FLOATNAME(LMatrix4) q_matrix;
  q.extract_to_matrix(q_matrix);

  // preserve the homogeneous coords and the translate
  FLOATNAME(LVector4) m_row3 = m.get_row(3);
  FLOATNAME(LVector4) m_col3 = m.get_col(3);

  q_matrix = m * q_matrix;
  q_matrix.set_row(3, m_row3);
  q_matrix.set_col(3, m_col3);

  return q_matrix;
}
