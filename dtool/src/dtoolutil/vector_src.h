// Filename: vector_src.h
// Created by:  drose (15May01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//
// This file defines the interface to declare and export from the DLL
// an STL vector of some type.
//
// To use this file you must #define a number of symbols and then
// #include it from a .h file.  You also must do the same thing with
// vector_something_src.cxx from a .cxx file.
//
// This is necessary because of the complexity involved in exporting a
// vector class from a DLL.  If we are using the Dinkumware STL
// implementation, it is even more complex.  However, all this
// complexity is only needed to support Windows builds; Unix shared
// libraries are able to export symbols (including templates) without
// any special syntax.
//
////////////////////////////////////////////////////////////////////

// The following variables should be defined prior to including this
// file:
//
//   EXPCL - the appropriate EXPCL_* symbol for this DLL.
//   EXPTP - the appropriate EXPTP_* symbol for this DLL.
//   TYPE - the type of thing we are building a vector on.
//   NAME - The name of the resulting vector typedef, e.g. vector_int.
//
// They will automatically be undefined at the end of the file.

#include "pvector.h"

#if defined(WIN32_VC) && !defined(CPPPARSER)

  #ifdef HAVE_DINKUM
// With the Dinkum library, we must first export the base class,
// _Vector_val.
    #define VV_BASE std::_Vector_val<TYPE, pallocator_array<TYPE> >
#pragma warning (disable : 4231)
EXPORT_TEMPLATE_CLASS(EXPCL, EXPTP, VV_BASE)
    #undef VV_BASE
  #endif

// Now we can export the vector class.
#pragma warning (disable : 4231)

#ifndef USE_STL_ALLOCATOR
EXPORT_TEMPLATE_CLASS(EXPCL, EXPTP, std::vector<TYPE>)
#else
#define STD_VECTOR std::vector<TYPE, pallocator_array<TYPE> >
EXPORT_TEMPLATE_CLASS(EXPCL, EXPTP, STD_VECTOR)
#undef STD_VECTOR
EXPORT_TEMPLATE_CLASS(EXPCL, EXPTP, pvector<TYPE>)
#endif

#endif

// Now make a typedef for the vector.

#ifndef USE_STL_ALLOCATOR
typedef std::vector<TYPE> NAME;
#else
typedef pvector<TYPE> NAME;
#endif

  // Finally, we must define a non-inline function that performs the
  // insert operation given a range of pointers.  We do this because
  // the Dinkum STL implementation uses member templates to handle
  // this, but we cannot export the member templates from the DLL.

/*
extern EXPCL void
insert_into_vector(NAME &vec, NAME::iterator where,
                   const TYPE *begin, const TYPE *end);
*/


#undef EXPCL
#undef EXPTP
#undef TYPE
#undef NAME

