// Filename: typedWritable.cxx
// Created by:  jason (08Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

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
