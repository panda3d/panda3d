// Filename: lrotation.I
// Created by:  frang, charles (23Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "lrotation.h"
#include <notify.h>
#include <math.h>

template<class NumType>
TypeHandle LRotation<NumType>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LRotation::Default Constructor
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LRotation<NumType>::
LRotation() {
}

////////////////////////////////////////////////////////////////////
//     Function: LRotation::Copy Constructor
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LRotation<NumType>::
LRotation(const LQuaternionBase<NumType>& c) : 
  LQuaternionBase<NumType>(c) {
}

////////////////////////////////////////////////////////////////////
//     Function: LRotation::Constructor
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LRotation<NumType>::
LRotation(NumType r, NumType i, NumType j, NumType k) : 
  LQuaternionBase<NumType>(r, i, j, k) {
}

////////////////////////////////////////////////////////////////////
//     Function: LRotation::Constructor
//       Access: public
//  Description: lmatrix3
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LRotation<NumType>::
LRotation(const LMatrix3<NumType> &m) {
  set(m);
}

////////////////////////////////////////////////////////////////////
//     Function: LRotation::Constructor
//       Access: public
//  Description: lmatrix4
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LRotation<NumType>::
LRotation(const LMatrix4<NumType> &m) {
  set(m);
}

////////////////////////////////////////////////////////////////////
//     Function: LRotation::Constructor
//       Access: public
//  Description: axis + angle (in degrees)
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LRotation<NumType>::
LRotation(const LVector3<NumType> &axis, float angle) {
  float radians = angle * ((float) MathNumbers::pi / 180.0f);
  float theta_over_2 = radians / 2.0f;
  float sin_to2 = sinf(theta_over_2);

  set_r(cosf(theta_over_2));
  set_i(axis[0] * sin_to2);
  set_j(axis[1] * sin_to2);
  set_k(axis[2] * sin_to2);
}

////////////////////////////////////////////////////////////////////
//     Function: LRotation::Constructor
//       Access: public
//  Description: hpr (Real-time Rendering, p.49)
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LRotation<NumType>::
LRotation(float h, float p, float r) {
  LQuaternionBase<NumType> quat_h, quat_p, quat_r;

  quat_h.set(cosf(h), 0, sinf(h), 0);
  quat_p.set(cosf(p), sinf(p), 0, 0);
  quat_r.set(cosf(r), 0, 0, sinf(r));

  (*this) = quat_h * quat_p * quat_r;
}

////////////////////////////////////////////////////////////////////
//     Function: LRotation::Destructor
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
LRotation<NumType>::
~LRotation() {
}

////////////////////////////////////////////////////////////////////
//     Function: LRotation::operator *
//       Access: public
//  Description: Rotation * Rotation = Rotation
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LRotation<NumType> LRotation<NumType>::
operator*(const LRotation<NumType>& other) const {
  return multiply(other);
}

////////////////////////////////////////////////////////////////////
//     Function: LRotation::operator *
//       Access: public
//  Description: Rotation * Orientation = Orientation
//               This is another meaningless operation, attempting
//               to apply an orientation to a rotation.  It simply
//               returns the rhs.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LQuaternionBase<NumType> LRotation<NumType>::
operator*(const LQuaternionBase<NumType>& other) const {
  return other;
}

////////////////////////////////////////////////////////////////////
//     Function: LRotation::init_type
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
void LRotation<NumType>::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    // Format a string to describe the type.
    do_init_type(NumType);
    string name =
      "LRotation<" + get_type_handle(NumType).get_name() + ">";
    register_type(_type_handle, name);
  }
}
