// Filename: lvector3_src.h
// Created by:  drose (24Sep99)
// 
////////////////////////////////////////////////////////////////////

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
class EXPCL_PANDA FLOATNAME(LVector3) : public FLOATNAME(LVecBase3) {
PUBLISHED:
  INLINE FLOATNAME(LVector3)();
  INLINE FLOATNAME(LVector3)(const FLOATNAME(LVecBase3) &copy);
  INLINE FLOATNAME(LVector3) &operator = (const FLOATNAME(LVecBase3) &copy);
  INLINE FLOATNAME(LVector3) &operator = (FLOATTYPE fill_value);
  INLINE FLOATNAME(LVector3)(FLOATTYPE fill_value);
  INLINE FLOATNAME(LVector3)(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z);

  INLINE static const FLOATNAME(LVector3) &zero();
  INLINE static const FLOATNAME(LVector3) &unit_x();
  INLINE static const FLOATNAME(LVector3) &unit_y();
  INLINE static const FLOATNAME(LVector3) &unit_z();

  INLINE FLOATNAME(LVector3) operator - () const;

  INLINE FLOATNAME(LVecBase3) operator + (const FLOATNAME(LVecBase3) &other) const;
  INLINE FLOATNAME(LVector3) operator + (const FLOATNAME(LVector3) &other) const;

  INLINE FLOATNAME(LVecBase3) operator - (const FLOATNAME(LVecBase3) &other) const;
  INLINE FLOATNAME(LVector3) operator - (const FLOATNAME(LVector3) &other) const;

  INLINE FLOATTYPE length() const;
  INLINE FLOATTYPE length_squared() const;
  INLINE bool normalize();
  INLINE FLOATNAME(LVector3) cross(const FLOATNAME(LVecBase3) &other) const;
  INLINE FLOATNAME(LVector3) operator * (FLOATTYPE scalar) const;
  INLINE FLOATNAME(LVector3) operator / (FLOATTYPE scalar) const;

  // Some special named constructors for LVector3.

  INLINE static FLOATNAME(LVector3) up(CoordinateSystem cs = CS_default);
  INLINE static FLOATNAME(LVector3) right(CoordinateSystem cs = CS_default);
  INLINE static FLOATNAME(LVector3) forward(CoordinateSystem cs = CS_default);

  INLINE static FLOATNAME(LVector3) down(CoordinateSystem cs = CS_default);
  INLINE static FLOATNAME(LVector3) left(CoordinateSystem cs = CS_default);
  INLINE static FLOATNAME(LVector3) back(CoordinateSystem cs = CS_default);

  INLINE static FLOATNAME(LVector3) rfu(FLOATTYPE right,
					FLOATTYPE fwd,
					FLOATTYPE up,
					CoordinateSystem cs = CS_default);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();
 
private:
  static TypeHandle _type_handle;
};

#include "lvector3_src.I"
