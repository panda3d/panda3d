// Filename: lvector2_src.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// 	 Class : LVector2
// Description : This is a two-component vector offset.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FLOATNAME(LVector2) : public FLOATNAME(LVecBase2) {
PUBLISHED:
  INLINE FLOATNAME(LVector2)();
  INLINE FLOATNAME(LVector2)(const FLOATNAME(LVecBase2) &copy);
  INLINE FLOATNAME(LVector2) &operator = (const FLOATNAME(LVecBase2) &copy);
  INLINE FLOATNAME(LVector2) &operator = (FLOATTYPE fill_value);
  INLINE FLOATNAME(LVector2)(FLOATTYPE fill_value);
  INLINE FLOATNAME(LVector2)(FLOATTYPE x, FLOATTYPE y);

  INLINE static const FLOATNAME(LVector2) &zero();
  INLINE static const FLOATNAME(LVector2) &unit_x();
  INLINE static const FLOATNAME(LVector2) &unit_y();

  INLINE FLOATNAME(LVector2) operator - () const;

  INLINE FLOATNAME(LVecBase2)operator + (const FLOATNAME(LVecBase2) &other) const;
  INLINE FLOATNAME(LVector2) operator + (const FLOATNAME(LVector2) &other) const;

  INLINE FLOATNAME(LVecBase2) operator - (const FLOATNAME(LVecBase2) &other) const;
  INLINE FLOATNAME(LVector2) operator - (const FLOATNAME(LVector2) &other) const;

  INLINE FLOATTYPE length() const;
  INLINE FLOATTYPE length_squared() const;
  INLINE bool normalize();
  INLINE FLOATNAME(LVector2) operator * (FLOATTYPE scalar) const;
  INLINE FLOATNAME(LVector2) operator / (FLOATTYPE scalar) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();
 
private:
  static TypeHandle _type_handle;
};

#include "lvector2_src.I"
