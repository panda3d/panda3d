// Filename: typedWritableReferenceCount.I
// Created by:  jason (08Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
  ReferenceCount(copy) {
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
