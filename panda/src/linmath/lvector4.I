// Filename: lvector4.I
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "cmath.h"


template<class NumType>
TypeHandle LVector4<NumType>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LVector4::Default Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector4<NumType>::
LVector4() {
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector4<NumType>::
LVector4(const LVecBase4<NumType> &copy) : LVecBase4<NumType>(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector4<NumType> &LVector4<NumType>::
operator = (const LVecBase4<NumType> &copy) {
  LVecBase4<NumType>::operator = (copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::Copy Fill Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector4<NumType> &LVector4<NumType>::
operator = (NumType fill_value) {
  LVecBase4<NumType>::operator = (fill_value);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector4<NumType>::
LVector4(NumType fill_value) :
  LVecBase4<NumType>(fill_value) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector4<NumType>::
LVector4(NumType x, NumType y, NumType z, NumType w) : 
  LVecBase4<NumType>(x, y, z, w) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::zero Named Constructor
//       Access: Public
//  Description: Returns a zero-length vector.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector4<NumType> LVector4<NumType>::
zero() {
  return LVector4<NumType>(0.0, 0.0, 0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::unit_x Named Constructor
//       Access: Public
//  Description: Returns a unit X vector.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector4<NumType> LVector4<NumType>::
unit_x() {
  return LVector4<NumType>(1.0, 0.0, 0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::unit_y Named Constructor
//       Access: Public
//  Description: Returns a unit Y vector.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector4<NumType> LVector4<NumType>::
unit_y() {
  return LVector4<NumType>(0.0, 1.0, 0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::unit_z Named Constructor
//       Access: Public
//  Description: Returns a unit Z vector.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector4<NumType> LVector4<NumType>::
unit_z() {
  return LVector4<NumType>(0.0, 0.0, 1.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::unit_w Named Constructor
//       Access: Public
//  Description: Returns a unit W vector.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector4<NumType> LVector4<NumType>::
unit_w() {
  return LVector4<NumType>(0.0, 0.0, 0.0, 1.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::unary -
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector4<NumType> LVector4<NumType>::
operator - () const {
  return LVecBase4<NumType>::operator - ();
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::vector + vecbase
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase4<NumType> LVector4<NumType>::
operator + (const LVecBase4<NumType> &other) const {
  return LVecBase4<NumType>::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::vector + vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector4<NumType> LVector4<NumType>::
operator + (const LVector4<NumType> &other) const {
  return LVecBase4<NumType>::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::vector - vecbase
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase4<NumType> LVector4<NumType>::
operator - (const LVecBase4<NumType> &other) const {
  return LVecBase4<NumType>::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::vector - vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector4<NumType> LVector4<NumType>::
operator - (const LVector4<NumType> &other) const {
  return LVecBase4<NumType>::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::length
//       Access: Public
//  Description: Returns the length of the vector, by the Pythagorean
//               theorem.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumType LVector4<NumType>::
length() const {
  return csqrt((*this).dot(*this));
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::length_squared
//       Access: Public
//  Description: Returns the square of the vector's length, cheap and
//               easy.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumType LVector4<NumType>::
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
template<class NumType>
INLINE bool LVector4<NumType>::
normalize() {
  NumType l2 = length_squared();
  if (l2 == (NumType)0.0) {
    set(0.0, 0.0, 0.0, 0.0);
    return false;

  } else if (!IS_THRESHOLD_EQUAL(l2, 1.0, NEARLY_ZERO(NumType) * NEARLY_ZERO(NumType))) {
    (*this) /= csqrt(l2);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::operator * scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector4<NumType> LVector4<NumType>::
operator * (NumType scalar) const {
  return LVector4<NumType>(LVecBase4<NumType>::operator * (scalar));
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::operator / scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector4<NumType> LVector4<NumType>::
operator / (NumType scalar) const {
  return LVector4<NumType>(LVecBase4<NumType>::operator / (scalar));
}

////////////////////////////////////////////////////////////////////
//     Function: LVector4::init_type
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
void LVector4<NumType>::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    LVecBase4<NumType>::init_type();
    string name =
      "LVector4<" + get_type_handle(NumType).get_name() + ">";
    register_type(_type_handle, name, 
		  LVecBase4<NumType>::get_class_type());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: lcast_to
//  Description: Converts a vector from one numeric representation to
//               another one.  This is usually invoked using the macro
//               LCAST.
////////////////////////////////////////////////////////////////////
template<class NumType, class NumType2>
INLINE LVector4<NumType2> 
lcast_to(NumType2 *, const LVector4<NumType> &source) {
  return LVector4<NumType2>(source[0], source[1], source[2], source[3]);
}
