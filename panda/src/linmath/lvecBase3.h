// Filename: lvecBase3.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LVECBASE3_H
#define LVECBASE3_H

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
#include "lvecBase3.I"

#include "dblnames.I"
#include "lvecBase3.I"

////////////////////////////////////////////////////////////////////
//     Function: lcast_to
//  Description: Converts a vector from one numeric representation to
//               another one.  This is usually invoked using the macro
//               LCAST.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME2(LVecBase3) 
lcast_to(FLOATTYPE2 *, const FLOATNAME(LVecBase3) &source) {
  return FLOATNAME2(LVecBase3)(source[0], source[1], source[2]);
}

#include "fltnames.I"
INLINE FLOATNAME2(LVecBase3) 
lcast_to(FLOATTYPE2 *, const FLOATNAME(LVecBase3) &source) {
  return FLOATNAME2(LVecBase3)(source[0], source[1], source[2]);
}

#endif
