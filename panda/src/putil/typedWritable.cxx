// Filename: typeWritable.h
// Created by:  jason (08Jun00)
//

#include "typedWritable.h"

TypeHandle TypedWritable::_type_handle;
TypedWritable* const TypedWritable::Null = (TypedWritable*)0L;

////////////////////////////////////////////////////////////////////
//     Function: TypedWritable::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
TypedWritable::~TypedWritable() 
{ 
}


////////////////////////////////////////////////////////////////////
//     Function: TypedWritable::complete_pointers
//       Access: Public, Virtual
//  Description: Takes in a vector of pointers to TypedWritable
//               objects that correspond to all the requests for 
//               pointers that this object made to BamReader.
////////////////////////////////////////////////////////////////////
int TypedWritable::
complete_pointers(vector_typedWritable &, BamReader*)
{
  return 0;
}
