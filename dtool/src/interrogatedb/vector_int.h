// Filename: vector_int.h
// Created by:  drose (10May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef VECTOR_INT_H
#define VECTOR_INT_H

#include <dtoolbase.h>

#include <vector>

////////////////////////////////////////////////////////////////////
//       Class : vector_int
// Description : A vector of ints.  This class is defined once here,
//               and exported to DTOOL.DLL; other packages that want
//               to use a vector of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the vector again.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_DTOOL, EXPTP_DTOOL, std::vector<int>)
typedef vector<int> vector_int;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
