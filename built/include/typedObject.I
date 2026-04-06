/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file typedObject.I
 * @author drose
 * @date 2001-05-11
 */

/**
 * Returns the internal index number associated with this object's TypeHandle,
 * a unique number for each different type.  This is equivalent to
 * get_type().get_index().
 */
INLINE int TypedObject::
get_type_index() const {
  return get_type().get_index();
}

/**
 * Returns true if the current object is or derives from the indicated type.
 */
INLINE bool TypedObject::
is_of_type(TypeHandle handle) const {
  // Shortcut for the common case where the type matches exactly.
  TypeHandle my_type = get_type();
  return handle == my_type || my_type.is_derived_from(handle, (TypedObject *)this);
}

/**
 * Returns true if the current object is the indicated type exactly.
 */
INLINE bool TypedObject::
is_exact_type(TypeHandle handle) const {
#ifndef NDEBUG
  // Call get_name() to force the type to look itself up if necessary.
  get_type().get_name((TypedObject *)this);
#endif
  return get_type() == handle;
}

/**
 * Returns the object, upcast (if necessary) to a TypedObject pointer.
 */
INLINE TypedObject *TypedObject::
as_typed_object() {
  return this;
}

/**
 * Returns the object, upcast (if necessary) to a TypedObject pointer.
 */
INLINE const TypedObject *TypedObject::
as_typed_object() const {
  return this;
}
