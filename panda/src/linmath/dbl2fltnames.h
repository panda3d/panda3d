// Filename: dbl2fltnames.h
// Created by:  drose (04Apr01)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//
// This file is used particularly by lcast_to.h and lcast_to.cxx to
// define functions that convert from type double to type float.
//
////////////////////////////////////////////////////////////////////

#include "fltnames.h"

#undef FLOATTYPE2
#undef FLOATNAME2
#undef FLOATTOKEN2

#define FLOATTYPE2 double
#define FLOATNAME2(ARG) ARG##d
#define FLOATTOKEN2 'd'
