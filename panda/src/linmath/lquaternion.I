// Filename: lquaternion.I
// Created by:  frang (06Jun00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : FLOATNAME(LQuaternionBase)
// Description : This is the base quaternion class
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FLOATNAME(LQuaternionBase)  {
protected:
  INLINE FLOATNAME(LQuaternionBase) 
    multiply(const FLOATNAME(LQuaternionBase)&) const;

PUBLISHED:
  INLINE FLOATNAME(LQuaternionBase)(void);
  INLINE FLOATNAME(LQuaternionBase)(const FLOATNAME(LQuaternionBase) &);
  INLINE FLOATNAME(LQuaternionBase)(FLOATTYPE1, FLOATTYPE1, FLOATTYPE1, FLOATTYPE1);
  virtual ~FLOATNAME(LQuaternionBase)(void);

  static FLOATNAME(LQuaternionBase) pure_imaginary(const FLOATNAME(LVector3) &);

  INLINE FLOATNAME(LQuaternionBase)& operator =(const FLOATNAME(LQuaternionBase) &);
  INLINE bool operator ==(const FLOATNAME(LQuaternionBase) &) const;
  INLINE bool operator !=(const FLOATNAME(LQuaternionBase) &) const;

  INLINE FLOATNAME(LQuaternionBase) operator *(const FLOATNAME(LQuaternionBase) &);
  INLINE FLOATNAME(LQuaternionBase)& operator *=(const FLOATNAME(LQuaternionBase) &);

  INLINE FLOATNAME(LMatrix3) operator *(const FLOATNAME(LMatrix3) &);
  INLINE FLOATNAME(LMatrix4) operator *(const FLOATNAME(LMatrix4) &);

  INLINE bool almost_equal(const FLOATNAME(LQuaternionBase) &, FLOATTYPE1) const;
  INLINE bool almost_equal(const FLOATNAME(LQuaternionBase) &) const;

  INLINE void output(ostream&) const;

  INLINE void set(FLOATTYPE1, FLOATTYPE1, FLOATTYPE1, FLOATTYPE1);

  void set(const FLOATNAME(LMatrix3) &m);
  INLINE void set(const FLOATNAME(LMatrix4) &m);

  void extract_to_matrix(FLOATNAME(LMatrix3) &m) const;
  void extract_to_matrix(FLOATNAME(LMatrix4) &m) const;

  void set_hpr(const FLOATNAME(LVecBase3) &hpr);
  FLOATNAME(LVecBase3) get_hpr() const;

  INLINE FLOATTYPE1 get_r(void) const;
  INLINE FLOATTYPE1 get_i(void) const;
  INLINE FLOATTYPE1 get_j(void) const;
  INLINE FLOATTYPE1 get_k(void) const;

  INLINE void set_r(FLOATTYPE1 r);
  INLINE void set_i(FLOATTYPE1 i);
  INLINE void set_j(FLOATTYPE1 j);
  INLINE void set_k(FLOATTYPE1 k);

  INLINE void normalize(void);

  static const FLOATNAME(LQuaternionBase) &ident_quat(void);

private:
  FLOATTYPE1 _r, _i, _j, _k;
public:
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void);
private:
  static TypeHandle _type_handle;
};


INLINE ostream& operator<<(ostream& os, const FLOATNAME(LQuaternionBase)& q) {
  q.output(os);
  return os;
}

BEGIN_PUBLISH
INLINE FLOATNAME(LMatrix3)
operator * (const FLOATNAME(LMatrix3) &m, const FLOATNAME(LQuaternionBase) &q);
INLINE FLOATNAME(LMatrix4)
operator * (const FLOATNAME(LMatrix4) &m, const FLOATNAME(LQuaternionBase) &q);
END_PUBLISH

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternionBase)::Default Constructor
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LQuaternionBase)::
FLOATNAME(LQuaternionBase)(void) {
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternionBase)::Copy Constructor
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LQuaternionBase)::
FLOATNAME(LQuaternionBase)(const FLOATNAME(LQuaternionBase)& c) : 
  _r(c._r), _i(c._i), _j(c._j), _k(c._k) {
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternionBase)::Constructor
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LQuaternionBase)::
FLOATNAME(LQuaternionBase)(FLOATTYPE1 r, FLOATTYPE1 i, FLOATTYPE1 j, FLOATTYPE1 k) {
  set(r, i, j, k);
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternionBase)::set
//       Access: public
//  Description: assignment
////////////////////////////////////////////////////////////////////

