// Filename: lvector2.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LVECTOR2_H
#define LVECTOR2_H

#include <pandabase.h>

#include "lvecBase2.h"

////////////////////////////////////////////////////////////////////
// 	 Class : LVector2
// Description : This is a two-component vector offset.
////////////////////////////////////////////////////////////////////
template<class NumType>
class LVector2 : public LVecBase2<NumType> {
PUBLISHED:
  INLINE LVector2();
  INLINE LVector2(const LVecBase2<NumType> &copy);
  INLINE LVector2<NumType> &operator = (const LVecBase2<NumType> &copy);
  INLINE LVector2<NumType> &operator = (NumType fill_value);
  INLINE LVector2(NumType fill_value);
  INLINE LVector2(NumType x, NumType y);

  INLINE static LVector2<NumType> zero();
  INLINE static LVector2<NumType> unit_x();
  INLINE static LVector2<NumType> unit_y();

  INLINE LVector2<NumType> operator - () const;

  INLINE LVecBase2<NumType>
  operator + (const LVecBase2<NumType> &other) const;
  INLINE LVector2<NumType>
  operator + (const LVector2<NumType> &other) const;

  INLINE LVecBase2<NumType>
  operator - (const LVecBase2<NumType> &other) const;
  INLINE LVector2<NumType>
  operator - (const LVector2<NumType> &other) const;

  INLINE NumType length() const;
  INLINE NumType length_squared() const;
  INLINE bool normalize();
  INLINE LVector2<NumType> operator * (NumType scalar) const;
  INLINE LVector2<NumType> operator / (NumType scalar) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();
 
private:
  static TypeHandle _type_handle;
};

// Cast to a different numeric type
template<class NumType, class NumType2>
INLINE LVector2<NumType2> 
lcast_to(NumType2 *type, const LVector2<NumType> &source);

#include "lvector2.I"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, LVector2<float>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, LVector2<double>)

#endif
