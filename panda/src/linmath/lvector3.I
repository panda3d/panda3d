// Filename: lvector3.I
// Created by:  drose (24Sep99)
// 
////////////////////////////////////////////////////////////////////

#include "config_linmath.h"
#include "cmath.h"

template<class NumType>
TypeHandle LVector3<NumType>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LVector3::Default Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector3<NumType>::
LVector3() {
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector3<NumType>::
LVector3(const LVecBase3<NumType> &copy) : LVecBase3<NumType>(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector3<NumType> &LVector3<NumType>::
operator = (const LVecBase3<NumType> &copy) {
  LVecBase3<NumType>::operator = (copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::Copy Fill Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector3<NumType> &LVector3<NumType>::
operator = (NumType fill_value) {
  LVecBase3<NumType>::operator = (fill_value);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector3<NumType>::
LVector3(NumType fill_value) :
  LVecBase3<NumType>(fill_value) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector3<NumType>::
LVector3(NumType x, NumType y, NumType z) : 
  LVecBase3<NumType>(x, y, z) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::zero Named Constructor
//       Access: Public
//  Description: Returns a zero-length vector.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector3<NumType> LVector3<NumType>::
zero() {
  return LVector3<NumType>(0.0, 0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::unit_x Named Constructor
//       Access: Public
//  Description: Returns a unit X vector.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector3<NumType> LVector3<NumType>::
unit_x() {
  return LVector3<NumType>(1.0, 0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::unit_y Named Constructor
//       Access: Public
//  Description: Returns a unit Y vector.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector3<NumType> LVector3<NumType>::
unit_y() {
  return LVector3<NumType>(0.0, 1.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::unit_z Named Constructor
//       Access: Public
//  Description: Returns a unit Z vector.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector3<NumType> LVector3<NumType>::
unit_z() {
  return LVector3<NumType>(0.0, 0.0, 1.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::unary -
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector3<NumType> LVector3<NumType>::
operator - () const {
  return LVecBase3<NumType>::operator - ();
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::vector + vecbase
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase3<NumType> LVector3<NumType>::
operator + (const LVecBase3<NumType> &other) const {
  return LVecBase3<NumType>::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::vector + vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector3<NumType> LVector3<NumType>::
operator + (const LVector3<NumType> &other) const {
  return LVecBase3<NumType>::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::vector - vecbase
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase3<NumType> LVector3<NumType>::
operator - (const LVecBase3<NumType> &other) const {
  return LVecBase3<NumType>::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::vector - vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector3<NumType> LVector3<NumType>::
operator - (const LVector3<NumType> &other) const {
  return LVecBase3<NumType>::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::length
//       Access: Public
//  Description: Returns the length of the vector, by the Pythagorean
//               theorem.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumType LVector3<NumType>::
length() const {
  return csqrt((*this).dot(*this));
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::length_squared
//       Access: Public
//  Description: Returns the square of the vector's length, cheap and
//               easy.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumType LVector3<NumType>::
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
template<class NumType>
INLINE bool LVector3<NumType>::
normalize() {
  NumType l2 = length_squared();
  if (l2 == 0.0) {
    set(0.0, 0.0, 0.0);
    return false;

  } else if (!IS_THRESHOLD_EQUAL(l2, 1.0, NEARLY_ZERO(NumType) * NEARLY_ZERO(NumType))) {
    (*this) /= csqrt(l2);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::cross
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector3<NumType> LVector3<NumType>::
cross(const LVecBase3<NumType> &other) const {
  return LVecBase3<NumType>::cross(other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::operator * scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector3<NumType> LVector3<NumType>::
operator * (NumType scalar) const {
  return LVector3<NumType>(LVecBase3<NumType>::operator * (scalar));
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::operator / scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector3<NumType> LVector3<NumType>::
operator / (NumType scalar) const {
  return LVector3<NumType>(LVecBase3<NumType>::operator / (scalar));
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::up
//       Access: Public, Static
//  Description: Returns the up vector for the given coordinate
//               system.
////////////////////////////////////////////////////////////////////
template<class NumType>
LVector3<NumType> LVector3<NumType>::
up(CoordinateSystem cs) {
  if (cs == CS_default) {
    cs = default_coordinate_system;
  }
  switch (cs) {
  case CS_zup_right:
  case CS_zup_left:
    return LVector3<NumType>(0.0, 0.0, 1.0);

  case CS_yup_right:
  case CS_yup_left:
    return LVector3<NumType>(0.0, 1.0, 0.0);

  default:
    linmath_cat.error()
      << "Invalid coordinate system!\n";
    return LVector3<NumType>(0.0, 0.0, 0.0);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::right
//       Access: Public, Static
//  Description: Returns the right vector for the given coordinate
//               system.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector3<NumType> LVector3<NumType>::
right(CoordinateSystem) {
  return LVector3<NumType>(1.0, 0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::forward
//       Access: Public, Static
//  Description: Returns the forward vector for the given coordinate
//               system.
////////////////////////////////////////////////////////////////////
template<class NumType>
LVector3<NumType> LVector3<NumType>::
forward(CoordinateSystem cs) {
  if (cs == CS_default) {
    cs = default_coordinate_system;
  }
  switch (cs) {
  case CS_zup_right:
    return LVector3<NumType>(0.0, 1.0, 0.0);

  case CS_zup_left:
    return LVector3<NumType>(0.0, -1.0, 0.0);

  case CS_yup_right:
    return LVector3<NumType>(0.0, 0.0, -1.0);

  case CS_yup_left:
    return LVector3<NumType>(0.0, 0.0, 1.0);

  default:
    linmath_cat.error()
      << "Invalid coordinate system!\n";
    return LVector3<NumType>(0.0, 0.0, 0.0);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::down
//       Access: Public, Static
//  Description: Returns the down vector for the given coordinate
//               system.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector3<NumType> LVector3<NumType>::
down(CoordinateSystem cs) {
  return -up(cs);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::left
//       Access: Public, Static
//  Description: Returns the left vector for the given coordinate
//               system.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector3<NumType> LVector3<NumType>::
left(CoordinateSystem cs) {
  return -right(cs);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::back
//       Access: Public, Static
//  Description: Returns the back vector for the given coordinate
//               system.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector3<NumType> LVector3<NumType>::
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
template<class NumType>
INLINE LVector3<NumType> LVector3<NumType>::
rfu(NumType right_v, NumType fwd_v, NumType up_v,
    CoordinateSystem cs) {
  return fwd_v * forward(cs) + up_v * up(cs) + right_v * right(cs);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector3::init_type
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
void LVector3<NumType>::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    LVecBase3<NumType>::init_type();
    string name =
      "LVector3<" + get_type_handle(NumType).get_name() + ">";
    register_type(_type_handle, name, 
		  LVecBase3<NumType>::get_class_type());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: lcast_to
//  Description: Converts a vector from one numeric representation to
//               another one.  This is usually invoked using the macro
//               LCAST.
////////////////////////////////////////////////////////////////////
template<class NumType, class NumType2>
INLINE LVector3<NumType2> 
lcast_to(NumType2 *, const LVector3<NumType> &source) {
  return LVector3<NumType2>(source[0], source[1], source[2]);
}
