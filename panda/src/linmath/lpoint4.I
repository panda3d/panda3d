// Filename: lpoint4.I
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

template<class NumType>
TypeHandle LPoint4<NumType>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::Default Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint4<NumType>::
LPoint4() {
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint4<NumType>::
LPoint4(const LVecBase4<NumType> &copy) : LVecBase4<NumType>(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint4<NumType> &LPoint4<NumType>::
operator = (const LVecBase4<NumType> &copy) {
  LVecBase4<NumType>::operator = (copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::Copy Fill Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint4<NumType> &LPoint4<NumType>::
operator = (NumType fill_value) {
  LVecBase4<NumType>::operator = (fill_value);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint4<NumType>::
LPoint4(NumType fill_value) :
  LVecBase4<NumType>(fill_value) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint4<NumType>::
LPoint4(NumType x, NumType y, NumType z, NumType w) : 
  LVecBase4<NumType>(x, y, z, w) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::zero Named Constructor
//       Access: Public
//  Description: Returns a zero point.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint4<NumType> LPoint4<NumType>::
zero() {
  return LPoint4<NumType>(0.0, 0.0, 0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::unit_x Named Constructor
//       Access: Public
//  Description: Returns a unit X point.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint4<NumType> LPoint4<NumType>::
unit_x() {
  return LPoint4<NumType>(1.0, 0.0, 0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::unit_y Named Constructor
//       Access: Public
//  Description: Returns a unit Y point.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint4<NumType> LPoint4<NumType>::
unit_y() {
  return LPoint4<NumType>(0.0, 1.0, 0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::unit_z Named Constructor
//       Access: Public
//  Description: Returns a unit Z point.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint4<NumType> LPoint4<NumType>::
unit_z() {
  return LPoint4<NumType>(0.0, 0.0, 1.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::unit_w Named Constructor
//       Access: Public
//  Description: Returns a unit W point.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint4<NumType> LPoint4<NumType>::
unit_w() {
  return LPoint4<NumType>(0.0, 0.0, 0.0, 1.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::unary -
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint4<NumType> LPoint4<NumType>::
operator - () const {
  return LVecBase4<NumType>::operator - ();
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::point + vecbase
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase4<NumType> LPoint4<NumType>::
operator + (const LVecBase4<NumType> &other) const {
  return LVecBase4<NumType>::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::point + vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint4<NumType> LPoint4<NumType>::
operator + (const LVector4<NumType> &other) const {
  return LVecBase4<NumType>::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::point - vecbase
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase4<NumType> LPoint4<NumType>::
operator - (const LVecBase4<NumType> &other) const {
  return LVecBase4<NumType>::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::point - point
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector4<NumType> LPoint4<NumType>::
operator - (const LPoint4<NumType> &other) const {
  return LVecBase4<NumType>::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::point - vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint4<NumType> LPoint4<NumType>::
operator - (const LVector4<NumType> &other) const {
  return LVecBase4<NumType>::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::operator * scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint4<NumType> LPoint4<NumType>::
operator * (NumType scalar) const {
  return LPoint4<NumType>(LVecBase4<NumType>::operator * (scalar));
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::operator / scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint4<NumType> LPoint4<NumType>::
operator / (NumType scalar) const {
  return LPoint4<NumType>(LVecBase4<NumType>::operator / (scalar));
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint4::init_type
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
void LPoint4<NumType>::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    LVecBase4<NumType>::init_type();
    string name =
      "LPoint4<" + get_type_handle(NumType).get_name() + ">";
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
INLINE LPoint4<NumType2> 
lcast_to(NumType2 *, const LPoint4<NumType> &source) {
  return LPoint4<NumType2>(source[0], source[1], source[2], source[3]);
}
