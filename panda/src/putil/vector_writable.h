// Filename: vector_writable.h
// Created by:  jason (14Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef VECTOR_WRITABLE_H
#define VECTOR_WRITABLE_H

#include <pandabase.h>

#include <vector>

class Writable;

////////////////////////////////////////////////////////////////////
//       Class : vector_writable
// Description : A vector of Writable *.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a vector of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the vector again.
////////////////////////////////////////////////////////////////////

#define EXPCL EXPCL_PANDA 
#define EXPTP EXPTP_PANDA 
#define TYPE Writable *
#define NAME vector_writable

#include <vector_src.h>

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
