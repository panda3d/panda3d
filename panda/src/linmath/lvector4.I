// Filename: lvector4.I
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// 	 Class : LVector4
// Description : This is a four-component vector distance.
////////////////////////////////////////////////////////////////////

class EXPCL_PANDA FLOATNAME(LVector4) : public FLOATNAME(LVecBase4) {
PUBLISHED:
  INLINE FLOATNAME(LVector4)();
  INLINE FLOATNAME(LVector4)(const FLOATNAME(LVecBase4) &copy);
  INLINE FLOATNAME(LVector4) &operator = (const FLOATNAME(LVecBase4) &copy);
  INLINE FLOATNAME(LVector4) &operator = (FLOATTYPE1 fill_value);
  INLINE FLOATNAME(LVector4)(FLOATTYPE1 fill_value);
  INLINE FLOATNAME(LVector4)(FLOATTYPE1 x, FLOATTYPE1 y, FLOATTYPE1 z, FLOATTYPE1 w);

  INLINE static FLOATNAME(LVector4) zero();
  INLINE static FLOATNAME(LVector4) unit_x();
  INLINE static FLOATNAME(LVector4) unit_y();
  INLINE static FLOATNAME(LVector4) unit_z();
  INLINE static FLOATNAME(LVector4) unit_w();

  INLINE FLOATNAME(LVector4) operator - () const;

  INLINE FLOATNAME(LVecBase4) operator + (const FLOATNAME(LVecBase4) &other) const;
  INLINE FLOATNAME(LVector4)  operator + (const FLOATNAME(LVector4) &other) const;

  INLINE FLOATNAME(LVecBase4) operator - (const FLOATNAME(LVecBase4) &other) const;
  INLINE FLOATNAME(LVector4)  operator - (const FLOATNAME(LVector4) &other) const;

  INLINE FLOATTYPE1 length() const;
  INLINE FLOATTYPE1 length_squared() const;
  INLINE bool normalize();
  INLINE FLOATNAME(LVector4) operator * (FLOATTYPE1 scalar) const;
  INLINE FLOATNAME(LVector4) operator / (FLOATTYPE1 scalar) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();
 
private:
  static TypeHandle _type_handle;
};

////////////////////////////////////////////////////////////////////
//     Function: LVector4::Default Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector4)::
FLOATNAME(LVector4)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector4)::
FLOATNAME(LVector4)(const FLOATNAME(LVecBase4) &copy) : FLOATNAME(LVecBase4)(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector4) &FLOATNAME(LVector4)::
operator = (const FLOATNAME(LVecBase4) &copy) {
  FLOATNAME(LVecBase4)::operator = (copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::Copy Fill Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector4) &FLOATNAME(LVector4)::
operator = (FLOATTYPE1 fill_value) {
  FLOATNAME(LVecBase4)::operator = (fill_value);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector4)::
FLOATNAME(LVector4)(FLOATTYPE1 fill_value) :
  FLOATNAME(LVecBase4)(fill_value) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector4)::
FLOATNAME(LVector4)(FLOATTYPE1 x, FLOATTYPE1 y, FLOATTYPE1 z, FLOATTYPE1 w) : 
  FLOATNAME(LVecBase4)(x, y, z, w) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::zero Named Constructor
//       Access: Public
//  Description: Returns a zero-length vector.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector4) FLOATNAME(LVector4)::
zero() {
  return FLOATNAME(LVector4)(0.0, 0.0, 0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::unit_x Named Constructor
//       Access: Public
//  Description: Returns a unit X vector.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector4) FLOATNAME(LVector4)::
unit_x() {
  return FLOATNAME(LVector4)(1.0, 0.0, 0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::unit_y Named Constructor
//       Access: Public
//  Description: Returns a unit Y vector.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector4) FLOATNAME(LVector4)::
unit_y() {
  return FLOATNAME(LVector4)(0.0, 1.0, 0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::unit_z Named Constructor
//       Access: Public
//  Description: Returns a unit Z vector.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector4) FLOATNAME(LVector4)::
unit_z() {
  return FLOATNAME(LVector4)(0.0, 0.0, 1.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::unit_w Named Constructor
//       Access: Public
//  Description: Returns a unit W vector.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector4) FLOATNAME(LVector4)::
unit_w() {
  return FLOATNAME(LVector4)(0.0, 0.0, 0.0, 1.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::unary -
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector4) FLOATNAME(LVector4)::
operator - () const {
  return FLOATNAME(LVecBase4)::operator - ();
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::vector + vecbase
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase4) FLOATNAME(LVector4)::
operator + (const FLOATNAME(LVecBase4) &other) const {
  return FLOATNAME(LVecBase4)::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::vector + vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector4) FLOATNAME(LVector4)::
operator + (const FLOATNAME(LVector4) &other) const {
  return FLOATNAME(LVecBase4)::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::vector - vecbase
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase4) FLOATNAME(LVector4)::
operator - (const FLOATNAME(LVecBase4) &other) const {
  return FLOATNAME(LVecBase4)::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::vector - vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector4) FLOATNAME(LVector4)::
operator - (const FLOATNAME(LVector4) &other) const {
  return FLOATNAME(LVecBase4)::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::length
//       Access: Public
//  Description: Returns the length of the vector, by the Pythagorean
//               theorem.
////////////////////////////////////////////////////////////////////

INLINE FLOATTYPE1 FLOATNAME(LVector4)::
length() const {
  return csqrt((*this).dot(*this));
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::length_squared
//       Access: Public
//  Description: Returns the square of the vector's length, cheap and
//               easy.
////////////////////////////////////////////////////////////////////

INLINE FLOATTYPE1 FLOATNAME(LVector4)::
length_squared() const {
  return (*this).dot(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::normalize
//       Access: Public
//  Description: Normalizes the vector in place.  Returns true if the
//               vector was normalized, false if it was a zero-length
//               vector.
////////////////////////////////////////////////////////////////////

INLINE bool FLOATNAME(LVector4)::
normalize() {
  FLOATTYPE1 l2 = length_squared();
  if (l2 == (FLOATTYPE1)0.0) {
    set(0.0, 0.0, 0.0, 0.0);
    return false;

  } else if (!IS_THRESHOLD_EQUAL(l2, 1.0, NEARLY_ZERO(FLOATTYPE1) * NEARLY_ZERO(FLOATTYPE1))) {
    (*this) /= csqrt(l2);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::operator * scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector4) FLOATNAME(LVector4)::
operator * (FLOATTYPE1 scalar) const {
  return FLOATNAME(LVector4)(FLOATNAME(LVecBase4)::operator * (scalar));
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::operator / scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector4) FLOATNAME(LVector4)::
operator / (FLOATTYPE1 scalar) const {
  return FLOATNAME(LVector4)(FLOATNAME(LVecBase4)::operator / (scalar));
}

