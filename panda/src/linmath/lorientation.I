// Filename: lorientation.I
// Created by:  frang, charles (23Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "lorientation.h"
#include <notify.h>

template<class NumType>
TypeHandle LOrientation<NumType>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LOrientation::Default Constructor
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LOrientation<NumType>::
LOrientation() {
}

////////////////////////////////////////////////////////////////////
//     Function: LOrientation::Copy Constructor
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LOrientation<NumType>::
LOrientation(const LQuaternionBase<NumType>& c) :
  LQuaternionBase<NumType>(c) {
}

////////////////////////////////////////////////////////////////////
//     Function: LOrientation::Constructor
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LOrientation<NumType>::
LOrientation(NumType r, NumType i, NumType j, NumType k) :
  LQuaternionBase<NumType>(r, i, j, k) {
}

////////////////////////////////////////////////////////////////////
//     Function: LOrientation::Constructor
//       Access: public
//  Description: vector + twist
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LOrientation<NumType>::
LOrientation(const LVector3<NumType> &point_at, float twist) {
  float radians = twist * ((float) MathNumbers::pi / 180.0f);
  float theta_over_2 = radians / 2.0f;
  float sin_to2 = sinf(theta_over_2);

  set_r(cosf(theta_over_2));
  set_i(point_at[0] * sin_to2);
  set_j(point_at[1] * sin_to2);
  set_k(point_at[2] * sin_to2);
}

////////////////////////////////////////////////////////////////////
//     Function: LOrientation::Constructor
//       Access: public
//  Description: matrix3
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LOrientation<NumType>::
LOrientation(const LMatrix3<NumType> &m) {
  set(m);
}

////////////////////////////////////////////////////////////////////
//     Function: LOrientation::Constructor
//       Access: public
//  Description: matrix4
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LOrientation<NumType>::
LOrientation(const LMatrix4<NumType> &m) {
  set(m);
}

////////////////////////////////////////////////////////////////////
//     Function: LOrientation::Destructor
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
LOrientation<NumType>::
~LOrientation() {
}

////////////////////////////////////////////////////////////////////
//     Function: LOrientation::operator *
//       Access: public
//  Description: Orientation * rotation = Orientation
//               Applies an rotation to an orientation.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LOrientation<NumType> LOrientation<NumType>::
operator *(const LQuaternionBase<NumType>& other) const {
  return multiply(other);
}

////////////////////////////////////////////////////////////////////
//     Function: LOrientation::operator *
//       Access: public
//  Description: Orientation * Orientation
//               This is a meaningless operation, and will always
//               simply return the rhs.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LOrientation<NumType> LOrientation<NumType>::
operator *(const LOrientation<NumType>& other) const {
  return other;
}

////////////////////////////////////////////////////////////////////
//     Function: LOrientation::init_type
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
void LOrientation<NumType>::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    // Format a string to describe the type.
    do_init_type(NumType);
    string name =
      "LOrientation<" + get_type_handle(NumType).get_name() + ">";
    register_type(_type_handle, name);
  }
}
