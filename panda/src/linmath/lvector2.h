// Filename: lvector2.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LVECTOR2_H
#define LVECTOR2_H

#include <pandabase.h>
#include "cmath.h"
#include "config_linmath.h"
#include "lvecBase2.h"

#include "fltnames.I"
#include "lvector2.I"

#include "dblnames.I"
#include "lvector2.I"

////////////////////////////////////////////////////////////////////
//     Function: lcast_to
//  Description: Converts a vector from one numeric representation to
//               another one.  This is usually invoked using the macro
//               LCAST.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME2(LVector2) 
lcast_to(FLOATTYPE2 *, const FLOATNAME(LVector2) &source) {
  return FLOATNAME2(LVector2)(source[0], source[1]);
}

#include "fltnames.I"

INLINE FLOATNAME2(LVector2) 
lcast_to(FLOATTYPE2 *, const FLOATNAME(LVector2) &source) {
  return FLOATNAME2(LVector2)(source[0], source[1]);
}




#endif
