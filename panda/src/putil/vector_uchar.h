// Filename: vector_uchar.h
// Created by:  drose (10May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef VECTOR_UCHAR_H
#define VECTOR_UCHAR_H

#include <pandabase.h>

#include <vector>

////////////////////////////////////////////////////////////////////
//       Class : vector_uchar
// Description : A vector of uchars.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a vector of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the vector again.
////////////////////////////////////////////////////////////////////

#define EXPCL EXPCL_PANDA 
#define EXPTP EXPTP_PANDA 
#define TYPE unsigned char
#define NAME vector_uchar

#include <vector_src.h>

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
