// Filename: typedObject.I
// Created by:  drose (11May01)
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
//       Access: Published
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
//       Access: Published
//  Description: Returns true if the current object is or derives from
//               the indicated type.
////////////////////////////////////////////////////////////////////
INLINE bool TypedObject::
is_of_type(TypeHandle handle) const {
  return get_type().is_derived_from(handle, (TypedObject *)this);
}

////////////////////////////////////////////////////////////////////
//     Function: TypedObject::is_exact_type
//       Access: Published
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
//     Function: TypedObject::get_best_parent_from_Set
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int TypedObject::
get_best_parent_from_Set(const std::set<int> &inset) const {
  return get_type().get_best_parent_from_Set(inset);
}

////////////////////////////////////////////////////////////////////
//     Function: TypedObject::as_typed_object
//       Access: Public
//  Description: Returns the object, upcast (if necessary) to a
//               TypedObject pointer.
////////////////////////////////////////////////////////////////////
INLINE TypedObject *TypedObject::
as_typed_object() {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: TypedObject::as_typed_object
//       Access: Public
//  Description: Returns the object, upcast (if necessary) to a
//               TypedObject pointer.
////////////////////////////////////////////////////////////////////
INLINE const TypedObject *TypedObject::
as_typed_object() const {
  return this;
}

