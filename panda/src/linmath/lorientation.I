// Filename: lorientation.I
// Created by:  frang, charles (23Jun00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//       Class : LOrientation
// Description : This is a unit quaternion representing an orientation.
////////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FLOATNAME(LOrientation) : public FLOATNAME(LQuaternionBase) {
public:
  INLINE FLOATNAME(LOrientation)();
  INLINE FLOATNAME(LOrientation)(const FLOATNAME(LQuaternionBase)&);
  INLINE FLOATNAME(LOrientation)(FLOATTYPE1, FLOATTYPE1, FLOATTYPE1, FLOATTYPE1);
  INLINE FLOATNAME(LOrientation)(const FLOATNAME(LVector3) &, float);
  INLINE FLOATNAME(LOrientation)(const FLOATNAME(LMatrix3) &);
  INLINE FLOATNAME(LOrientation)(const FLOATNAME(LMatrix4) &);
  virtual ~FLOATNAME(LOrientation)();

  INLINE FLOATNAME(LOrientation) 
  operator *(const FLOATNAME(LQuaternionBase)& other) const;

  INLINE FLOATNAME(LOrientation)
  operator *(const FLOATNAME(LOrientation)& other) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();
private:
  static TypeHandle _type_handle;
};

////////////////////////////////////////////////////////////////////
//     Function: LOrientation::Default Constructor
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LOrientation)::
FLOATNAME(LOrientation)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LOrientation::Copy Constructor
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LOrientation)::
FLOATNAME(LOrientation)(const FLOATNAME(LQuaternionBase)& c) :
  FLOATNAME(LQuaternionBase)(c) {
}

////////////////////////////////////////////////////////////////////
//     Function: LOrientation::Constructor
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LOrientation)::
FLOATNAME(LOrientation)(FLOATTYPE1 r, FLOATTYPE1 i, FLOATTYPE1 j, FLOATTYPE1 k) :
  FLOATNAME(LQuaternionBase)(r, i, j, k) {
}

////////////////////////////////////////////////////////////////////
//     Function: LOrientation::Constructor
//       Access: public
//  Description: vector + twist
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LOrientation)::
FLOATNAME(LOrientation)(const FLOATNAME(LVector3) &point_at, float twist) {
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

INLINE FLOATNAME(LOrientation)::
FLOATNAME(LOrientation)(const FLOATNAME(LMatrix3) &m) {
  set(m);
}

////////////////////////////////////////////////////////////////////
//     Function: LOrientation::Constructor
//       Access: public
//  Description: matrix4
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LOrientation)::
FLOATNAME(LOrientation)(const FLOATNAME(LMatrix4) &m) {
  set(m);
}

////////////////////////////////////////////////////////////////////
//     Function: LOrientation::operator *
//       Access: public
//  Description: Orientation * rotation = Orientation
//               Applies an rotation to an orientation.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LOrientation) FLOATNAME(LOrientation)::
operator *(const FLOATNAME(LQuaternionBase)& other) const {
  return multiply(other);
}

////////////////////////////////////////////////////////////////////
//     Function: LOrientation::operator *
//       Access: public
//  Description: Orientation * Orientation
//               This is a meaningless operation, and will always
//               simply return the rhs.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LOrientation) FLOATNAME(LOrientation)::
operator *(const FLOATNAME(LOrientation)& other) const {
  return other;
}
