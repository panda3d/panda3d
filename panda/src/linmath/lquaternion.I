// Filename: lquaternion.I
// Created by:  frang (06Jun00)
// 
////////////////////////////////////////////////////////////////////

template<class NumType>
TypeHandle LQuaternionBase<NumType>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LQuaternionBase::Default Constructor
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LQuaternionBase<NumType>::
LQuaternionBase(void) {
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternionBase::Copy Constructor
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LQuaternionBase<NumType>::
LQuaternionBase(const LQuaternionBase<NumType>& c) : 
  _r(c._r), _i(c._i), _j(c._j), _k(c._k) {
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternionBase::Constructor
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LQuaternionBase<NumType>::
LQuaternionBase(NumType r, NumType i, NumType j, NumType k) {
  set(r, i, j, k);
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternionBase::Destructor
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
LQuaternionBase<NumType>::
~LQuaternionBase() {
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternionBase::pure_imaginary_quat
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
LQuaternionBase<NumType> LQuaternionBase<NumType>::
pure_imaginary(const LVector3<NumType> &v) {
  return LQuaternionBase<NumType>(0, v[0], v[1], v[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternionBase::ident_quat
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
const LQuaternionBase<NumType> &LQuaternionBase<NumType>::
ident_quat(void) {
  static LQuaternionBase<NumType> q(1, 0, 0, 0);
  return q;
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternionBase::set
//       Access: public
//  Description: assignment
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void LQuaternionBase<NumType>::
set(NumType r, NumType i, NumType j, NumType k) {
  _r = r;
  _i = i;
  _j = j;
  _k = k;
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternionBase::Assignment Operator
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LQuaternionBase<NumType>& LQuaternionBase<NumType>::
operator =(const LQuaternionBase<NumType>& c) {
  _r = c._r;
  _i = c._i;
  _j = c._j;
  _k = c._k;

  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternionBase::Equality Operator
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE bool LQuaternionBase<NumType>::
operator ==(const LQuaternionBase<NumType>& c) const {
  return (_r == c._r &&
	  _i == c._i &&
	  _j == c._j &&
	  _k == c._k);
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternionBase::Inequality Operator
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE bool LQuaternionBase<NumType>::
operator !=(const LQuaternionBase<NumType>& c) const {
  return !operator==(c);
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternionBase::multiply
//       Access: protected
//  Description: actual multiply call (non virtual)
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LQuaternionBase<NumType> LQuaternionBase<NumType>::
multiply(const LQuaternionBase<NumType>& rhs) const {
  NumType r = (_r * rhs._r) - (_i * rhs._i) - (_j * rhs._j) - (_k * rhs._k);
  NumType i = (_i * rhs._r) + (_r * rhs._i) - (_k * rhs._j) + (_j * rhs._k);
  NumType j = (_j * rhs._r) + (_k * rhs._i) + (_r * rhs._j) - (_i * rhs._k);
  NumType k = (_k * rhs._r) - (_j * rhs._i) + (_i * rhs._j) + (_r * rhs._k);

  return LQuaternionBase<NumType>(r, i , j, k);
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternionBase::Multiply Operator
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LQuaternionBase<NumType> LQuaternionBase<NumType>::
operator *(const LQuaternionBase<NumType>& c) {
  return multiply(c);
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternionBase::Multiply Assignment Operator
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LQuaternionBase<NumType>& LQuaternionBase<NumType>::
operator *=(const LQuaternionBase<NumType>& c) {
  (*this) = operator*(c);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternionBase::Multiply Operator
//       Access: public
//  Description: Quat * Matrix = matrix
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LMatrix3<NumType> LQuaternionBase<NumType>::
operator *(const LMatrix3<NumType> &m) {  
  LMatrix3<NumType> result;
  extract_to_matrix(result);
  return result * m;
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternionBase::Multiply Operator
//       Access: public
//  Description: Quat * Matrix = matrix
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LMatrix4<NumType> LQuaternionBase<NumType>::
operator *(const LMatrix4<NumType> &m) {
  LMatrix3<NumType> m_upper_3 = m.get_upper_3();
  LMatrix3<NumType> this_quat;
  extract_to_matrix(this_quat);

  LMatrix4<NumType> result;
  result.set_upper_3(this_quat * m_upper_3);
  result.set_row(3, m.get_row(3));
  result.set_col(3, m.get_col(3));

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternionBase::almost_equal
//       Access: public
//  Description: Returns true if two quaternions are memberwise equal
//               within a specified tolerance.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE bool LQuaternionBase<NumType>::
almost_equal(const LQuaternionBase<NumType>& c, NumType threshold) const {
  return (IS_THRESHOLD_EQUAL(_r, c._r, threshold) &&
	  IS_THRESHOLD_EQUAL(_i, c._i, threshold) &&
	  IS_THRESHOLD_EQUAL(_j, c._j, threshold) &&
	  IS_THRESHOLD_EQUAL(_k, c._k, threshold));
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternionBase::almost_equal
//       Access: public
//  Description: Returns true if two quaternions are memberwise equal
//               within a default tolerance based on the numeric type.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE bool LQuaternionBase<NumType>::
almost_equal(const LQuaternionBase<NumType>& c) const {
  return almost_equal(c, NEARLY_ZERO(NumType));
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternionBase::output
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void LQuaternionBase<NumType>::
output(ostream& os) const {
  os << MAYBE_ZERO(_r) << " + "
     << MAYBE_ZERO(_i) << "i + "
     << MAYBE_ZERO(_j) << "j + "
     << MAYBE_ZERO(_k) << "k";
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternionBase::get_r
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumType LQuaternionBase<NumType>::
get_r(void) const {
  return _r;
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternionBase::get_i
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumType LQuaternionBase<NumType>::
get_i(void) const {
  return _i;
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternionBase::get_j
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumType LQuaternionBase<NumType>::
get_j(void) const {
  return _j;
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternionBase::get_k
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumType LQuaternionBase<NumType>::
get_k(void) const {
  return _k;
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternionBase::set_r
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void LQuaternionBase<NumType>::
set_r(NumType r) {
  _r = r;
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternionBase::set_i
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void LQuaternionBase<NumType>::
set_i(NumType i) {
  _i = i;
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternionBase::set_j
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void LQuaternionBase<NumType>::
set_j(NumType j) {
  _j = j;
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternionBase::set_k
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void LQuaternionBase<NumType>::
set_k(NumType k) {
  _k = k;
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternionBase::normalize
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void LQuaternionBase<NumType>::
normalize(void) {
  NumType l = csqrt((_r*_r)+(_i*_i)+(_j*_j)+(_k*_k));

  if (l == 0.0) {
    _r = 0.;
    _i = 0.;
    _j = 0.;
    _k = 0.;
  } else {
    l = 1. / l;
    _r *= l;
    _i *= l;
    _j *= l;
    _k *= l;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: set
//       Access: public
//  Description: Do-While Jones.
////////////////////////////////////////////////////////////////////
template<class NumType>
void LQuaternionBase<NumType>::
set(const LMatrix3<NumType> &m) {
  NumType m00 = m.get_cell(0, 0);
  NumType m11 = m.get_cell(1, 1);
  NumType m22 = m.get_cell(2, 2);

  if (m00 != 0.0 && ((m11 + m22) != 0.0)) {
    _r = 1.0 + m00 + m11 + m22;
    _i = m.get_cell(2, 1) - m.get_cell(1, 2);
    _j = m.get_cell(0, 2) - m.get_cell(2, 0);
    _k = m.get_cell(1, 0) - m.get_cell(0, 1);
  }
  else if (m00 != 0.0 && ((m11 + m22) == 0.0)) {
    _r = m.get_cell(2, 1) - m.get_cell(1, 2);
    _i = 1.0 + m00 - m11 - m22;
    _j = m.get_cell(1, 0) + m.get_cell(0, 1);
    _k = m.get_cell(0, 2) + m.get_cell(2, 0);
  }
  else if (m00 == 0.0 && ((m11 - m22) != 0.0)) {
    _r = m.get_cell(0, 2) - m.get_cell(2, 0);
    _i = m.get_cell(1, 0) + m.get_cell(0, 1);
    _j = 1.0 - m00 + m11 - m22;
    _k = m.get_cell(2, 1) + m.get_cell(1, 2);
  }
  else {
    _r = m.get_cell(1, 0) - m.get_cell(0, 1);
    _i = m.get_cell(0, 2) + m.get_cell(2, 0);
    _j = m.get_cell(2, 1) + m.get_cell(1, 2);
    _k = 1.0 - m00 - m11 + m22;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: set
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void LQuaternionBase<NumType>::
set(const LMatrix4<NumType> &m) {
  set(m.get_upper_3());
}

////////////////////////////////////////////////////////////////////
//     Function: extract (LMatrix3)
//       Access: public
//  Description: Do-While Jones paper from cary.
////////////////////////////////////////////////////////////////////
template<class NumType>
void LQuaternionBase<NumType>::
extract_to_matrix(LMatrix3<NumType> &m) const {
  NumType tx, ty, tz, tq, tk, tk_denom;

  tx = _i * _i;
  ty = _j * _j;
  tz = _k * _k;
  tq = ty + tz;

  tk_denom = tq + tx + (_r * _r);
  if (tk_denom == 0.0)
    tk = 0.0;
  else
    tk = 2.0 / tk_denom;

  m.set_cell(0, 0, 1.0 - (tk * tq));
  m.set_cell(1, 1, 1.0 - (tk * (tx + tz)));
  m.set_cell(2, 2, 1.0 - (tk * (tx + ty)));
  tx = tk * _i;
  ty = tk * _j;
  tq = (tk * _k) * _r;
  tk = tx * _j;
  m.set_cell(0, 1, tk - tq);
  m.set_cell(1, 0, tk + tq);
  tq = ty * _r;
  tk = tx * _k;
  m.set_cell(0, 2, tk + tq);
  m.set_cell(2, 0, tk - tq);
  tq = tx * _r;
  tk = ty * _k;
  m.set_cell(1, 2, tk - tq);
  m.set_cell(2, 1, tk + tq);
}

////////////////////////////////////////////////////////////////////
//     Function: extract (LMatrix4)
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
void LQuaternionBase<NumType>::
extract_to_matrix(LMatrix4<NumType> &m) const {
  NumType tx, ty, tz, tq, tk, tk_denom;

  tx = _i * _i;
  ty = _j * _j;
  tz = _k * _k;
  tq = ty + tz;

  tk_denom = tq + tx + (_r * _r);
  if (tk_denom == 0.0)
    tk = 0.0;
  else
    tk = 2.0 / tk_denom;

  m.set_cell(0, 0, 1.0 - (tk * tq));
  m.set_cell(1, 1, 1.0 - (tk * (tx + tz)));
  m.set_cell(2, 2, 1.0 - (tk * (tx + ty)));
  tx = tk * _i;
  ty = tk * _j;
  tq = (tk * _k) * _r;
  tk = tx * _j;
  m.set_cell(0, 1, tk - tq);
  m.set_cell(1, 0, tk + tq);
  tq = ty * _r;
  tk = tx * _k;
  m.set_cell(0, 2, tk + tq);
  m.set_cell(2, 0, tk - tq);
  tq = tx * _r;
  tk = ty * _k;
  m.set_cell(1, 2, tk - tq);
  m.set_cell(2, 1, tk + tq);
}

////////////////////////////////////////////////////////////////////
//     Function: set_hpr
//       Access: public
//  Description: Sets the quaternion as the unit quaternion that
//               is equivalent to these Euler angles.
//               (from Real-time Rendering, p.49)
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void LQuaternionBase<NumType>::
set_hpr(const LVecBase3<NumType> &hpr) {
  LQuaternionBase<NumType> quat_h, quat_p, quat_r;

  quat_h.set(ccos(hpr[0]), 0, csin(hpr[0]), 0);
  quat_p.set(ccos(hpr[1]), csin(hpr[1]), 0, 0);
  quat_r.set(ccos(hpr[2]), 0, 0, csin(hpr[2]));

  (*this) = quat_h * quat_p * quat_r;
}

////////////////////////////////////////////////////////////////////
//     Function: get_hpr
//       Access: public
//  Description: Extracts the equivalent Euler angles from the unit
//               quaternion.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase3<NumType> LQuaternionBase<NumType>::
get_hpr() const {
  NumType sint = (2.0 * _r * _j) - (2.0 * _i * _k);
  NumType cost = csqrt(1 - sint * sint);

  NumType sinv, cosv, sinf, cosf;

  if (cost != 0.0) {
    sinv = ((2.0 * _j * _k) + (2.0 * _r * _i)) / cost;
    cosv = (1.0 - (2.0 * _i * _i) - (2.0 * _j * _j)) / cost;
    sinf = (1.0 - (2.0 * _i * _i) - (2.0 * _j * _j)) / cost;
    cosf = (1.0 - (2.0 * _j * _j) - (2.0 * _k * _k)) / cost;
    
  } else {
    sinv = ((2.0 * _r * _i) - (2.0 * _j * _k));
    cosv = 1.0 - (2.0 * _i * _i) - (2.0 * _k * _k);
    sinf = 0.0;
    cosf = 1.0;
  }

  return LVecBase3<NumType>(rad_2_deg(atan2(sinv, cosv)),
			    rad_2_deg(atan2(sint, cost)),
			    rad_2_deg(atan2(sinf, cosf)));
}

////////////////////////////////////////////////////////////////////
//     Function: operator *(Matrix3, Quat)
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
LMatrix3<NumType> operator *(const LMatrix3<NumType> &m,
			     const LQuaternionBase<NumType> &q) {
  LMatrix3<NumType> q_matrix;
  q.extract_to_matrix(q_matrix);

  return m * q_matrix;
}

////////////////////////////////////////////////////////////////////
//     Function: operator *(Matrix4, Quat)
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
LMatrix4<NumType> operator *(const LMatrix4<NumType> &m,
			     const LQuaternionBase<NumType> &q) {
  LMatrix4<NumType> q_matrix;
  q.extract_to_matrix(q_matrix);

  // preserve the homogeneous coords and the translate
  LVector4<NumType> m_row3 = m.get_row(3);
  LVector4<NumType> m_col3 = m.get_col(3);

  q_matrix = m * q_matrix;
  q_matrix.set_row(3, m_row3);
  q_matrix.set_col(3, m_col3);

  return q_matrix;
}

////////////////////////////////////////////////////////////////////
//     Function: LQuaternionBase::init_type
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
void LQuaternionBase<NumType>::
init_type(void) {
  if (_type_handle == TypeHandle::none()) {
    do_init_type(NumType);
    string name = "LQuaternionBase<" + get_type_handle(NumType).get_name()
                  + ">";
    register_type(_type_handle, name);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: lcast_to
//  Description: Converts a quaternion from one numeric representation
//               to another one.  This is usually invoked using the
//               macro LCAST.
////////////////////////////////////////////////////////////////////
template<class NumType, class NumType2>
INLINE LQuaternionBase<NumType2>
lcast_to(NumType2 *, const LQuaternionBase<NumType>& c) {
  return LQuaternionBase<NumType2>(c.get_r(), c.get_i(), c.get_j(), c.get_k());
}
