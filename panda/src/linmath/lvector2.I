// Filename: lvector2.I
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// 	 Class : LVector2
// Description : This is a two-component vector offset.
////////////////////////////////////////////////////////////////////

class EXPCL_PANDA FLOATNAME(LVector2) : public FLOATNAME(LVecBase2) {
PUBLISHED:
  INLINE FLOATNAME(LVector2)();
  INLINE FLOATNAME(LVector2)(const FLOATNAME(LVecBase2) &copy);
  INLINE FLOATNAME(LVector2) &operator = (const FLOATNAME(LVecBase2) &copy);
  INLINE FLOATNAME(LVector2) &operator = (FLOATTYPE1 fill_value);
  INLINE FLOATNAME(LVector2)(FLOATTYPE1 fill_value);
  INLINE FLOATNAME(LVector2)(FLOATTYPE1 x, FLOATTYPE1 y);

  INLINE static FLOATNAME(LVector2) zero();
  INLINE static FLOATNAME(LVector2) unit_x();
  INLINE static FLOATNAME(LVector2) unit_y();

  INLINE FLOATNAME(LVector2) operator - () const;

  INLINE FLOATNAME(LVecBase2)operator + (const FLOATNAME(LVecBase2) &other) const;
  INLINE FLOATNAME(LVector2) operator + (const FLOATNAME(LVector2) &other) const;

  INLINE FLOATNAME(LVecBase2) operator - (const FLOATNAME(LVecBase2) &other) const;
  INLINE FLOATNAME(LVector2) operator - (const FLOATNAME(LVector2) &other) const;

  INLINE FLOATTYPE1 length() const;
  INLINE FLOATTYPE1 length_squared() const;
  INLINE bool normalize();
  INLINE FLOATNAME(LVector2) operator * (FLOATTYPE1 scalar) const;
  INLINE FLOATNAME(LVector2) operator / (FLOATTYPE1 scalar) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();
 
private:
  static TypeHandle _type_handle;
};

////////////////////////////////////////////////////////////////////
//     Function: LVector2::Default Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector2)::
FLOATNAME(LVector2)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector2)::
FLOATNAME(LVector2)(const FLOATNAME(LVecBase2) &copy) : FLOATNAME(LVecBase2)(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector2) &FLOATNAME(LVector2)::
operator = (const FLOATNAME(LVecBase2) &copy) {
  FLOATNAME(LVecBase2)::operator = (copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::Copy Fill Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector2) &FLOATNAME(LVector2)::
operator = (FLOATTYPE1 fill_value) {
  FLOATNAME(LVecBase2)::operator = (fill_value);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector2)::
FLOATNAME(LVector2)(FLOATTYPE1 fill_value) :
  FLOATNAME(LVecBase2)(fill_value) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector2)::
FLOATNAME(LVector2)(FLOATTYPE1 x, FLOATTYPE1 y) : 
  FLOATNAME(LVecBase2)(x, y) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::zero Named Constructor
//       Access: Public
//  Description: Returns a zero-length vector.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector2) FLOATNAME(LVector2)::
zero() {
  return FLOATNAME(LVector2)(0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::unit_x Named Constructor
//       Access: Public
//  Description: Returns a unit X vector.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector2) FLOATNAME(LVector2)::
unit_x() {
  return FLOATNAME(LVector2)(1.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::unit_y Named Constructor
//       Access: Public
//  Description: Returns a unit Y vector.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector2) FLOATNAME(LVector2)::
unit_y() {
  return FLOATNAME(LVector2)(0.0, 1.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::unary -
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector2) FLOATNAME(LVector2)::
operator - () const {
  return FLOATNAME(LVecBase2)::operator - ();
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::vector + vecbase
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase2) FLOATNAME(LVector2)::
operator + (const FLOATNAME(LVecBase2) &other) const {
  return FLOATNAME(LVecBase2)::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::vector + vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector2) FLOATNAME(LVector2)::
operator + (const FLOATNAME(LVector2) &other) const {
  return FLOATNAME(LVecBase2)::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::vector - vecbase
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase2) FLOATNAME(LVector2)::
operator - (const FLOATNAME(LVecBase2) &other) const {
  return FLOATNAME(LVecBase2)::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::vector - vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector2) FLOATNAME(LVector2)::
operator - (const FLOATNAME(LVector2) &other) const {
  return FLOATNAME(LVecBase2)::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::length
//       Access: Public
//  Description: Returns the length of the vector, by the Pythagorean
//               theorem.
////////////////////////////////////////////////////////////////////

INLINE FLOATTYPE1 FLOATNAME(LVector2)::
length() const {
  return csqrt((*this).dot(*this));
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::length_squared
//       Access: Public
//  Description: Returns the square of the vector's length, cheap and
//               easy.
////////////////////////////////////////////////////////////////////

INLINE FLOATTYPE1 FLOATNAME(LVector2)::
length_squared() const {
  return (*this).dot(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::normalize
//       Access: Public
//  Description: Normalizes the vector in place.  Returns true if the
//               vector was normalized, false if it was a zero-length
//               vector.
////////////////////////////////////////////////////////////////////

INLINE bool FLOATNAME(LVector2)::
normalize() {
  FLOATTYPE1 l2 = length_squared();
  if (l2 == (FLOATTYPE1)0.0) {
    set(0.0, 0.0);
    return false;

  } else if (!IS_THRESHOLD_EQUAL(l2, 1.0, NEARLY_ZERO(FLOATTYPE1) * NEARLY_ZERO(FLOATTYPE1))) {
    (*this) /= csqrt(l2);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::operator * scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector2) FLOATNAME(LVector2)::
operator * (FLOATTYPE1 scalar) const {
  return FLOATNAME(LVector2)(FLOATNAME(LVecBase2)::operator * (scalar));
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::operator / scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector2) FLOATNAME(LVector2)::
operator / (FLOATTYPE1 scalar) const {
  return FLOATNAME(LVector2)(FLOATNAME(LVecBase2)::operator / (scalar));
}

