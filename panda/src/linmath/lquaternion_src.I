// Filename: lquaternion_src.I
// Created by:  frang (06Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternion)::Default Constructor
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LQuaternion)::
FLOATNAME(LQuaternion)() {
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternion)::Copy Constructor
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LQuaternion)::
FLOATNAME(LQuaternion)(const FLOATNAME(LQuaternion) &c) :
  FLOATNAME(LVecBase4)(c)
{
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternion)::Constructor
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LQuaternion)::
FLOATNAME(LQuaternion)(FLOATTYPE r, FLOATTYPE i, FLOATTYPE j, FLOATTYPE k) {
  set(r, i, j, k);
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternion)::multiply
//       Access: protected
//  Description: actual multiply call (non virtual)
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LQuaternion) FLOATNAME(LQuaternion)::
multiply(const FLOATNAME(LQuaternion)& rhs) const {
  FLOATTYPE r = (_v.data[0] * rhs._v.data[0]) - (_v.data[1] * rhs._v.data[1]) - (_v.data[2] * rhs._v.data[2]) - (_v.data[3] * rhs._v.data[3]);
  FLOATTYPE i = (_v.data[1] * rhs._v.data[0]) + (_v.data[0] * rhs._v.data[1]) - (_v.data[3] * rhs._v.data[2]) + (_v.data[2] * rhs._v.data[3]);
  FLOATTYPE j = (_v.data[2] * rhs._v.data[0]) + (_v.data[3] * rhs._v.data[1]) + (_v.data[0] * rhs._v.data[2]) - (_v.data[1] * rhs._v.data[3]);
  FLOATTYPE k = (_v.data[3] * rhs._v.data[0]) - (_v.data[2] * rhs._v.data[1]) + (_v.data[1] * rhs._v.data[2]) + (_v.data[0] * rhs._v.data[3]);

  return FLOATNAME(LQuaternion)(r, i , j, k);
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternion)::Multiply Operator
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LQuaternion) FLOATNAME(LQuaternion)::
operator *(const FLOATNAME(LQuaternion)& c) {
  return multiply(c);
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternion)::Multiply Assignment Operator
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LQuaternion)& FLOATNAME(LQuaternion)::
operator *=(const FLOATNAME(LQuaternion)& c) {
  (*this) = operator*(c);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternion)::Multiply Operator
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
//     Function: FLOATNAME(LQuaternion)::Multiply Operator
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
//     Function: FLOATNAME(LQuaternion)::almost_equal
//       Access: public
//  Description: Returns true if two quaternions are memberwise equal
//               within a specified tolerance.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LQuaternion)::
almost_equal(const FLOATNAME(LQuaternion)& c, FLOATTYPE threshold) const {
  return (IS_THRESHOLD_EQUAL(_v.data[0], c._v.data[0], threshold) &&
          IS_THRESHOLD_EQUAL(_v.data[1], c._v.data[1], threshold) &&
          IS_THRESHOLD_EQUAL(_v.data[2], c._v.data[2], threshold) &&
          IS_THRESHOLD_EQUAL(_v.data[3], c._v.data[3], threshold));
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternion)::almost_equal
//       Access: public
//  Description: Returns true if two quaternions are memberwise equal
//               within a default tolerance based on the numeric type.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LQuaternion)::
almost_equal(const FLOATNAME(LQuaternion)& c) const {
  return almost_equal(c, NEARLY_ZERO(FLOATTYPE));
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternion)::output
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LQuaternion)::
output(ostream& os) const {
  os << MAYBE_ZERO(_v.data[0]) << " + "
     << MAYBE_ZERO(_v.data[1]) << "i + "
     << MAYBE_ZERO(_v.data[2]) << "j + "
     << MAYBE_ZERO(_v.data[3]) << "k";
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternion)::get_r
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LQuaternion)::
get_r() const {
  return _v.data[0];
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternion)::get_i
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LQuaternion)::
get_i() const {
  return _v.data[1];
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternion)::get_j
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LQuaternion)::
get_j() const {
  return _v.data[2];
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternion)::get_k
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LQuaternion)::
get_k() const {
  return _v.data[3];
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternion)::set_r
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LQuaternion)::
set_r(FLOATTYPE r) {
  _v.data[0] = r;
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternion)::set_i
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LQuaternion)::
set_i(FLOATTYPE i) {
  _v.data[1] = i;
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternion)::set_j
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LQuaternion)::
set_j(FLOATTYPE j) {
  _v.data[2] = j;
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternion)::set_k
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LQuaternion)::
set_k(FLOATTYPE k) {
  _v.data[3] = k;
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternion)::normalize
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LQuaternion)::
normalize() {
  FLOATTYPE l2 = (*this).dot(*this);
  if (l2 == (FLOATTYPE)0.0f) {
    set(0.0f, 0.0f, 0.0f, 0.0f);
    return false;

  } else if (!IS_THRESHOLD_EQUAL(l2, 1.0f, NEARLY_ZERO(FLOATTYPE) * NEARLY_ZERO(FLOATTYPE))) {
    (*this) /= csqrt(l2);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: set_from_matrix
//       Access: public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LQuaternion)::
set_from_matrix(const FLOATNAME(LMatrix4) &m) {
  set_from_matrix(m.get_upper_3());
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


////////////////////////////////////////////////////////////////////
//     Function: LQuaternion::ident_quat
//       Access: Public, Static
//  Description: Returns an identity quaternion.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LQuaternion) &FLOATNAME(LQuaternion)::
ident_quat() {
  return _ident_quat;
}
