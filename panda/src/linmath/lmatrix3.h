// Filename: lmatrix3.h
// Created by:  drose (29Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef LMATRIX3_H
#define LMATRIX3_H

#include <pandabase.h>
#include <math.h>
#include <typeHandle.h>
#include <datagram.h>
#include <datagramIterator.h>
#include <notify.h>
#include <indent.h>
#include "deg_2_rad.h"
#include "nearly_zero.h"
#include "coordinateSystem.h"
#include "lvecBase3.h"
#include "lvecBase2.h"

#include "fltnames.I"
#include "lmatrix3.I"

#include "dblnames.I"
#include "lmatrix3.I"

////////////////////////////////////////////////////////////////////
//     Function: lcast_to
//  Description: Converts a matrix from one numeric representation to
//               another one.  This is usually invoked using the macro
//               LCAST.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME2(LMatrix3) 
lcast_to(FLOATTYPE2 *, const FLOATNAME(LMatrix3) &source) {
  return FLOATNAME2(LMatrix3)
    (source(0, 0), source(0, 1), source(0, 2),
     source(1, 0), source(1, 1), source(1, 2),
     source(2, 0), source(2, 1), source(2, 2));
}

#include "fltnames.I"
INLINE FLOATNAME2(LMatrix3) 
lcast_to(FLOATTYPE2 *, const FLOATNAME(LMatrix3) &source) {
  return FLOATNAME2(LMatrix3)
    (source(0, 0), source(0, 1), source(0, 2),
     source(1, 0), source(1, 1), source(1, 2),
     source(2, 0), source(2, 1), source(2, 2));
}






#endif
