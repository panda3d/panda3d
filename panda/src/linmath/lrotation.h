// Filename: lrotation.h
// Created by:  frang, charles (07Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __LROTATION_H__
#define __LROTATION_H__

#include <pandabase.h>
#include "lquaternion.h"

////////////////////////////////////////////////////////////////////////
//       Class : LRotation
// Description : This is a unit quaternion representing a rotation.
////////////////////////////////////////////////////////////////////////
template <class NumType>
class LRotation : public LQuaternionBase<NumType> {
public:
  INLINE LRotation();
  INLINE LRotation(const LQuaternionBase<NumType>&);
  INLINE LRotation(NumType, NumType, NumType, NumType);
  INLINE LRotation(const LVector3<NumType> &, float);
  INLINE LRotation(const LMatrix3<NumType> &);
  INLINE LRotation(const LMatrix4<NumType> &);
  INLINE LRotation(float, float, float);
  virtual ~LRotation();

  INLINE LRotation<NumType>
  operator*(const LRotation<NumType>& other) const;

  INLINE LQuaternionBase<NumType>
  operator*(const LQuaternionBase<NumType>& other) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();
private:
  static TypeHandle _type_handle;
};

#include "lrotation.I"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, LRotation<float>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, LRotation<double>)

#endif /* __LROTATION_H__ */
