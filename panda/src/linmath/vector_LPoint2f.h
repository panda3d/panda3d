// Filename: vector_LPoint2f.h
// Created by:  drose (10May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef VECTOR_LPOINT2F_H
#define VECTOR_LPOINT2F_H

#include <pandabase.h>

#include "luse.h"

#include <vector>

////////////////////////////////////////////////////////////////////
//       Class : vector_LPoint2f
// Description : A vector of LPoint2fs.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a vector of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the vector again.
////////////////////////////////////////////////////////////////////

#define EXPCL EXPCL_PANDA 
#define EXPTP EXPTP_PANDA 
#define TYPE LPoint2f
#define NAME vector_LPoint2f

#include <vector_src.h>

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
