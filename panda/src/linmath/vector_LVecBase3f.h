// Filename: vector_LVecBase3f.h
// Created by:  drose (11Dec00)
// 
////////////////////////////////////////////////////////////////////

#ifndef VECTOR_LVECBASE3F_H
#define VECTOR_LVECBASE3F_H

#include <pandabase.h>

#include "luse.h"

#include <vector>

////////////////////////////////////////////////////////////////////
//       Class : vector_LVecBase3f
// Description : A vector of LVecBase3fs.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a vector of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the vector again.
////////////////////////////////////////////////////////////////////

#define EXPCL EXPCL_PANDA 
#define EXPTP EXPTP_PANDA 
#define TYPE LVecBase3f
#define NAME vector_LVecBase3f

#include <vector_src.h>

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
