// Filename: lpoint3.I
// Created by:  drose (25Sep99)
// 
////////////////////////////////////////////////////////////////////

#include "lvector3.h"

template<class NumType>
TypeHandle LPoint3<NumType>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::Default Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint3<NumType>::
LPoint3() {
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint3<NumType>::
LPoint3(const LVecBase3<NumType> &copy) : LVecBase3<NumType>(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint3<NumType> &LPoint3<NumType>::
operator = (const LVecBase3<NumType> &copy) {
  LVecBase3<NumType>::operator = (copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::Copy Fill Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint3<NumType> &LPoint3<NumType>::
operator = (NumType fill_value) {
  LVecBase3<NumType>::operator = (fill_value);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint3<NumType>::
LPoint3(NumType fill_value) :
  LVecBase3<NumType>(fill_value) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint3<NumType>::
LPoint3(NumType x, NumType y, NumType z) : 
  LVecBase3<NumType>(x, y, z) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::zero Named Constructor
//       Access: Public
//  Description: Returns a zero point.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint3<NumType> LPoint3<NumType>::
zero() {
  return LPoint3<NumType>(0.0, 0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::unit_x Named Constructor
//       Access: Public
//  Description: Returns a unit X point.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint3<NumType> LPoint3<NumType>::
unit_x() {
  return LPoint3<NumType>(1.0, 0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::unit_y Named Constructor
//       Access: Public
//  Description: Returns a unit Y point.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint3<NumType> LPoint3<NumType>::
unit_y() {
  return LPoint3<NumType>(0.0, 1.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::unit_z Named Constructor
//       Access: Public
//  Description: Returns a unit Z point.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint3<NumType> LPoint3<NumType>::
unit_z() {
  return LPoint3<NumType>(0.0, 0.0, 1.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::unary -
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint3<NumType> LPoint3<NumType>::
operator - () const {
  return LVecBase3<NumType>::operator - ();
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::point + vecbase
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase3<NumType> LPoint3<NumType>::
operator + (const LVecBase3<NumType> &other) const {
  return LVecBase3<NumType>::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::point + vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint3<NumType> LPoint3<NumType>::
operator + (const LVector3<NumType> &other) const {
  return LVecBase3<NumType>::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::point - vecbase
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase3<NumType> LPoint3<NumType>::
operator - (const LVecBase3<NumType> &other) const {
  return LVecBase3<NumType>::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::point - point
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector3<NumType> LPoint3<NumType>::
operator - (const LPoint3<NumType> &other) const {
  return LVecBase3<NumType>::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::point - vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint3<NumType> LPoint3<NumType>::
operator - (const LVector3<NumType> &other) const {
  return LVecBase3<NumType>::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::cross
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint3<NumType> LPoint3<NumType>::
cross(const LVecBase3<NumType> &other) const {
  return LVecBase3<NumType>::cross(other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::operator * scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint3<NumType> LPoint3<NumType>::
operator * (NumType scalar) const {
  return LPoint3<NumType>(LVecBase3<NumType>::operator * (scalar));
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::operator / scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint3<NumType> LPoint3<NumType>::
operator / (NumType scalar) const {
  return LPoint3<NumType>(LVecBase3<NumType>::operator / (scalar));
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::origin
//       Access: Public, Static
//  Description: Returns the origin of the indicated coordinate
//               system.  This is always 0, 0, 0 with all of our
//               existing coordinate systems; it's hard to imagine it
//               ever being different.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint3<NumType> LPoint3<NumType>::
origin(CoordinateSystem) {
  return LPoint3<NumType>(0.0, 0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::rfu
//       Access: Public, Static
//  Description: Returns a point described by right, forward, up
//               displacements from the origin, wherever that maps to
//               in the given coordinate system.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint3<NumType> LPoint3<NumType>::
rfu(NumType right_v, NumType fwd_v, NumType up_v,
    CoordinateSystem cs) {
  return origin(cs) + 
    LVector3<NumType>::rfu(right_v, fwd_v, up_v, cs);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::init_type
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
void LPoint3<NumType>::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    LVecBase3<NumType>::init_type();
    string name =
      "LPoint3<" + get_type_handle(NumType).get_name() + ">";
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
INLINE LPoint3<NumType2> 
lcast_to(NumType2 *, const LPoint3<NumType> &source) {
  return LPoint3<NumType2>(source[0], source[1], source[2]);
}
