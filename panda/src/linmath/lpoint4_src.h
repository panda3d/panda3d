// Filename: lpoint4_src.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// 	 Class : LPoint4
// Description : This is a four-component point in space.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FLOATNAME(LPoint4) : public FLOATNAME(LVecBase4) {
PUBLISHED:
  INLINE FLOATNAME(LPoint4)();
  INLINE FLOATNAME(LPoint4)(const FLOATNAME(LVecBase4) &copy);
  INLINE FLOATNAME(LPoint4) &operator = (const FLOATNAME(LVecBase4) &copy);
  INLINE FLOATNAME(LPoint4) &operator = (FLOATTYPE fill_value);
  INLINE FLOATNAME(LPoint4)(FLOATTYPE fill_value);
  INLINE FLOATNAME(LPoint4)(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z, FLOATTYPE w);

  INLINE static const FLOATNAME(LPoint4) &zero();
  INLINE static const FLOATNAME(LPoint4) &unit_x();
  INLINE static const FLOATNAME(LPoint4) &unit_y();
  INLINE static const FLOATNAME(LPoint4) &unit_z();
  INLINE static const FLOATNAME(LPoint4) &unit_w();

  INLINE FLOATNAME(LPoint4) operator - () const;

  INLINE FLOATNAME(LVecBase4)
  operator + (const FLOATNAME(LVecBase4) &other) const;
  INLINE FLOATNAME(LPoint4)
  operator + (const FLOATNAME(LVector4) &other) const;

  INLINE FLOATNAME(LVecBase4)
  operator - (const FLOATNAME(LVecBase4) &other) const;
  INLINE FLOATNAME(LVector4)
  operator - (const FLOATNAME(LPoint4) &other) const;
  INLINE FLOATNAME(LPoint4)
  operator - (const FLOATNAME(LVector4) &other) const;

  INLINE FLOATNAME(LPoint4) operator * (FLOATTYPE scalar) const;
  INLINE FLOATNAME(LPoint4) operator / (FLOATTYPE scalar) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();
 
private:
  static TypeHandle _type_handle;
};

#include "lpoint4_src.I"
