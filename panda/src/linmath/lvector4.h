// Filename: lvector4.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LVECTOR4_H
#define LVECTOR4_H

#include <pandabase.h>
#include "cmath.h"
#include "config_linmath.h"
#include "lvecBase4.h"

#include "fltnames.I"
#include "lvector4.I"

#include "dblnames.I"
#include "lvector4.I"

////////////////////////////////////////////////////////////////////
//     Function: lcast_to
//  Description: Converts a vector from one numeric representation to
//               another one.  This is usually invoked using the macro
//               LCAST.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME2(LVector4) 
lcast_to(FLOATTYPE2 *, const FLOATNAME(LVector4) &source) {
  return FLOATNAME2(LVector4)(source[0], source[1], source[2], source[3]);
}

#include "fltnames.I"
INLINE FLOATNAME2(LVector4) 
lcast_to(FLOATTYPE2 *, const FLOATNAME(LVector4) &source) {
  return FLOATNAME2(LVector4)(source[0], source[1], source[2], source[3]);
}




#endif
