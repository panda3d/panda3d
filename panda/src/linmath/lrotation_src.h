// Filename: lrotation_src.h
// Created by:  frang, charles (23Jun00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//       Class : LRotation
// Description : This is a unit quaternion representing a rotation.
////////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FLOATNAME(LRotation) : public FLOATNAME(LQuaternion) {
PUBLISHED:
  INLINE FLOATNAME(LRotation)();
  INLINE FLOATNAME(LRotation)(const FLOATNAME(LQuaternion)&);
  INLINE FLOATNAME(LRotation)(FLOATTYPE, FLOATTYPE, FLOATTYPE, FLOATTYPE);
  INLINE FLOATNAME(LRotation)(const FLOATNAME(LVector3) &, FLOATTYPE);
  INLINE FLOATNAME(LRotation)(const FLOATNAME(LMatrix3) &);
  INLINE FLOATNAME(LRotation)(const FLOATNAME(LMatrix4) &);
  INLINE FLOATNAME(LRotation)(FLOATTYPE, FLOATTYPE, FLOATTYPE);

  INLINE FLOATNAME(LRotation)
  operator*(const FLOATNAME(LRotation)& other) const;

  INLINE FLOATNAME(LQuaternion)
  operator*(const FLOATNAME(LQuaternion)& other) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();
private:
  static TypeHandle _type_handle;
};

#include "lrotation_src.I"
