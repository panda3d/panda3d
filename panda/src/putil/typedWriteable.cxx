// Filename: typeWriteable.h
// Created by:  jason (08Jun00)
//

#include "typedWriteable.h"

TypeHandle TypedWriteable::_type_handle;
TypedWriteable* const TypedWriteable::Null = (TypedWriteable*)0L;

////////////////////////////////////////////////////////////////////
//     Function: TypedWriteable::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
TypedWriteable::~TypedWriteable() 
{ 
}


////////////////////////////////////////////////////////////////////
//     Function: TypedWriteable::complete_pointers
//       Access: Public, Virtual
//  Description: Takes in a vector of pointers to TypedWriteable
//               objects that correspond to all the requests for 
//               pointers that this object made to BamReader.
////////////////////////////////////////////////////////////////////
int TypedWriteable::
complete_pointers(vector_typedWriteable &, BamReader*)
{
  return 0;
}
