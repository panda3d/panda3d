// Filename: lvector3.I
// Created by:  drose (24Sep99)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// 	 Class : LVector3
// Description : This is a three-component vector distance (as opposed
//               to a three-component point, which represents a
//               particular point in space).  Some of the methods are
//               slightly different between LPoint3 and LVector3; in
//               particular, subtraction of two points yields a
//               vector, while addition of a vector and a point yields
//               a point.
////////////////////////////////////////////////////////////////////

class EXPCL_PANDA FLOATNAME(LVector3) : public FLOATNAME(LVecBase3) {
PUBLISHED:
  INLINE FLOATNAME(LVector3)();
  INLINE FLOATNAME(LVector3)(const FLOATNAME(LVecBase3) &copy);
  INLINE FLOATNAME(LVector3) &operator = (const FLOATNAME(LVecBase3) &copy);
  INLINE FLOATNAME(LVector3) &operator = (FLOATTYPE1 fill_value);
  INLINE FLOATNAME(LVector3)(FLOATTYPE1 fill_value);
  INLINE FLOATNAME(LVector3)(FLOATTYPE1 x, FLOATTYPE1 y, FLOATTYPE1 z);

  INLINE static FLOATNAME(LVector3) zero();
  INLINE static FLOATNAME(LVector3) unit_x();
  INLINE static FLOATNAME(LVector3) unit_y();
  INLINE static FLOATNAME(LVector3) unit_z();

  INLINE FLOATNAME(LVector3) operator - () const;

  INLINE FLOATNAME(LVecBase3) operator + (const FLOATNAME(LVecBase3) &other) const;
  INLINE FLOATNAME(LVector3) operator + (const FLOATNAME(LVector3) &other) const;

  INLINE FLOATNAME(LVecBase3) operator - (const FLOATNAME(LVecBase3) &other) const;
  INLINE FLOATNAME(LVector3) operator - (const FLOATNAME(LVector3) &other) const;

  INLINE FLOATTYPE1 length() const;
  INLINE FLOATTYPE1 length_squared() const;
  INLINE bool normalize();
  INLINE FLOATNAME(LVector3) cross(const FLOATNAME(LVecBase3) &other) const;
  INLINE FLOATNAME(LVector3) operator * (FLOATTYPE1 scalar) const;
  INLINE FLOATNAME(LVector3) operator / (FLOATTYPE1 scalar) const;

  // Some special named constructors for LVector3.

  INLINE static FLOATNAME(LVector3) up(CoordinateSystem cs = CS_default);
  INLINE static FLOATNAME(LVector3) right(CoordinateSystem cs = CS_default);
  INLINE static FLOATNAME(LVector3) forward(CoordinateSystem cs = CS_default);

  INLINE static FLOATNAME(LVector3) down(CoordinateSystem cs = CS_default);
  INLINE static FLOATNAME(LVector3) left(CoordinateSystem cs = CS_default);
  INLINE static FLOATNAME(LVector3) back(CoordinateSystem cs = CS_default);

  INLINE static FLOATNAME(LVector3) rfu(FLOATTYPE1 right,
				      FLOATTYPE1 fwd,
				      FLOATTYPE1 up,
				      CoordinateSystem cs = CS_default);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();
 
private:
  static TypeHandle _type_handle;
};

////////////////////////////////////////////////////////////////////
//     Function: LVector3::Default Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector3)::
FLOATNAME(LVector3)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector3)::
FLOATNAME(LVector3)(const FLOATNAME(LVecBase3) &copy) : FLOATNAME(LVecBase3)(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector3) &FLOATNAME(LVector3)::
operator = (const FLOATNAME(LVecBase3) &copy) {
  FLOATNAME(LVecBase3)::operator = (copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::Copy Fill Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector3) &FLOATNAME(LVector3)::
operator = (FLOATTYPE1 fill_value) {
  FLOATNAME(LVecBase3)::operator = (fill_value);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector3)::
FLOATNAME(LVector3)(FLOATTYPE1 fill_value) :
  FLOATNAME(LVecBase3)(fill_value) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector3)::
FLOATNAME(LVector3)(FLOATTYPE1 x, FLOATTYPE1 y, FLOATTYPE1 z) : 
  FLOATNAME(LVecBase3)(x, y, z) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::zero Named Constructor
