/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file typedWritableReferenceCount.cxx
 * @author jason
 * @date 2000-06-08
 */

#include "typedWritableReferenceCount.h"
#include "dcast.h"

TypeHandle TypedWritableReferenceCount::_type_handle;

/**
 * Returns the pointer cast to a ReferenceCount pointer, if it is in fact of
 * that type.
 */
ReferenceCount *TypedWritableReferenceCount::
as_reference_count() {
  return this;
}

/**
 * Reads the bytes created by a previous call to encode_to_bam_stream(), and
 * extracts and returns the single object on those bytes.  Returns NULL on
 * error.
 *
 * This method is intended to replace decode_raw_from_bam_stream() when you
 * know the stream in question returns an object of type
 * TypedWritableReferenceCount, allowing for easier reference count
 * management.  Note that the caller is still responsible for maintaining the
 * reference count on the return value.
 */
PT(TypedWritableReferenceCount) TypedWritableReferenceCount::
decode_from_bam_stream(vector_uchar data, BamReader *reader) {
  TypedWritable *object;
  ReferenceCount *ref_ptr;

  if (TypedWritable::decode_raw_from_bam_stream(object, ref_ptr, std::move(data), reader)) {
    return DCAST(TypedWritableReferenceCount, object);
  } else {
    return nullptr;
  }
}
