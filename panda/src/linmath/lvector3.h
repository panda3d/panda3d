// Filename: lvector3.h
// Created by:  drose (24Sep99)
//
////////////////////////////////////////////////////////////////////

#ifndef LVECTOR3_H
#define LVECTOR3_H

#include <pandabase.h>

#include "coordinateSystem.h"
#include "cmath.h"
#include "config_linmath.h"
#include "lvecBase3.h"

#include "fltnames.I"
#include "lvector3.I"

#include "dblnames.I"
#include "lvector3.I"

////////////////////////////////////////////////////////////////////
//     Function: lcast_to
//  Description: Converts a vector from one numeric representation to
//               another one.  This is usually invoked using the macro
//               LCAST.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME2(LVector3) 
lcast_to(FLOATTYPE2 *, const FLOATNAME(LVector3) &source) {
  return FLOATNAME2(LVector3)(source[0], source[1], source[2]);
}


#include "fltnames.I"

INLINE FLOATNAME2(LVector3) 
lcast_to(FLOATTYPE2 *, const FLOATNAME(LVector3) &source) {
  return FLOATNAME2(LVector3)(source[0], source[1], source[2]);
}




#endif
