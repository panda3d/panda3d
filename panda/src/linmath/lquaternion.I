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
  NumType m01 = m.get_cell(0, 1);
  NumType m02 = m.get_cell(0, 2);
  NumType m10 = m.get_cell(1, 0);
  NumType m11 = m.get_cell(1, 1);
  NumType m12 = m.get_cell(1, 2);
  NumType m20 = m.get_cell(2, 0);
  NumType m21 = m.get_cell(2, 1);
  NumType m22 = m.get_cell(2, 2);

  NumType T = m00 + m11 + m22 + 1.;

  if (T > 0.) {
    // the easy case
    NumType S = 0.5 / csqrt(T);
    _r = 0.25 / S;
    _i = (m21 - m12) * S;
    _j = (m02 - m20) * S;
    _k = (m10 - m01) * S;
  } else {
    // figure out which column to take as root
    int c = 0;
    if (cabs(m00) > cabs(m11)) {
      if (cabs(m00) > cabs(m22))
	c = 0;
      else
	c = 2;
    } else if (cabs(m11) > cabs(m22))
      c = 1;
    else
      c = 2;

    NumType S;

    switch (c) {
    case 0:
      S = csqrt(1. + m00 - m11 - m22) * 2.;
      _r = (m12 + m21) / S;
      _i = 0.5 / S;
      _j = (m01 + m10) / S;
      _k = (m02 + m20) / S;
      break;
    case 1:
      S = csqrt(1. + m11 - m00 - m22) * 2.;
      _r = (m02 + m20) / S;
      _i = (m01 + m10) / S;
      _j = 0.5 / S;
      _k = (m12 + m21) / S;
      break;
    case 2:
      S = csqrt(1. + m22 - m00 - m11) * 2.;
      _r = (m01 + m10) / S;
      _i = (m02 + m20) / S;
      _j = (m12 + m21) / S;
      _k = 0.5 / S;
      break;
    }
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
  NumType N = (_r * _r) + (_i * _i) + (_j * _j) + (_k * _k);
  NumType s = (N == 0.) ? 0. : (2. / N);
  NumType xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;

  xs = _i * s;   ys = _j * s;   zs = _k * s;
  wx = _r * xs;  wy = _r * ys;  wz = _r * zs;
  xx = _i * xs;  xy = _i * ys;  xz = _i * zs;
  yy = _j * ys;  yz = _j * zs;  zz = _k * zs;

  m = LMatrix3<NumType>((1. - (yy + zz)), (xy - wz), (xz + wy),
			(xy + wz), (1. - (xx + zz)), (yz - wx),
			(xz - wy), (yz + wx), (1. - (xx + yy)));
}

////////////////////////////////////////////////////////////////////
//     Function: extract (LMatrix4)
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
void LQuaternionBase<NumType>::
extract_to_matrix(LMatrix4<NumType> &m) const {
  NumType N = (_r * _r) + (_i * _i) + (_j * _j) + (_k * _k);
  NumType s = (N == 0.) ? 0. : (2. / N);
  NumType xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;

  xs = _i * s;   ys = _j * s;   zs = _k * s;
  wx = _r * xs;  wy = _r * ys;  wz = _r * zs;
  xx = _i * xs;  xy = _i * ys;  xz = _i * zs;
  yy = _j * ys;  yz = _j * zs;  zz = _k * zs;

  m = LMatrix4<NumType>((1. - (yy + zz)), (xy - wz), (xz + wy), 0.,
			(xy + wz), (1. - (xx + zz)), (yz - wx), 0.,
			(xz - wy), (yz + wx), (1. - (xx + yy)), 0.,
			0., 0., 0., 1.);
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

  LVector3<NumType> v = LVector3<NumType>::up();
  NumType a = deg_2_rad(hpr[0] * 0.5);
  NumType s,c;

  csincos(a,&s,&c);
  quat_h.set(c, v[0] * s, v[1] * s, v[2] * s);
  v = LVector3<NumType>::right();
  a = deg_2_rad(hpr[1] * 0.5);
  csincos(a,&s,&c);
  s = csin(a);
  quat_p.set(c, v[0] * s, v[1] * s, v[2] * s);
  v = LVector3<NumType>::forward();
  a = deg_2_rad(hpr[2] * 0.5);
  csincos(a,&s,&c);
  quat_r.set(c, v[0] * s, v[1] * s, v[2] * s);

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
  NumType heading, pitch, roll;
  NumType N = (_r * _r) + (_i * _i) + (_j * _j) + (_k * _k);
  NumType s = (N == 0.) ? 0. : (2. / N);
  NumType xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz, c1, c2, c3, c4;
  NumType cr, sr, cp, sp, ch, sh;

  xs = _i * s;   ys = _j * s;   zs = _k * s;
  wx = _r * xs;  wy = _r * ys;  wz = _r * zs;
  xx = _i * xs;  xy = _i * ys;  xz = _i * zs;
  yy = _j * ys;  yz = _j * zs;  zz = _k * zs;
  c1 = xz - wy;
  c2 = 1. - (xx + yy);
  c3 = 1. - (yy + zz);
  c4 = xy + wz;

  if (c1 == 0.) {  // (roll = 0 or 180) or (pitch = +/- 90
    if (c2 >= 0.) {
      roll = 0.;
      ch = c3;
      sh = c4;
      cp = c2;
    } else {
      roll = 180.;
      ch = -c3;
      sh = -c4;
      cp = -c2;
    }
  } else {
    // this should work all the time, but the above saves some trig operations
    roll = catan2(-c1, c2);
	csincos(roll,&sr,&cr);
    roll = rad_2_deg(roll);
    ch = (cr * c3) + (sr * (xz + wy));
    sh = (cr * c4) + (sr * (yz - wx));
    cp = (cr * c2) - (sr * c1);
  }
  sp = yz + wx;
  heading = rad_2_deg(catan2(sh, ch));
  pitch = rad_2_deg(catan2(sp, cp));

  return LVecBase3<NumType>(heading, pitch, roll);
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
