// Filename: lrotation.I
// Created by:  frang, charles (23Jun00)
// 
////////////////////////////////////////////////////////////////////

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
LRotation(const LVector3<NumType> &axis, NumType angle) {
  NumType radians = angle * ((NumType) MathNumbers::pi / (NumType)180.0);
  NumType theta_over_2 = radians / (NumType)2.0;
  NumType sin_to2 = csin(theta_over_2);

  set_r(ccos(theta_over_2));
  set_i(axis[0] * sin_to2);
  set_j(axis[1] * sin_to2);
  set_k(axis[2] * sin_to2);
}

////////////////////////////////////////////////////////////////////
//     Function: LRotation::Constructor
//       Access: public
//  Description: Sets the rotation from the given Euler angles.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LRotation<NumType>::
LRotation(NumType h, NumType p, NumType r) {
  set_hpr(LVecBase3<NumType>(h, p, r));
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
