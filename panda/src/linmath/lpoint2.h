// Filename: lpoint2.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LPOINT2_H
#define LPOINT2_H

#include <pandabase.h>

#include "lvecBase2.h"
#include "lvector2.h"

////////////////////////////////////////////////////////////////////
// 	 Class : LPoint2
// Description : This is a two-component point in space.
////////////////////////////////////////////////////////////////////
template<class NumType>
class LPoint2 : public LVecBase2<NumType> {
PUBLISHED:
  INLINE LPoint2();
  INLINE LPoint2(const LVecBase2<NumType> &copy);
  INLINE LPoint2<NumType> &operator = (const LVecBase2<NumType> &copy);
  INLINE LPoint2<NumType> &operator = (NumType fill_value);
  INLINE LPoint2(NumType fill_value);
  INLINE LPoint2(NumType x, NumType y);

  INLINE static LPoint2<NumType> zero();
  INLINE static LPoint2<NumType> unit_x();
  INLINE static LPoint2<NumType> unit_y();

  INLINE LPoint2<NumType> operator - () const;

  INLINE LVecBase2<NumType>
  operator + (const LVecBase2<NumType> &other) const;
  INLINE LPoint2<NumType>
  operator + (const LVector2<NumType> &other) const;

  INLINE LVecBase2<NumType>
  operator - (const LVecBase2<NumType> &other) const;
  INLINE LVector2<NumType>
  operator - (const LPoint2<NumType> &other) const;
  INLINE LPoint2<NumType>
  operator - (const LVector2<NumType> &other) const;

  INLINE LPoint2<NumType> operator * (NumType scalar) const;
  INLINE LPoint2<NumType> operator / (NumType scalar) const;

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
INLINE LPoint2<NumType2> 
lcast_to(NumType2 *type, const LPoint2<NumType> &source);

#include "lpoint2.I"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, LPoint2<float>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, LPoint2<double>)

#endif
