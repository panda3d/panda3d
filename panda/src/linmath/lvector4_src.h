// Filename: lvector4_src.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// 	 Class : LVector4
// Description : This is a four-component vector distance.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FLOATNAME(LVector4) : public FLOATNAME(LVecBase4) {
PUBLISHED:
  INLINE FLOATNAME(LVector4)();
  INLINE FLOATNAME(LVector4)(const FLOATNAME(LVecBase4) &copy);
  INLINE FLOATNAME(LVector4) &operator = (const FLOATNAME(LVecBase4) &copy);
  INLINE FLOATNAME(LVector4) &operator = (FLOATTYPE fill_value);
  INLINE FLOATNAME(LVector4)(FLOATTYPE fill_value);
  INLINE FLOATNAME(LVector4)(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z, FLOATTYPE w);

  INLINE static const FLOATNAME(LVector4) &zero();
  INLINE static const FLOATNAME(LVector4) &unit_x();
  INLINE static const FLOATNAME(LVector4) &unit_y();
  INLINE static const FLOATNAME(LVector4) &unit_z();
  INLINE static const FLOATNAME(LVector4) &unit_w();

  INLINE FLOATNAME(LVector4) operator - () const;

  INLINE FLOATNAME(LVecBase4) operator + (const FLOATNAME(LVecBase4) &other) const;
  INLINE FLOATNAME(LVector4)  operator + (const FLOATNAME(LVector4) &other) const;

  INLINE FLOATNAME(LVecBase4) operator - (const FLOATNAME(LVecBase4) &other) const;
  INLINE FLOATNAME(LVector4)  operator - (const FLOATNAME(LVector4) &other) const;

  INLINE FLOATTYPE length() const;
  INLINE FLOATTYPE length_squared() const;
  INLINE bool normalize();
  INLINE FLOATNAME(LVector4) operator * (FLOATTYPE scalar) const;
  INLINE FLOATNAME(LVector4) operator / (FLOATTYPE scalar) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();
 
private:
  static TypeHandle _type_handle;
};

#include "lvector4_src.I"
