// Filename: vector_Colorf.h
// Created by:  drose (10May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef VECTOR_COLORF_H
#define VECTOR_COLORF_H

#include <pandabase.h>

#include "luse.h"

#include <vector>

////////////////////////////////////////////////////////////////////
//       Class : vector_Colorf
// Description : A vector of Colorfs.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a vector of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the vector again.
////////////////////////////////////////////////////////////////////

#define EXPCL EXPCL_PANDA 
#define EXPTP EXPTP_PANDA 
#define TYPE Colorf
#define NAME vector_Colorf

#include <vector_src.h>

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
