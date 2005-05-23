// Filename: typedObject.I
// Created by:  drose (11May01)
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
//     Function: TypedObject::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TypedObject::
TypedObject() {
}

////////////////////////////////////////////////////////////////////
//     Function: TypedObject::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TypedObject::
TypedObject(const TypedObject &) {
}

////////////////////////////////////////////////////////////////////
//     Function: TypedObject::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TypedObject::
operator = (const TypedObject &) {
}

////////////////////////////////////////////////////////////////////
//     Function: TypedObject::get_type_index
//       Access: Public
//  Description: Returns the internal index number associated with
//               this object's TypeHandle, a unique number for each
//               different type.  This is equivalent to
//               get_type().get_index().
////////////////////////////////////////////////////////////////////
INLINE int TypedObject::
get_type_index() const {
  return get_type().get_index();
}

////////////////////////////////////////////////////////////////////
//     Function: TypedObject::is_of_type
//       Access: Public
//  Description: Returns true if the current object is or derives from
//               the indicated type.
////////////////////////////////////////////////////////////////////
INLINE bool TypedObject::
is_of_type(TypeHandle handle) const {
  return get_type().is_derived_from(handle, (TypedObject *)this);
}

////////////////////////////////////////////////////////////////////
//     Function: TypedObject::is_exact_type
//       Access: Public
//  Description: Returns true if the current object is the indicated
//               type exactly.
////////////////////////////////////////////////////////////////////
INLINE bool TypedObject::
is_exact_type(TypeHandle handle) const {
#ifndef NDEBUG
  // Call get_name() to force the type to look itself up if necessary.
  get_type().get_name((TypedObject *)this);
#endif
  return get_type() == handle;
}

////////////////////////////////////////////////////////////////////
//     Function: TypedObject::is_exact_type
//       Access: Public
//  Description: Returns true if the current object is the indicated
//               type exactly.
////////////////////////////////////////////////////////////////////
INLINE int TypedObject::
get_best_parent_from_Set(const std::set<int> &inset) const
{
    return get_type().get_best_parent_from_Set(inset);
}
