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

#ifdef HAVE_DINKUM
#define VV_COLORF std::_Vector_val<Colorf, std::allocator<Colorf> >
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, VV_COLORF)
#endif
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, std::vector<Colorf>)
typedef vector<Colorf> vector_Colorf;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
