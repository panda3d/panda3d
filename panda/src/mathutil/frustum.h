// Filename: frustum.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef FRUSTUM_H
#define FRUSTUM_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <luse.h>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : Frustum
// Description :
////////////////////////////////////////////////////////////////////
template<class P_numtype>
class EXPCL_PANDA Frustum {
public:
  Frustum();
 
  void make_ortho_2D(void);
  void make_ortho_2D(P_numtype l, P_numtype r, P_numtype t, P_numtype b);

  void make_ortho(P_numtype fnear, P_numtype ffar);
  void make_ortho(P_numtype fnear, P_numtype ffar,
		  P_numtype l, P_numtype r, P_numtype t, P_numtype b);

  void make_perspective_hfov(P_numtype xfov, P_numtype aspect,
			     P_numtype fnear, P_numtype ffar);
  void make_perspective_vfov(P_numtype yfov, P_numtype aspect,
			     P_numtype fnear, P_numtype ffar);
  void make_perspective(P_numtype xfov, P_numtype yfov, P_numtype fnear,
			P_numtype ffar);
  void get_perspective_params(P_numtype &yfov, P_numtype &aspect,
			      P_numtype &fnear, P_numtype &ffar) const;
  void get_perspective_params(P_numtype &xfov, P_numtype &yfov,
			      P_numtype &aspect, P_numtype &fnear,
			      P_numtype &ffar) const;
 
  LMatrix4<P_numtype>
  get_perspective_projection_mat(CoordinateSystem cs = CS_default) const;

  LMatrix4<P_numtype>
  get_ortho_projection_mat(CoordinateSystem cs = CS_default) const;
 
public:
  P_numtype _l, _r, _b, _t;
  P_numtype _fnear, _ffar;
};

#include "frustum.I"

typedef Frustum<float> Frustumf;
typedef Frustum<double> Frustumd;

#endif
