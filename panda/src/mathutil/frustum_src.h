// Filename: frustum_src.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : Frustum
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FLOATNAME(Frustum) {
PUBLISHED:
  INLINE_MATHUTIL FLOATNAME(Frustum)();
 
  INLINE_MATHUTIL void make_ortho_2D(void);
  INLINE_MATHUTIL void make_ortho_2D(FLOATTYPE l, FLOATTYPE r, FLOATTYPE t, FLOATTYPE b);

  INLINE_MATHUTIL void make_ortho(FLOATTYPE fnear, FLOATTYPE ffar);
  INLINE_MATHUTIL void make_ortho(FLOATTYPE fnear, FLOATTYPE ffar,
                         FLOATTYPE l, FLOATTYPE r, FLOATTYPE t, FLOATTYPE b);
  
  INLINE_MATHUTIL void make_perspective_hfov(FLOATTYPE xfov, FLOATTYPE aspect,
                                    FLOATTYPE fnear, FLOATTYPE ffar);
  INLINE_MATHUTIL void make_perspective_vfov(FLOATTYPE yfov, FLOATTYPE aspect,
                                    FLOATTYPE fnear, FLOATTYPE ffar);
  INLINE_MATHUTIL void make_perspective(FLOATTYPE xfov, FLOATTYPE yfov, FLOATTYPE fnear,
                               FLOATTYPE ffar);
  INLINE_MATHUTIL void get_perspective_params(FLOATTYPE &yfov, FLOATTYPE &aspect,
                                     FLOATTYPE &fnear, FLOATTYPE &ffar) const;
  INLINE_MATHUTIL void get_perspective_params(FLOATTYPE &xfov, FLOATTYPE &yfov,
                                     FLOATTYPE &aspect, FLOATTYPE &fnear,
                                     FLOATTYPE &ffar) const;
  
public: 
  INLINE_MATHUTIL FLOATNAME(LMatrix4)
  get_perspective_projection_mat(CoordinateSystem cs = CS_default) const;

  INLINE_MATHUTIL FLOATNAME(LMatrix4)
  get_ortho_projection_mat(CoordinateSystem cs = CS_default) const;
 
public:
  FLOATTYPE _l, _r, _b, _t;
  FLOATTYPE _fnear, _ffar;
};

#include "frustum_src.I"
