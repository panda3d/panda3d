// Filename: vector_int.C
// Created by:  drose (10May00)
// 
////////////////////////////////////////////////////////////////////

#include "vector_int.h"

#define EXPCL EXPCL_DTOOLCONFIG 
#define EXPTP EXPTP_DTOOLCONFIG 
#define TYPE int
#define NAME vector_int

#include "vector_src.cxx"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma implementation
#endif
