// Filename: lpoint3.h
// Created by:  drose (25Sep99)
//
////////////////////////////////////////////////////////////////////

#ifndef LPOINT3_H
#define LPOINT3_H

#include <pandabase.h>

#include "coordinateSystem.h"
#include "lvecBase3.h"
#include "lvector3.h"

#include "fltnames.I"
#include "lpoint3.I"

#include "dblnames.I"
#include "lpoint3.I"


////////////////////////////////////////////////////////////////////
//     Function: lcast_to
//  Description: Converts a vector from one numeric representation to
//               another one.  This is usually invoked using the macro
//               LCAST.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME2(LPoint3) 
lcast_to(FLOATTYPE2 *, const FLOATNAME(LPoint3) &source) {
  return FLOATNAME2(LPoint3)(source[0], source[1], source[2]);
}

#include "fltnames.I"
INLINE FLOATNAME2(LPoint3) 
lcast_to(FLOATTYPE2 *, const FLOATNAME(LPoint3) &source) {
  return FLOATNAME2(LPoint3)(source[0], source[1], source[2]);
}




#endif
