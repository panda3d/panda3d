// Filename: lpoint3.h
// Created by:  drose (25Sep99)
//
////////////////////////////////////////////////////////////////////

#ifndef LPOINT3_H
#define LPOINT3_H

#include <pandabase.h>

#include "coordinateSystem.h"
#include "lvecBase3.h"
#include "lvector3.h"

////////////////////////////////////////////////////////////////////
// 	 Class : LPoint3
// Description : This is a three-component point in space (as opposed
//               to a three-component vector, which represents a
//               direction and a distance).  Some of the methods are
//               slightly different between LPoint3 and LVector3; in
//               particular, subtraction of two points yields a
//               vector, while addition of a vector and a point yields
//               a point.
////////////////////////////////////////////////////////////////////
template<class NumType>
class LPoint3 : public LVecBase3<NumType> {
PUBLISHED:
  INLINE LPoint3();
  INLINE LPoint3(const LVecBase3<NumType> &copy);
  INLINE LPoint3<NumType> &operator = (const LVecBase3<NumType> &copy);
  INLINE LPoint3<NumType> &operator = (NumType fill_value);
  INLINE LPoint3(NumType fill_value);
  INLINE LPoint3(NumType x, NumType y, NumType z);

  INLINE static LPoint3<NumType> zero();
  INLINE static LPoint3<NumType> unit_x();
  INLINE static LPoint3<NumType> unit_y();
  INLINE static LPoint3<NumType> unit_z();

  INLINE LPoint3<NumType> operator - () const;

  INLINE LVecBase3<NumType>
  operator + (const LVecBase3<NumType> &other) const;
  INLINE LPoint3<NumType>
  operator + (const LVector3<NumType> &other) const;

  INLINE LVecBase3<NumType>
  operator - (const LVecBase3<NumType> &other) const;
  INLINE LVector3<NumType>
  operator - (const LPoint3<NumType> &other) const;
  INLINE LPoint3<NumType>
  operator - (const LVector3<NumType> &other) const;

  INLINE LPoint3<NumType> cross(const LVecBase3<NumType> &other) const;
  INLINE LPoint3<NumType> operator * (NumType scalar) const;
  INLINE LPoint3<NumType> operator / (NumType scalar) const;

  // Some special named constructors for LPoint3.

  INLINE static LPoint3<NumType> origin(CoordinateSystem cs = CS_default);
  INLINE static LPoint3<NumType> rfu(NumType right,
				     NumType fwd,
				     NumType up,
				     CoordinateSystem cs = CS_default);
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
INLINE LPoint3<NumType2> 
lcast_to(NumType2 *type, const LPoint3<NumType> &source);

#include "lpoint3.I"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, LPoint3<float>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, LPoint3<double>)

#endif
