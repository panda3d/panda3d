// Filename: lrotation.I
// Created by:  frang, charles (23Jun00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//       Class : LRotation
// Description : This is a unit quaternion representing a rotation.
////////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FLOATNAME(LRotation) : public FLOATNAME(LQuaternionBase) {
PUBLISHED:
  INLINE FLOATNAME(LRotation)();
  INLINE FLOATNAME(LRotation)(const FLOATNAME(LQuaternionBase)&);
  INLINE FLOATNAME(LRotation)(FLOATTYPE1, FLOATTYPE1, FLOATTYPE1, FLOATTYPE1);
  INLINE FLOATNAME(LRotation)(const FLOATNAME(LVector3) &, FLOATTYPE1);
  INLINE FLOATNAME(LRotation)(const FLOATNAME(LMatrix3) &);
  INLINE FLOATNAME(LRotation)(const FLOATNAME(LMatrix4) &);
  INLINE FLOATNAME(LRotation)(FLOATTYPE1, FLOATTYPE1, FLOATTYPE1);
  virtual ~FLOATNAME(LRotation)();

  INLINE FLOATNAME(LRotation)
  operator*(const FLOATNAME(LRotation)& other) const;

  INLINE FLOATNAME(LQuaternionBase)
  operator*(const FLOATNAME(LQuaternionBase)& other) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();
private:
  static TypeHandle _type_handle;
};

////////////////////////////////////////////////////////////////////
//     Function: LRotation::Default Constructor
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LRotation)::
FLOATNAME(LRotation)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LRotation::Copy Constructor
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LRotation)::
FLOATNAME(LRotation)(const FLOATNAME(LQuaternionBase)& c) : 
  FLOATNAME(LQuaternionBase)(c) {
}

////////////////////////////////////////////////////////////////////
//     Function: LRotation::Constructor
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LRotation)::
FLOATNAME(LRotation)(FLOATTYPE1 r, FLOATTYPE1 i, FLOATTYPE1 j, FLOATTYPE1 k) : 
  FLOATNAME(LQuaternionBase)(r, i, j, k) {
}

////////////////////////////////////////////////////////////////////
//     Function: LRotation::Constructor
//       Access: public
//  Description: lmatrix3
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LRotation)::
FLOATNAME(LRotation)(const FLOATNAME(LMatrix3) &m) {
  set(m);
}

////////////////////////////////////////////////////////////////////
//     Function: LRotation::Constructor
//       Access: public
//  Description: lmatrix4
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LRotation)::
FLOATNAME(LRotation)(const FLOATNAME(LMatrix4) &m) {
  set(m);
}

////////////////////////////////////////////////////////////////////
//     Function: LRotation::Constructor
//       Access: public
//  Description: axis + angle (in degrees)
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LRotation)::
FLOATNAME(LRotation)(const FLOATNAME(LVector3) &axis, FLOATTYPE1 angle) {
  FLOATTYPE1 radians = angle * ((FLOATTYPE1) MathNumbers::pi / (FLOATTYPE1)180.0);
  FLOATTYPE1 theta_over_2 = radians / (FLOATTYPE1)2.0;
  FLOATTYPE1 sin_to2 = csin(theta_over_2);

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

INLINE FLOATNAME(LRotation)::
FLOATNAME(LRotation)(FLOATTYPE1 h, FLOATTYPE1 p, FLOATTYPE1 r) {
  set_hpr(FLOATNAME(LVecBase3)(h, p, r));
}

////////////////////////////////////////////////////////////////////
//     Function: LRotation::operator *
//       Access: public
//  Description: Rotation * Rotation = Rotation
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LRotation) FLOATNAME(LRotation)::
operator*(const FLOATNAME(LRotation)& other) const {
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

INLINE FLOATNAME(LQuaternionBase) FLOATNAME(LRotation)::
operator*(const FLOATNAME(LQuaternionBase)& other) const {
  return other;
}


