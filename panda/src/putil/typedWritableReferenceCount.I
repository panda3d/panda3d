// Filename: typedWritableReferenceCount.I
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


////////////////////////////////////////////////////////////////////
//     Function: TypedWritableReferenceCount::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TypedWritableReferenceCount::
TypedWritableReferenceCount() {
  MemoryUsage::update_type(this, this);
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
  MemoryUsage::update_type(this, this);
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
