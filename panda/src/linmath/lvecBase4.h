// Filename: lvecBase4.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LVECBASE4_H
#define LVECBASE4_H

#include <pandabase.h>
#include <typeHandle.h>
#include <notify.h>
#include <datagram.h>
#include <datagramIterator.h>
#include "cmath.h"
#include "nearly_zero.h"

class Datagram;
class DatagramIterator;

#include "fltnames.I"
#include "lvecBase4.I"

#include "dblnames.I"
#include "lvecBase4.I"

////////////////////////////////////////////////////////////////////
//     Function: lcast_to
//  Description: Converts a vector from one numeric representation to
//               another one.  This is usually invoked using the macro
//               LCAST.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME2(LVecBase4) 
lcast_to(FLOATTYPE2 *, const FLOATNAME(LVecBase4) &source) {
  return FLOATNAME2(LVecBase4)(source[0], source[1], source[2], source[3]);
}

#include "fltnames.I"
INLINE FLOATNAME2(LVecBase4) 
lcast_to(FLOATTYPE2 *, const FLOATNAME(LVecBase4) &source) {
  return FLOATNAME2(LVecBase4)(source[0], source[1], source[2], source[3]);
}





#endif
