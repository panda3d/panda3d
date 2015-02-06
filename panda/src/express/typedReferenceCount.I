// Filename: typedReferenceCount.I
// Created by:  drose (25May00)
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
//     Function: TypedReferenceCount::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TypedReferenceCount::
TypedReferenceCount() {
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, this);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: TypedReferenceCount::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TypedReferenceCount::
TypedReferenceCount(const TypedReferenceCount &copy) :
  TypedObject(copy),
  ReferenceCount(copy)
{
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, this);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: TypedReferenceCount::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TypedReferenceCount::
operator = (const TypedReferenceCount &copy) {
  TypedObject::operator = (copy);
  ReferenceCount::operator = (copy);
}

////////////////////////////////////////////////////////////////////
//     Function: TypedReferenceCount::as_typed_object
//       Access: Public
//  Description: If this object derives from TypedReferenceCount or
//               TypedWritableReferenceCount, returns a pointer to
//               the base TypedObject.  Otherwise, returns NULL.
////////////////////////////////////////////////////////////////////
INLINE TypedObject *TypedReferenceCount::
as_typed_object() {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: TypedReferenceCount::as_typed_object
//       Access: Public
//  Description: If this object derives from TypedReferenceCount or
//               TypedWritableReferenceCount, returns a pointer to
//               the base TypedObject.  Otherwise, returns NULL.
////////////////////////////////////////////////////////////////////
INLINE const TypedObject *TypedReferenceCount::
as_typed_object() const {
  return this;
}
