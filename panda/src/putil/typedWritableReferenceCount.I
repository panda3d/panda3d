// Filename: typedWritableReferenceCount.I
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


////////////////////////////////////////////////////////////////////
//     Function: TypedWritableReferenceCount::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TypedWritableReferenceCount::
TypedWritableReferenceCount() {
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, this);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: TypedWritableReferenceCount::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TypedWritableReferenceCount::
TypedWritableReferenceCount(const TypedWritableReferenceCount &copy) :
  TypedWritable(copy),
  ReferenceCount(copy)
{
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, this);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: TypedWritableReferenceCount::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TypedWritableReferenceCount::
operator = (const TypedWritableReferenceCount &copy) {
  TypedWritable::operator = (copy);
  ReferenceCount::operator = (copy);
}

////////////////////////////////////////////////////////////////////
//     Function: TypedWritableReferenceCount::as_typed_object
//       Access: Public
//  Description: If this object derives from TypedReferenceCount or
//               TypedWritableReferenceCount, returns a pointer to
//               the base TypedObject.  Otherwise, returns NULL.
////////////////////////////////////////////////////////////////////
INLINE TypedObject *TypedWritableReferenceCount::
as_typed_object() {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: TypedWritableReferenceCount::as_typed_object
//       Access: Public
//  Description: If this object derives from TypedReferenceCount or
//               TypedWritableReferenceCount, returns a pointer to
//               the base TypedObject.  Otherwise, returns NULL.
////////////////////////////////////////////////////////////////////
INLINE const TypedObject *TypedWritableReferenceCount::
as_typed_object() const {
  return this;
}
