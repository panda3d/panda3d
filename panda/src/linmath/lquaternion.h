// Filename: lquaternion.h
// Created by:  frang (06Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __LQUATERNION_H__
#define __LQUATERNION_H__

#include "lmatrix.h"
#include "lvector3.h"
#include "lvector4.h"
#include "nearly_zero.h"
#include "cmath.h"
#include "deg_2_rad.h"

#include <notify.h>

#include "fltnames.I"
#include "lquaternion.I"

#include "dblnames.I"
#include "lquaternion.I"

////////////////////////////////////////////////////////////////////
//     Function: lcast_to
//  Description: Converts a quaternion from one numeric representation
//               to another one.  This is usually invoked using the
//               macro LCAST.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME2(LQuaternionBase)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LQuaternionBase)& c) {
  return FLOATNAME2(LQuaternionBase)(c.get_r(), c.get_i(), c.get_j(), c.get_k());
}

#include "fltnames.I"
INLINE FLOATNAME2(LQuaternionBase)
lcast_to(FLOATTYPE2 *, const FLOATNAME(LQuaternionBase)& c) {
  return FLOATNAME2(LQuaternionBase)(c.get_r(), c.get_i(), c.get_j(), c.get_k());
}





#endif /* __LQUATERNION_H__ */