INLINE void FLOATNAME(LQuaternionBase)::
set(FLOATTYPE1 r, FLOATTYPE1 i, FLOATTYPE1 j, FLOATTYPE1 k) {
  _r = r;
  _i = i;
  _j = j;
  _k = k;
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternionBase)::Assignment Operator
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LQuaternionBase)& FLOATNAME(LQuaternionBase)::
operator =(const FLOATNAME(LQuaternionBase)& c) {
  _r = c._r;
  _i = c._i;
  _j = c._j;
  _k = c._k;

  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternionBase)::Equality Operator
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE bool FLOATNAME(LQuaternionBase)::
operator ==(const FLOATNAME(LQuaternionBase)& c) const {
  return (_r == c._r &&
	  _i == c._i &&
	  _j == c._j &&
	  _k == c._k);
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternionBase)::Inequality Operator
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE bool FLOATNAME(LQuaternionBase)::
operator !=(const FLOATNAME(LQuaternionBase)& c) const {
  return !operator==(c);
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternionBase)::multiply
//       Access: protected
//  Description: actual multiply call (non virtual)
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LQuaternionBase) FLOATNAME(LQuaternionBase)::
multiply(const FLOATNAME(LQuaternionBase)& rhs) const {
  FLOATTYPE1 r = (_r * rhs._r) - (_i * rhs._i) - (_j * rhs._j) - (_k * rhs._k);
  FLOATTYPE1 i = (_i * rhs._r) + (_r * rhs._i) - (_k * rhs._j) + (_j * rhs._k);
  FLOATTYPE1 j = (_j * rhs._r) + (_k * rhs._i) + (_r * rhs._j) - (_i * rhs._k);
  FLOATTYPE1 k = (_k * rhs._r) - (_j * rhs._i) + (_i * rhs._j) + (_r * rhs._k);

  return FLOATNAME(LQuaternionBase)(r, i , j, k);
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternionBase)::Multiply Operator
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LQuaternionBase) FLOATNAME(LQuaternionBase)::
operator *(const FLOATNAME(LQuaternionBase)& c) {
  return multiply(c);
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternionBase)::Multiply Assignment Operator
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LQuaternionBase)& FLOATNAME(LQuaternionBase)::
operator *=(const FLOATNAME(LQuaternionBase)& c) {
  (*this) = operator*(c);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternionBase)::Multiply Operator
//       Access: public
//  Description: Quat * Matrix = matrix
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LMatrix3) FLOATNAME(LQuaternionBase)::
operator *(const FLOATNAME(LMatrix3) &m) {  
  FLOATNAME(LMatrix3) result;
  extract_to_matrix(result);
  return result * m;
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternionBase)::Multiply Operator
//       Access: public
//  Description: Quat * Matrix = matrix
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LMatrix4) FLOATNAME(LQuaternionBase)::
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
//     Function: FLOATNAME(LQuaternionBase)::almost_equal
//       Access: public
//  Description: Returns true if two quaternions are memberwise equal
//               within a specified tolerance.
////////////////////////////////////////////////////////////////////

INLINE bool FLOATNAME(LQuaternionBase)::
almost_equal(const FLOATNAME(LQuaternionBase)& c, FLOATTYPE1 threshold) const {
  return (IS_THRESHOLD_EQUAL(_r, c._r, threshold) &&
	  IS_THRESHOLD_EQUAL(_i, c._i, threshold) &&
	  IS_THRESHOLD_EQUAL(_j, c._j, threshold) &&
	  IS_THRESHOLD_EQUAL(_k, c._k, threshold));
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternionBase)::almost_equal
//       Access: public
//  Description: Returns true if two quaternions are memberwise equal
//               within a default tolerance based on the numeric type.
////////////////////////////////////////////////////////////////////

INLINE bool FLOATNAME(LQuaternionBase)::
almost_equal(const FLOATNAME(LQuaternionBase)& c) const {
  return almost_equal(c, NEARLY_ZERO(FLOATTYPE1));
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternionBase)::output
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE void FLOATNAME(LQuaternionBase)::
output(ostream& os) const {
  os << MAYBE_ZERO(_r) << " + "
     << MAYBE_ZERO(_i) << "i + "
     << MAYBE_ZERO(_j) << "j + "
     << MAYBE_ZERO(_k) << "k";
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternionBase)::get_r
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATTYPE1 FLOATNAME(LQuaternionBase)::
get_r(void) const {
  return _r;
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternionBase)::get_i
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATTYPE1 FLOATNAME(LQuaternionBase)::
get_i(void) const {
  return _i;
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternionBase)::get_j
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATTYPE1 FLOATNAME(LQuaternionBase)::
get_j(void) const {
  return _j;
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternionBase)::get_k
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATTYPE1 FLOATNAME(LQuaternionBase)::
get_k(void) const {
  return _k;
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternionBase)::set_r
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE void FLOATNAME(LQuaternionBase)::
set_r(FLOATTYPE1 r) {
  _r = r;
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternionBase)::set_i
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE void FLOATNAME(LQuaternionBase)::
set_i(FLOATTYPE1 i) {
  _i = i;
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternionBase)::set_j
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE void FLOATNAME(LQuaternionBase)::
set_j(FLOATTYPE1 j) {
  _j = j;
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternionBase)::set_k
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE void FLOATNAME(LQuaternionBase)::
set_k(FLOATTYPE1 k) {
  _k = k;
}

////////////////////////////////////////////////////////////////////
//     Function: FLOATNAME(LQuaternionBase)::normalize
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE void FLOATNAME(LQuaternionBase)::
normalize(void) {
  FLOATTYPE1 l = csqrt((_r*_r)+(_i*_i)+(_j*_j)+(_k*_k));

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
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE void FLOATNAME(LQuaternionBase)::
set(const FLOATNAME(LMatrix4) &m) {
  set(m.get_upper_3());
}

////////////////////////////////////////////////////////////////////
//     Function: operator *(Matrix3, Quat)
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LMatrix3) operator *(const FLOATNAME(LMatrix3) &m,
			     const FLOATNAME(LQuaternionBase) &q) {
  FLOATNAME(LMatrix3) q_matrix;
  q.extract_to_matrix(q_matrix);

  return m * q_matrix;
}

////////////////////////////////////////////////////////////////////
//     Function: operator *(Matrix4, Quat)
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LMatrix4) operator *(const FLOATNAME(LMatrix4) &m,
			     const FLOATNAME(LQuaternionBase) &q) {
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

