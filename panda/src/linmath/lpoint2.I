// Filename: lpoint2.I
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

template<class NumType>
TypeHandle LPoint2<NumType>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::Default Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint2<NumType>::
LPoint2() {
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint2<NumType>::
LPoint2(const LVecBase2<NumType> &copy) : LVecBase2<NumType>(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint2<NumType> &LPoint2<NumType>::
operator = (const LVecBase2<NumType> &copy) {
  LVecBase2<NumType>::operator = (copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::Copy Fill Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint2<NumType> &LPoint2<NumType>::
operator = (NumType fill_value) {
  LVecBase2<NumType>::operator = (fill_value);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint2<NumType>::
LPoint2(NumType fill_value) :
  LVecBase2<NumType>(fill_value) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint2<NumType>::
LPoint2(NumType x, NumType y) : 
  LVecBase2<NumType>(x, y) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::zero Named Constructor
//       Access: Public
//  Description: Returns a zero point.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint2<NumType> LPoint2<NumType>::
zero() {
  return LPoint2<NumType>(0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::unit_x Named Constructor
//       Access: Public
//  Description: Returns a unit X point.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint2<NumType> LPoint2<NumType>::
unit_x() {
  return LPoint2<NumType>(1.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::unit_y Named Constructor
//       Access: Public
//  Description: Returns a unit Y point.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint2<NumType> LPoint2<NumType>::
unit_y() {
  return LPoint2<NumType>(0.0, 1.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::unary -
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint2<NumType> LPoint2<NumType>::
operator - () const {
  return LVecBase2<NumType>::operator - ();
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::point + vecbase
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase2<NumType> LPoint2<NumType>::
operator + (const LVecBase2<NumType> &other) const {
  return LVecBase2<NumType>::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::point + vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint2<NumType> LPoint2<NumType>::
operator + (const LVector2<NumType> &other) const {
  return LVecBase2<NumType>::operator + (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::point - vecbase
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase2<NumType> LPoint2<NumType>::
operator - (const LVecBase2<NumType> &other) const {
  return LVecBase2<NumType>::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::point - point
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVector2<NumType> LPoint2<NumType>::
operator - (const LPoint2<NumType> &other) const {
  return LVecBase2<NumType>::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::point - vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint2<NumType> LPoint2<NumType>::
operator - (const LVector2<NumType> &other) const {
  return LVecBase2<NumType>::operator - (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::operator * scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint2<NumType> LPoint2<NumType>::
operator * (NumType scalar) const {
  return LPoint2<NumType>(LVecBase2<NumType>::operator * (scalar));
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::operator / scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LPoint2<NumType> LPoint2<NumType>::
operator / (NumType scalar) const {
  return LPoint2<NumType>(LVecBase2<NumType>::operator / (scalar));
}

////////////////////////////////////////////////////////////////////
//     Function: LPoint2::init_type
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
void LPoint2<NumType>::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    LVecBase2<NumType>::init_type();
    string name =
      "LPoint2<" + get_type_handle(NumType).get_name() + ">";
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
INLINE LPoint2<NumType2> 
lcast_to(NumType2 *, const LPoint2<NumType> &source) {
  return LPoint2<NumType2>(source[0], source[1]);
}
