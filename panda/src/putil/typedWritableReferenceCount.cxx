// Filename: typedWritableReferenceCount.cxx
// Created by:  jason (08Jun00)
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

#include "typedWritableReferenceCount.h"

TypeHandle TypedWritableReferenceCount::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TypedWritableReferenceCount::as_reference_count
//       Access: Public, Virtual
//  Description: Returns the pointer cast to a ReferenceCount pointer,
//               if it is in fact of that type.
////////////////////////////////////////////////////////////////////
ReferenceCount *TypedWritableReferenceCount::
as_reference_count() {
  return this;
}
