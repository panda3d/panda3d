// Filename: lmatrix4.h
// Created by:  drose (29Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef LMATRIX4_H
#define LMATRIX4_H

#include <pandabase.h>
#include <math.h>
#include <typeHandle.h>
#include <datagram.h>
#include <datagramIterator.h>
#include <indent.h>

#include "deg_2_rad.h"
#include "nearly_zero.h"

#include "coordinateSystem.h"
#include "lvecBase4.h"
#include "lvecBase3.h"
#include "lmatrix3.h"

#include "fltnames.I"
#include "lmatrix4.I"

#include "dblnames.I"
#include "lmatrix4.I"


////////////////////////////////////////////////////////////////////
//     Function: lcast_to
//  Description: Converts a matrix from one numeric representation to
//               another one.  This is usually invoked using the macro
//               LCAST.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME2(LMatrix4) 
lcast_to(FLOATTYPE2 *, const FLOATNAME(LMatrix4) &source) {
  return FLOATNAME2(LMatrix4)
    (source(0, 0), source(0, 1), source(0, 2), source(0, 3),
     source(1, 0), source(1, 1), source(1, 2), source(1, 3),
     source(2, 0), source(2, 1), source(2, 2), source(2, 3),
     source(3, 0), source(3, 1), source(3, 2), source(3, 3));
}

#include "fltnames.I"

INLINE FLOATNAME2(LMatrix4) 
lcast_to(FLOATTYPE2 *, const FLOATNAME(LMatrix4) &source) {
  return FLOATNAME2(LMatrix4)
    (source(0, 0), source(0, 1), source(0, 2), source(0, 3),
     source(1, 0), source(1, 1), source(1, 2), source(1, 3),
     source(2, 0), source(2, 1), source(2, 2), source(2, 3),
     source(3, 0), source(3, 1), source(3, 2), source(3, 3));
}

#endif
