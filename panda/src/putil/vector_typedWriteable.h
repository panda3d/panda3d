// Filename: vector_typedWriteable.h
// Created by:  jason (19Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef VECTOR_TYPED_WRITEABLE_H
#define VECTOR_TYPED_WRITEABLE_H

#include <pandabase.h>

#include <vector>

class TypedWriteable;

////////////////////////////////////////////////////////////////////
//       Class : vector_typedWriteable
// Description : A vector of TypedWriteable.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a vector of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the vector again.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, std::vector<TypedWriteable*>)
typedef vector<TypedWriteable*> vector_typedWriteable;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
