// Filename: frustum.I
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "frustum.h"
#include "mathutil.h"
#include "config_mathutil.h"

#include <math.h>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: Constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
template<class P_numtype>
Frustum<P_numtype>::
Frustum() {
  _fnear = 1.4142;
  _ffar = 10.0;
  _l = -1;
  _r = 1;
  _t = 1;
  _b = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: make_ortho_2D 
//       Access:
//  Description: Sets up a two-dimensional orthographic frustum 
////////////////////////////////////////////////////////////////////
template<class P_numtype>
void Frustum<P_numtype>::make_ortho_2D(void) {
  make_ortho(-1, 1);
}

////////////////////////////////////////////////////////////////////
//     Function: make_ortho_2D 
//       Access:
//  Description: Sets up a two-dimensional orthographic frustum 
////////////////////////////////////////////////////////////////////
template<class P_numtype>
void Frustum<P_numtype>::
make_ortho_2D(P_numtype l, P_numtype r, P_numtype t, P_numtype b) {
  make_ortho(-1, 1, l, r, t, b);
}

////////////////////////////////////////////////////////////////////
//     Function: make_ortho_2D
//       Access:
//  Description: Behaves like gluOrtho
////////////////////////////////////////////////////////////////////
template<class P_numtype>
void Frustum<P_numtype>::make_ortho(P_numtype fnear, P_numtype ffar) {
  _fnear = fnear;
  _ffar = ffar; 
  _l = -1;
  _r = 1;
  _t = 1;
  _b = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: make_ortho_2D
//       Access:
//  Description: Behaves like gluOrtho
////////////////////////////////////////////////////////////////////
template<class P_numtype>
void Frustum<P_numtype>::
make_ortho(P_numtype fnear, P_numtype ffar, P_numtype l, P_numtype r,
					  P_numtype t, P_numtype b) {
  _fnear = fnear;
  _ffar = ffar; 
  _l = l;
  _r = r;
  _t = t;
  _b = b;
}

////////////////////////////////////////////////////////////////////
//     Function: make_perspective
//       Access:
//  Description: Behaves like gluPerspective (Aspect = width/height,
//		 Yfov in degrees)
//       aspect
//   +------------+
//   |   	  |
// 1 |            | yfov
//   |            |
//   +------------+
//
//     -------+------
//      \     |     /
//	 \    |    /
//	  \   |   /
//	   \  |  /
//	    \ | /
//	     \|/
//	      W yfov
//
////////////////////////////////////////////////////////////////////
template<class P_numtype>
void Frustum<P_numtype>::
make_perspective_hfov(P_numtype hfov, P_numtype aspect, P_numtype fnear,
		      P_numtype ffar) {
  _fnear = fnear;
  _ffar = ffar;
  _r = tan(deg_2_rad(hfov) * 0.5) * _fnear;
  _l = -_r;
  _t = _r / aspect;
  _b = -_t;
}

template<class P_numtype>
void Frustum<P_numtype>::
make_perspective_vfov(P_numtype yfov, P_numtype aspect, P_numtype fnear,
		      P_numtype ffar) {
  _fnear = fnear;
  _ffar = ffar;
  _t = tan(deg_2_rad(yfov) * 0.5) * _fnear;
  _b = -_t;
  _r = _t * aspect;
  _l = -_r;
}

template<class P_numtype>
void Frustum<P_numtype>::
make_perspective(P_numtype xfov, P_numtype yfov, P_numtype fnear,
		 P_numtype ffar) {
  _fnear = fnear;
  _ffar = ffar;
  _t = tan(deg_2_rad(yfov) * 0.5) * _fnear;
  _b = -_t;
  _r = tan(deg_2_rad(xfov) * 0.5) * _fnear;
  _l = -_r;
}

////////////////////////////////////////////////////////////////////
//     Function: get_perspective_params
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
template<class P_numtype>
void Frustum<P_numtype>::
get_perspective_params(P_numtype& yfov, P_numtype& aspect,
		       P_numtype& fnear, P_numtype& ffar) const {
  yfov = rad_2_deg(atan(_t / _fnear)) * 2.0;
  aspect = _r / _t;
  fnear = _fnear;
  ffar = _ffar;
}

////////////////////////////////////////////////////////////////////
//     Function: get_perspective_params
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
template<class P_numtype>
void Frustum<P_numtype>::
get_perspective_params(P_numtype& xfov, P_numtype& yfov, P_numtype& aspect,
		       P_numtype& fnear, P_numtype& ffar) const {
  xfov = rad_2_deg(atan(_r / _fnear)) * 2.0;
  get_perspective_params(yfov, aspect, fnear, ffar);
}

////////////////////////////////////////////////////////////////////
//     Function: get_perspective_projection_mat
//       Access: Public
//  Description: This computes a transform matrix that performs the
//               perspective transform defined by the frustum,
//               accordinate to the indicated coordinate system.
////////////////////////////////////////////////////////////////////
template<class P_numtype>
LMatrix4<P_numtype> Frustum<P_numtype>::
get_perspective_projection_mat(CoordinateSystem cs) const {
  if (cs == CS_default) {
    cs = default_coordinate_system;
  }
  
  P_numtype a = (2.0 * _fnear) / (_r - _l);
  P_numtype b = (_t + _b) / (_t - _b);
  P_numtype c = (_ffar + _fnear) / (_ffar - _fnear);
  P_numtype d = (_r + _l) / (_r - _l);
  P_numtype e = (2.0 * _fnear) / (_t - _b);
  P_numtype f = (-2.0 * _ffar * _fnear) / (_ffar - _fnear);

  switch (cs) {
  case CS_zup_right:
    return LMatrix4<P_numtype>(  a, 0.0, 0.0, 0.0,
			       0.0,  -b,   c, 1.0,
				 d,   e, 0.0, 0.0,
			       0.0, 0.0,   f, 0.0);

  case CS_yup_right:
    return LMatrix4<P_numtype>(  a, 0.0, 0.0, 0.0,
			       0.0,   e, 0.0, 0.0,
			         d,   b,  -c,-1.0,
			       0.0, 0.0,   f, 0.0);

  case CS_zup_left:
    return LMatrix4<P_numtype>::convert_mat(CS_zup_right, CS_zup_left) *
      get_perspective_projection_mat(CS_zup_right);

  case CS_yup_left:
    return LMatrix4<P_numtype>::convert_mat(CS_yup_right, CS_yup_left) *
      get_perspective_projection_mat(CS_yup_right);
   
  default:
    mathutil_cat.error()
      << "Invalid coordinate system!\n";
    return LMatrix4<P_numtype>::ident_mat();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: get_ortho_projection_mat
//       Access: Public
//  Description: This computes a transform matrix that performs the
//               orthographic transform defined by the frustum,
//               accordinate to the indicated coordinate system.
////////////////////////////////////////////////////////////////////
template<class P_numtype>
LMatrix4<P_numtype> Frustum<P_numtype>::
get_ortho_projection_mat(CoordinateSystem cs) const {
  if (cs == CS_default) {
    cs = default_coordinate_system;
  }
 
  P_numtype a = 2.0 / (_r - _l);
  P_numtype b = 2.0 / (_t - _b);
  P_numtype c = 2.0 / (_ffar - _fnear);
  P_numtype d = (_r + _l) / (_r - _l);
  P_numtype e = (_t + _b) / (_t - _b);
  P_numtype f = (_ffar + _fnear) / (_ffar - _fnear);

  switch (cs) {
  case CS_zup_right:
    return LMatrix4<P_numtype>::convert_mat(CS_yup_right, CS_zup_right) *
      get_ortho_projection_mat(CS_yup_right);

  case CS_yup_right:
    return LMatrix4<P_numtype>(  a, 0.0, 0.0, 0.0,
                               0.0,   b, 0.0, 0.0,
                               0.0, 0.0,  -c, 0.0,
                                -d,  -e,  -f, 1.0);

  case CS_zup_left:
    return LMatrix4<P_numtype>::convert_mat(CS_zup_right, CS_zup_left) *
      get_ortho_projection_mat(CS_zup_right);

  case CS_yup_left:
    return LMatrix4<P_numtype>::convert_mat(CS_yup_right, CS_yup_left) *
      get_ortho_projection_mat(CS_yup_right);

  default:
    mathutil_cat.error()
      << "Invalid coordinate system!\n";
    return LMatrix4<P_numtype>::ident_mat();
  }
}
