// Filename: lvector4.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LVECTOR4_H
#define LVECTOR4_H

#include <pandabase.h>

#include "lvecBase4.h"

////////////////////////////////////////////////////////////////////
// 	 Class : LVector4
// Description : This is a four-component vector distance.
////////////////////////////////////////////////////////////////////
template<class NumType>
class LVector4 : public LVecBase4<NumType> {
PUBLISHED:
  INLINE LVector4();
  INLINE LVector4(const LVecBase4<NumType> &copy);
  INLINE LVector4<NumType> &operator = (const LVecBase4<NumType> &copy);
  INLINE LVector4<NumType> &operator = (NumType fill_value);
  INLINE LVector4(NumType fill_value);
  INLINE LVector4(NumType x, NumType y, NumType z, NumType w);

  INLINE static LVector4<NumType> zero();
  INLINE static LVector4<NumType> unit_x();
  INLINE static LVector4<NumType> unit_y();
  INLINE static LVector4<NumType> unit_z();
  INLINE static LVector4<NumType> unit_w();

  INLINE LVector4<NumType> operator - () const;

  INLINE LVecBase4<NumType>
  operator + (const LVecBase4<NumType> &other) const;
  INLINE LVector4<NumType>
  operator + (const LVector4<NumType> &other) const;

  INLINE LVecBase4<NumType>
  operator - (const LVecBase4<NumType> &other) const;
  INLINE LVector4<NumType>
  operator - (const LVector4<NumType> &other) const;

  INLINE NumType length() const;
  INLINE NumType length_squared() const;
  INLINE bool normalize();
  INLINE LVector4<NumType> operator * (NumType scalar) const;
  INLINE LVector4<NumType> operator / (NumType scalar) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();
 
private:
  static TypeHandle _type_handle;
};

template<class NumType, class NumType2>
INLINE LVector4<NumType2> 
lcast_to(NumType2 *type, const LVector4<NumType> &source);

#include "lvector4.I"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, LVector4<float>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, LVector4<double>)

#endif