//       Access: Public
//  Description: Returns a zero-length vector.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector3) FLOATNAME(LVector3)::
zero() {
  return FLOATNAME(LVector3)(0.0, 0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::unit_x Named Constructor
//       Access: Public
//  Description: Returns a unit X vector.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector3) FLOATNAME(LVector3)::
unit_x() {
  return FLOATNAME(LVector3)(1.0, 0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::unit_y Named Constructor
//       Access: Public
//  Description: Returns a unit Y vector.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector3) FLOATNAME(LVector3)::
unit_y() {
  return FLOATNAME(LVector3)(0.0, 1.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::unit_z Named Constructor
//       Access: Public
//  Description: Returns a unit Z vector.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector3) FLOATNAME(LVector3)::
unit_z() {
  return FLOATNAME(LVector3)(0.0, 0.0, 1.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::unary -
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector3) FLOATNAME(LVector3)::
operator - () const {
  return FLOATNAME(LVecBase3)::operator - ();
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::vector + vecbase
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase3) FLOATNAME(LVector3)::
operator + (const FLOATNAME(LVecBase3) &other) const {
  return FLOATNAME(LVecBase3)::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::vector + vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector3) FLOATNAME(LVector3)::
operator + (const FLOATNAME(LVector3) &other) const {
  return FLOATNAME(LVecBase3)::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::vector - vecbase
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase3) FLOATNAME(LVector3)::
operator - (const FLOATNAME(LVecBase3) &other) const {
  return FLOATNAME(LVecBase3)::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::vector - vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector3) FLOATNAME(LVector3)::
operator - (const FLOATNAME(LVector3) &other) const {
  return FLOATNAME(LVecBase3)::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::length
//       Access: Public
//  Description: Returns the length of the vector, by the Pythagorean
//               theorem.
////////////////////////////////////////////////////////////////////

INLINE FLOATTYPE1 FLOATNAME(LVector3)::
length() const {
  return csqrt((*this).dot(*this));
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::length_squared
//       Access: Public
//  Description: Returns the square of the vector's length, cheap and
//               easy.
////////////////////////////////////////////////////////////////////

INLINE FLOATTYPE1 FLOATNAME(LVector3)::
length_squared() const {
  return (*this).dot(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::normalize
//       Access: Public
//  Description: Normalizes the vector in place.  Returns true if the
//               vector was normalized, false if it was a zero-length
//               vector.
////////////////////////////////////////////////////////////////////

INLINE bool FLOATNAME(LVector3)::
normalize() {
  FLOATTYPE1 l2 = length_squared();
  if (l2 == (FLOATTYPE1)0.0) {
    set(0.0, 0.0, 0.0);
    return false;

  } else if (!IS_THRESHOLD_EQUAL(l2, 1.0, NEARLY_ZERO(FLOATTYPE1) * NEARLY_ZERO(FLOATTYPE1))) {
    (*this) /= csqrt(l2);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::cross
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector3) FLOATNAME(LVector3)::
cross(const FLOATNAME(LVecBase3) &other) const {
  return FLOATNAME(LVecBase3)::cross(other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::operator * scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector3) FLOATNAME(LVector3)::
operator * (FLOATTYPE1 scalar) const {
  return FLOATNAME(LVector3)(FLOATNAME(LVecBase3)::operator * (scalar));
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::operator / scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector3) FLOATNAME(LVector3)::
operator / (FLOATTYPE1 scalar) const {
  return FLOATNAME(LVector3)(FLOATNAME(LVecBase3)::operator / (scalar));
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::up
//       Access: Public, Static
//  Description: Returns the up vector for the given coordinate
//               system.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector3) FLOATNAME(LVector3)::
up(CoordinateSystem cs) {
  if (cs == CS_default) {
    cs = default_coordinate_system;
  }
  switch (cs) {
  case CS_zup_right:
  case CS_zup_left:
    return FLOATNAME(LVector3)(0.0, 0.0, 1.0);

  case CS_yup_right:
  case CS_yup_left:
    return FLOATNAME(LVector3)(0.0, 1.0, 0.0);

  default:
    linmath_cat.error()
      << "Invalid coordinate system!\n";
    return FLOATNAME(LVector3)(0.0, 0.0, 0.0);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::right
//       Access: Public, Static
//  Description: Returns the right vector for the given coordinate
//               system.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector3) FLOATNAME(LVector3)::
right(CoordinateSystem) {
  return FLOATNAME(LVector3)(1.0, 0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::forward
//       Access: Public, Static
//  Description: Returns the forward vector for the given coordinate
//               system.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector3) FLOATNAME(LVector3)::
forward(CoordinateSystem cs) {
  if (cs == CS_default) {
    cs = default_coordinate_system;
  }
  switch (cs) {
  case CS_zup_right:
    return FLOATNAME(LVector3)(0.0, 1.0, 0.0);

  case CS_zup_left:
    return FLOATNAME(LVector3)(0.0, -1.0, 0.0);

  case CS_yup_right:
    return FLOATNAME(LVector3)(0.0, 0.0, -1.0);

  case CS_yup_left:
    return FLOATNAME(LVector3)(0.0, 0.0, 1.0);

  default:
    linmath_cat.error()
      << "Invalid coordinate system!\n";
    return FLOATNAME(LVector3)(0.0, 0.0, 0.0);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::down
//       Access: Public, Static
//  Description: Returns the down vector for the given coordinate
//               system.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector3) FLOATNAME(LVector3)::
down(CoordinateSystem cs) {
  return -up(cs);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::left
//       Access: Public, Static
//  Description: Returns the left vector for the given coordinate
//               system.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector3) FLOATNAME(LVector3)::
left(CoordinateSystem cs) {
  return -right(cs);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::back
//       Access: Public, Static
//  Description: Returns the back vector for the given coordinate
//               system.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector3) FLOATNAME(LVector3)::
back(CoordinateSystem cs) {
  return -forward(cs);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::rfu
//       Access: Public, Static
//  Description: Returns a vector that is described by its right,
//               forward, and up components, in whatever way the
//               coordinate system represents that vector.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVector3) FLOATNAME(LVector3)::
rfu(FLOATTYPE1 right_v, FLOATTYPE1 fwd_v, FLOATTYPE1 up_v,
    CoordinateSystem cs) {
  return forward(cs) * fwd_v + up(cs) * up_v + right(cs) * right_v;
}

