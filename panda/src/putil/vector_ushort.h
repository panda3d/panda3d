// Filename: vector_ushort.h
// Created by:  drose (10May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef VECTOR_USHORT_H
#define VECTOR_USHORT_H

#include <pandabase.h>

#include <vector>

////////////////////////////////////////////////////////////////////
//       Class : vector_ushort
// Description : A vector of ushorts.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a vector of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the vector again.
////////////////////////////////////////////////////////////////////

#ifdef HAVE_DINKUM
#define VV_USHORT std::_Vector_val<unsigned short, std::allocator<unsigned short> >
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, VV_USHORT)
#endif
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, std::vector<unsigned short>)
typedef vector<unsigned short> vector_ushort;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
