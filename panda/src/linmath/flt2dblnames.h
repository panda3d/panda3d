// Filename: flt2dblnames.h
// Created by:  drose (04Apr01)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//
// This file is used particularly by lcast_to.h and lcast_to.cxx to
// define functions that convert from type float to type double.
//
////////////////////////////////////////////////////////////////////

#include "dblnames.h"

#undef FLOATTYPE2
#undef FLOATNAME2
#undef FLOATTOKEN2

#define FLOATTYPE2 float
#define FLOATNAME2(ARG) ARG##f
#define FLOATTOKEN2 'f'
