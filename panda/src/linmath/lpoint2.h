// Filename: lpoint2.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LPOINT2_H
#define LPOINT2_H

#include <pandabase.h>

#include "lvecBase2.h"
#include "lvector2.h"

#include "fltnames.I"
#include "lpoint2.I"

#include "dblnames.I"
#include "lpoint2.I"

////////////////////////////////////////////////////////////////////
//     Function: lcast_to
//  Description: Converts a vector from one numeric representation to
//               another one.  This is usually invoked using the macro
//               LCAST.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME2(LPoint2) 
lcast_to(FLOATTYPE2 *, const FLOATNAME(LPoint2) &source) {
  return FLOATNAME2(LPoint2)(source[0], source[1]);
}

#include "fltnames.I"
INLINE FLOATNAME2(LPoint2) 
lcast_to(FLOATTYPE2 *, const FLOATNAME(LPoint2) &source) {
  return FLOATNAME2(LPoint2)(source[0], source[1]);
}




#endif
