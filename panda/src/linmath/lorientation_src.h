// Filename: lorientation_src.h
// Created by:  frang, charles (23Jun00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//       Class : LOrientation
// Description : This is a unit quaternion representing an orientation.
////////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FLOATNAME(LOrientation) : public FLOATNAME(LQuaternion) {
public:
  INLINE_LINMATH FLOATNAME(LOrientation)();
  INLINE_LINMATH FLOATNAME(LOrientation)(const FLOATNAME(LQuaternion)&);
  INLINE_LINMATH FLOATNAME(LOrientation)(FLOATTYPE, FLOATTYPE, FLOATTYPE, FLOATTYPE);
  INLINE_LINMATH FLOATNAME(LOrientation)(const FLOATNAME(LVector3) &, float);
  INLINE_LINMATH FLOATNAME(LOrientation)(const FLOATNAME(LMatrix3) &);
  INLINE_LINMATH FLOATNAME(LOrientation)(const FLOATNAME(LMatrix4) &);

  INLINE_LINMATH FLOATNAME(LOrientation) 
  operator *(const FLOATNAME(LQuaternion)& other) const;

  INLINE_LINMATH FLOATNAME(LOrientation)
  operator *(const FLOATNAME(LOrientation)& other) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();
private:
  static TypeHandle _type_handle;
};

#include "lorientation_src.I"
