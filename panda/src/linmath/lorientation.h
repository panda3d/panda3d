// Filename: lorientation.h
// Created by:  frang, charles (23Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __LORIENTATION_H__
#define __LORIENTATION_H__

#include <pandabase.h>
#include "lquaternion.h"

////////////////////////////////////////////////////////////////////////
//       Class : LOrientation
// Description : This is a unit quaternion representing an orientation.
////////////////////////////////////////////////////////////////////////
template <class NumType>
class LOrientation : public LQuaternionBase<NumType> {
public:
  INLINE LOrientation();
  INLINE LOrientation(const LQuaternionBase<NumType>&);
  INLINE LOrientation(NumType, NumType, NumType, NumType);
  INLINE LOrientation(const LVector3<NumType> &, float);
  INLINE LOrientation(const LMatrix3<NumType> &);
  INLINE LOrientation(const LMatrix4<NumType> &);
  virtual ~LOrientation();

  INLINE LOrientation<NumType> 
  operator *(const LQuaternionBase<NumType>& other) const;

  INLINE LOrientation<NumType>
  operator *(const LOrientation<NumType>& other) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();
private:
  static TypeHandle _type_handle;
};

#include "lorientation.I"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, LOrientation<float>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, LOrientation<double>)

#endif /* __LORIENTATION_H__ */
