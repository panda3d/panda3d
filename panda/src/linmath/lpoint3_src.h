// Filename: lpoint3_src.h
// Created by:  drose (25Sep99)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : LPoint3
// Description : This is a three-component point in space (as opposed
//               to a three-component vector, which represents a
//               direction and a distance).  Some of the methods are
//               slightly different between LPoint3 and LVector3; in
//               particular, subtraction of two points yields a
//               vector, while addition of a vector and a point yields
//               a point.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FLOATNAME(LPoint3) : public FLOATNAME(LVecBase3) {
PUBLISHED:
  INLINE_LINMATH FLOATNAME(LPoint3)();
  INLINE_LINMATH FLOATNAME(LPoint3)(const FLOATNAME(LVecBase3) &copy);
  INLINE_LINMATH FLOATNAME(LPoint3) &operator = (const FLOATNAME(LVecBase3) &copy);
  INLINE_LINMATH FLOATNAME(LPoint3) &operator = (FLOATTYPE fill_value);
  INLINE_LINMATH FLOATNAME(LPoint3)(FLOATTYPE fill_value);
  INLINE_LINMATH FLOATNAME(LPoint3)(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z);

  INLINE_LINMATH static const FLOATNAME(LPoint3) &zero();
  INLINE_LINMATH static const FLOATNAME(LPoint3) &unit_x();
  INLINE_LINMATH static const FLOATNAME(LPoint3) &unit_y();
  INLINE_LINMATH static const FLOATNAME(LPoint3) &unit_z();

  INLINE_LINMATH FLOATNAME(LPoint3) operator - () const;

  INLINE_LINMATH FLOATNAME(LVecBase3)
  operator + (const FLOATNAME(LVecBase3) &other) const;
  INLINE_LINMATH FLOATNAME(LPoint3)
  operator + (const FLOATNAME(LVector3) &other) const;

  INLINE_LINMATH FLOATNAME(LVecBase3)
  operator - (const FLOATNAME(LVecBase3) &other) const;
  INLINE_LINMATH FLOATNAME(LVector3)
  operator - (const FLOATNAME(LPoint3) &other) const;
  INLINE_LINMATH FLOATNAME(LPoint3)
  operator - (const FLOATNAME(LVector3) &other) const;

  INLINE_LINMATH FLOATNAME(LPoint3) cross(const FLOATNAME(LVecBase3) &other) const;
  INLINE_LINMATH FLOATNAME(LPoint3) operator * (FLOATTYPE scalar) const;
  INLINE_LINMATH FLOATNAME(LPoint3) operator / (FLOATTYPE scalar) const;

  // Some special named constructors for LPoint3.

  INLINE_LINMATH static FLOATNAME(LPoint3) origin(CoordinateSystem cs = CS_default);
  INLINE_LINMATH static FLOATNAME(LPoint3) rfu(FLOATTYPE right,
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

#include "lpoint3_src.I"
