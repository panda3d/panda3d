// Filename: lpoint4.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LPOINT4_H
#define LPOINT4_H

#include <pandabase.h>

#include "lvecBase4.h"
#include "lvector4.h"

#include "fltnames.I"
#include "lpoint4.I"

#include "dblnames.I"
#include "lpoint4.I"

////////////////////////////////////////////////////////////////////
//     Function: lcast_to
//  Description: Converts a vector from one numeric representation to
//               another one.  This is usually invoked using the macro
//               LCAST.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME2(LPoint4) 
lcast_to(FLOATTYPE2 *, const FLOATNAME(LPoint4) &source) {
  return FLOATNAME2(LPoint4)(source[0], source[1], source[2], source[3]);
}

#include "fltnames.I"

INLINE FLOATNAME2(LPoint4) 
lcast_to(FLOATTYPE2 *, const FLOATNAME(LPoint4) &source) {
  return FLOATNAME2(LPoint4)(source[0], source[1], source[2], source[3]);
}


////EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, LPoint4<float>)
////EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, LPoint4<double>)

#endif
