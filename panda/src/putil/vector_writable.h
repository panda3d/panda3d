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
// Description : A vector of TypedWritable.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a vector of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the vector again.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, std::vector<Writable*>)
typedef vector<Writable*> vector_writable;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
