// Filename: lvecBase2.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LVECBASE2_H
#define LVECBASE2_H

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
#include "lvecBase2.I"

#include "dblnames.I"
#include "lvecBase2.I"

////////////////////////////////////////////////////////////////////
//     Function: lcast_to
//  Description: Converts a vector from one numeric representation to
//               another one.  This is usually invoked using the macro
//               LCAST.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME2(LVecBase2) 
lcast_to(FLOATTYPE2 *, const FLOATNAME(LVecBase2) &source) {
  return FLOATNAME2(LVecBase2)(source[0], source[1]);
}

#include "fltnames.I"
INLINE FLOATNAME2(LVecBase2) 
lcast_to(FLOATTYPE2 *, const FLOATNAME(LVecBase2) &source) {
  return FLOATNAME2(LVecBase2)(source[0], source[1]);
}




#endif
