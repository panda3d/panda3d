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

#ifdef HAVE_DINKUM
#define VV_LVECBASE3F std::_Vector_val<LVecBase3f, std::allocator<LVecBase3f> >
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, VV_LVECBASE3F)
#endif
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, std::vector<LVecBase3f>)
typedef vector<LVecBase3f> vector_LVecBase3f;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
