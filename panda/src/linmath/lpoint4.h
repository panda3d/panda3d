// Filename: lpoint4.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LPOINT4_H
#define LPOINT4_H

#include <pandabase.h>

#include "lvecBase4.h"
#include "lvector4.h"

////////////////////////////////////////////////////////////////////
// 	 Class : LPoint4
// Description : This is a four-component point in space.
////////////////////////////////////////////////////////////////////
template<class NumType>
class LPoint4 : public LVecBase4<NumType> {
PUBLISHED:
  INLINE LPoint4();
  INLINE LPoint4(const LVecBase4<NumType> &copy);
  INLINE LPoint4<NumType> &operator = (const LVecBase4<NumType> &copy);
  INLINE LPoint4<NumType> &operator = (NumType fill_value);
  INLINE LPoint4(NumType fill_value);
  INLINE LPoint4(NumType x, NumType y, NumType z, NumType w);

  INLINE static LPoint4<NumType> zero();
  INLINE static LPoint4<NumType> unit_x();
  INLINE static LPoint4<NumType> unit_y();
  INLINE static LPoint4<NumType> unit_z();
  INLINE static LPoint4<NumType> unit_w();

  INLINE LPoint4<NumType> operator - () const;

  INLINE LVecBase4<NumType>
  operator + (const LVecBase4<NumType> &other) const;
  INLINE LPoint4<NumType>
  operator + (const LVector4<NumType> &other) const;

  INLINE LVecBase4<NumType>
  operator - (const LVecBase4<NumType> &other) const;
  INLINE LVector4<NumType>
  operator - (const LPoint4<NumType> &other) const;
  INLINE LPoint4<NumType>
  operator - (const LVector4<NumType> &other) const;

  INLINE LPoint4<NumType> operator * (NumType scalar) const;
  INLINE LPoint4<NumType> operator / (NumType scalar) const;

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
INLINE LPoint4<NumType2> 
lcast_to(NumType2 *type, const LPoint4<NumType> &source);

#include "lpoint4.I"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, LPoint4<float>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, LPoint4<double>)

#endif
