// Filename: vector_string.h
// Created by:  drose (15May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef VECTOR_STRING_H
#define VECTOR_STRING_H

#include <dtoolbase.h>

#include <vector>

////////////////////////////////////////////////////////////////////
//       Class : vector_string
// Description : A vector of strings.  This class is defined once here,
//               and exported to DTOOL.DLL; other packages that want
//               to use a vector of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the vector again.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_DTOOL, EXPTP_DTOOL, std::vector<std::string>)
typedef vector<string> vector_string;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
