// Filename: vector_ulong.h
// Created by:  drose (10May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef VECTOR_ULONG_H
#define VECTOR_ULONG_H

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

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, std::vector<unsigned long>)
typedef vector<unsigned long> vector_ulong;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
