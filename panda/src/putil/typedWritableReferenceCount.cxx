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
#include "dcast.h"

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

////////////////////////////////////////////////////////////////////
//     Function: TypedWritableReferenceCount::decode_from_bam_stream
//       Access: Published, Static
//  Description: Reads the string created by a previous call to
//               encode_to_bam_stream(), and extracts and returns the
//               single object on that string.  Returns NULL on error.
//
//               This method is intended to replace
//               decode_raw_from_bam_stream() when you know the stream
//               in question returns an object of type
//               TypedWritableReferenceCount, allowing for easier
//               reference count management.  Note that the caller is
//               still responsible for maintaining the reference count
//               on the return value.
////////////////////////////////////////////////////////////////////
PT(TypedWritableReferenceCount) TypedWritableReferenceCount::
decode_from_bam_stream(const string &data, BamReader *reader) {
  TypedWritable *object;
  ReferenceCount *ref_ptr;

  if (!TypedWritable::decode_raw_from_bam_stream(object, ref_ptr, data, reader)) {
    return NULL;
  }

  return DCAST(TypedWritableReferenceCount, object);
}
