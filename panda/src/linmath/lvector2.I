// Filename: lvector2.I
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "cmath.h"

template<class NumType>
TypeHandle LVector2<NumType>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LVector2::Default Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector2<NumType>::
LVector2() {
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector2<NumType>::
LVector2(const LVecBase2<NumType> &copy) : LVecBase2<NumType>(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector2<NumType> &LVector2<NumType>::
operator = (const LVecBase2<NumType> &copy) {
  LVecBase2<NumType>::operator = (copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::Copy Fill Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector2<NumType> &LVector2<NumType>::
operator = (NumType fill_value) {
  LVecBase2<NumType>::operator = (fill_value);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector2<NumType>::
LVector2(NumType fill_value) :
  LVecBase2<NumType>(fill_value) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector2<NumType>::
LVector2(NumType x, NumType y) : 
  LVecBase2<NumType>(x, y) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::zero Named Constructor
//       Access: Public
//  Description: Returns a zero-length vector.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector2<NumType> LVector2<NumType>::
zero() {
  return LVector2<NumType>(0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::unit_x Named Constructor
//       Access: Public
//  Description: Returns a unit X vector.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector2<NumType> LVector2<NumType>::
unit_x() {
  return LVector2<NumType>(1.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::unit_y Named Constructor
//       Access: Public
//  Description: Returns a unit Y vector.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector2<NumType> LVector2<NumType>::
unit_y() {
  return LVector2<NumType>(0.0, 1.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::unary -
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector2<NumType> LVector2<NumType>::
operator - () const {
  return LVecBase2<NumType>::operator - ();
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::vector + vecbase
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase2<NumType> LVector2<NumType>::
operator + (const LVecBase2<NumType> &other) const {
  return LVecBase2<NumType>::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::vector + vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector2<NumType> LVector2<NumType>::
operator + (const LVector2<NumType> &other) const {
  return LVecBase2<NumType>::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::vector - vecbase
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase2<NumType> LVector2<NumType>::
operator - (const LVecBase2<NumType> &other) const {
  return LVecBase2<NumType>::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::vector - vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector2<NumType> LVector2<NumType>::
operator - (const LVector2<NumType> &other) const {
  return LVecBase2<NumType>::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::length
//       Access: Public
//  Description: Returns the length of the vector, by the Pythagorean
//               theorem.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumType LVector2<NumType>::
length() const {
  return csqrt((*this).dot(*this));
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::length_squared
//       Access: Public
//  Description: Returns the square of the vector's length, cheap and
//               easy.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumType LVector2<NumType>::
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
template<class NumType>
INLINE bool LVector2<NumType>::
normalize() {
  NumType l2 = length_squared();
  if (l2 == (NumType)0.0) {
    set(0.0, 0.0);
    return false;

  } else if (!IS_THRESHOLD_EQUAL(l2, 1.0, NEARLY_ZERO(NumType) * NEARLY_ZERO(NumType))) {
    (*this) /= csqrt(l2);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::operator * scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector2<NumType> LVector2<NumType>::
operator * (NumType scalar) const {
  return LVector2<NumType>(LVecBase2<NumType>::operator * (scalar));
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::operator / scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector2<NumType> LVector2<NumType>::
operator / (NumType scalar) const {
  return LVector2<NumType>(LVecBase2<NumType>::operator / (scalar));
}

////////////////////////////////////////////////////////////////////
//     Function: LVector2::init_type
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
void LVector2<NumType>::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    LVecBase2<NumType>::init_type();
    string name =
      "LVector2<" + get_type_handle(NumType).get_name() + ">";
    register_type(_type_handle, name, 
		  LVecBase2<NumType>::get_class_type());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: lcast_to
//  Description: Converts a vector from one numeric representation to
//               another one.  This is usually invoked using the macro
//               LCAST.
////////////////////////////////////////////////////////////////////
template<class NumType, class NumType2>
INLINE LVector2<NumType2> 
lcast_to(NumType2 *, const LVector2<NumType> &source) {
  return LVector2<NumType2>(source[0], source[1]);
}
