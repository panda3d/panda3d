// Filename: lvector3.h
// Created by:  drose (24Sep99)
//
////////////////////////////////////////////////////////////////////

#ifndef LVECTOR3_H
#define LVECTOR3_H

#include <pandabase.h>

#include "coordinateSystem.h"
#include "lvecBase3.h"

////////////////////////////////////////////////////////////////////
// 	 Class : LVector3
// Description : This is a three-component vector distance (as opposed
//               to a three-component point, which represents a
//               particular point in space).  Some of the methods are
//               slightly different between LPoint3 and LVector3; in
//               particular, subtraction of two points yields a
//               vector, while addition of a vector and a point yields
//               a point.
////////////////////////////////////////////////////////////////////
template<class NumType>
class LVector3 : public LVecBase3<NumType> {
public:
  INLINE LVector3();
  INLINE LVector3(const LVecBase3<NumType> &copy);
  INLINE LVector3<NumType> &operator = (const LVecBase3<NumType> &copy);
  INLINE LVector3<NumType> &operator = (NumType fill_value);
  INLINE LVector3(NumType fill_value);
  INLINE LVector3(NumType x, NumType y, NumType z);

  INLINE static LVector3<NumType> zero();
  INLINE static LVector3<NumType> unit_x();
  INLINE static LVector3<NumType> unit_y();
  INLINE static LVector3<NumType> unit_z();

  INLINE LVector3<NumType> operator - () const;

  INLINE LVecBase3<NumType>
  operator + (const LVecBase3<NumType> &other) const;
  INLINE LVector3<NumType>
  operator + (const LVector3<NumType> &other) const;

  INLINE LVecBase3<NumType>
  operator - (const LVecBase3<NumType> &other) const;
  INLINE LVector3<NumType>
  operator - (const LVector3<NumType> &other) const;

  INLINE NumType length() const;
  INLINE NumType length_squared() const;
  INLINE bool normalize();
  INLINE LVector3<NumType> cross(const LVecBase3<NumType> &other) const;
  INLINE LVector3<NumType> operator * (NumType scalar) const;
  INLINE LVector3<NumType> operator / (NumType scalar) const;

  // Some special named constructors for LVector3.

  static LVector3<NumType> up(CoordinateSystem cs = CS_default);
  INLINE static LVector3<NumType> right(CoordinateSystem cs = CS_default);
  static LVector3<NumType> forward(CoordinateSystem cs = CS_default);

  INLINE static LVector3<NumType> down(CoordinateSystem cs = CS_default);
  INLINE static LVector3<NumType> left(CoordinateSystem cs = CS_default);
  INLINE static LVector3<NumType> back(CoordinateSystem cs = CS_default);

  INLINE static LVector3<NumType> rfu(NumType right,
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
INLINE LVector3<NumType2> 
lcast_to(NumType2 *type, const LVector3<NumType> &source);

#include "lvector3.I"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, LVector3<float>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, LVector3<double>)

#endif
