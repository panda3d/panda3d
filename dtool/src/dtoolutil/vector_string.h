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

#define EXPCL EXPCL_DTOOL 
#define EXPTP EXPTP_DTOOL 
#define TYPE std::string
#define NAME vector_string

#include "vector_src.h"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
