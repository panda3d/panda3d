/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file typeHandle.I
 * @author drose
 * @date 2000-02-22
 */

/**
 *
 */
INLINE bool TypeHandle::
operator == (const TypeHandle &other) const {
  return (_index == other._index);
}

/**
 *
 */
INLINE bool TypeHandle::
operator != (const TypeHandle &other) const {
  return (_index != other._index);
}

/**
 *
 */
INLINE bool TypeHandle::
operator < (const TypeHandle &other) const {
  return (_index < other._index);
}

/**
 *
 */
INLINE bool TypeHandle::
operator <= (const TypeHandle &other) const {
  return (_index <= other._index);
}

/**
 *
 */
INLINE bool TypeHandle::
operator > (const TypeHandle &other) const {
  return (_index > other._index);
}

/**
 *
 */
INLINE bool TypeHandle::
operator >= (const TypeHandle &other) const {
  return (_index >= other._index);
}

/**
 * Sorts TypeHandles arbitrarily (according to <, >, etc.).  Returns a number
 * less than 0 if this type sorts before the other one, greater than zero if
 * it sorts after, 0 if they are equivalent.
 */
INLINE int TypeHandle::
compare_to(const TypeHandle &other) const {
  return _index - other._index;
}

/**
 * Returns a hash code suitable for phash_map.
 */
INLINE size_t TypeHandle::
get_hash() const {
  return (size_t)_index;
}

/**
 * Returns the name of the type.
 *
 * The "object" pointer is an optional pointer to the TypedObject class that
 * owns this TypeHandle.  It is only used in case the TypeHandle is
 * inadvertantly undefined.
 */
INLINE std::string TypeHandle::
get_name(TypedObject *object) const {
  if ((*this) == TypeHandle::none()) {
    return "none";
  } else {
    return TypeRegistry::ptr()->get_name(*this, object);
  }
}

/**
 * Returns true if this type is derived from the indicated type, false
 * otherwise.
 *
 * The "object" pointer is an optional pointer to the TypedObject class that
 * owns this TypeHandle.  It is only used in case the TypeHandle is
 * inadvertantly undefined.
 */
INLINE bool TypeHandle::
is_derived_from(TypeHandle parent, TypedObject *object) const {
  return TypeRegistry::ptr()->is_derived_from(*this, parent, object);
}

/**
 * Returns the number of parent classes that this type is known to have.  This
 * may then be used to index into get_parent_class().  The result will be 0 if
 * this class does not inherit from any other classes, 1 if normal, single
 * inheritance is in effect, or greater than one if multiple inheritance is in
 * effect.
 *
 * The "object" pointer is an optional pointer to the TypedObject class that
 * owns this TypeHandle.  It is only used in case the TypeHandle is
 * inadvertantly undefined.
 */
INLINE int TypeHandle::
get_num_parent_classes(TypedObject *object) const {
  return TypeRegistry::ptr()->get_num_parent_classes(*this, object);
}

/**
 * Returns the nth parent class of this type.  The index should be in the
 * range 0 <= index < get_num_parent_classes().
 */
INLINE TypeHandle TypeHandle::
get_parent_class(int index) const {
  return TypeRegistry::ptr()->get_parent_class(*this, index);
}

/**
 * Returns the number of child classes that this type is known to have.  This
 * may then be used to index into get_child_class().
 *
 * The "object" pointer is an optional pointer to the TypedObject class that
 * owns this TypeHandle.  It is only used in case the TypeHandle is
 * inadvertantly undefined.
 */
INLINE int TypeHandle::
get_num_child_classes(TypedObject *object) const {
  return TypeRegistry::ptr()->get_num_child_classes(*this, object);
}

/**
 * Returns the nth child class of this type.  The index should be in the range
 * 0 <= index < get_num_child_classes().
 */
INLINE TypeHandle TypeHandle::
get_child_class(int index) const {
  return TypeRegistry::ptr()->get_child_class(*this, index);
}

/**
 * Returns the parent class that is in a direct line of inheritance to the
 * indicated ancestor class.  This is useful in the presence of multiple
 * inheritance to try to determine what properties an unknown type may have.
 *
 * The return value is TypeHandle::none() if the type does not inherit from
 * the ancestor.  If ancestor is the same as this type, the return value is
 * this type.
 *
 * The "object" pointer is an optional pointer to the TypedObject class that
 * owns this TypeHandle.  It is only used in case the TypeHandle is
 * inadvertantly undefined.
 */
INLINE TypeHandle TypeHandle::
get_parent_towards(TypeHandle ancestor, TypedObject *object) const {
  return TypeRegistry::ptr()->get_parent_towards(*this, ancestor, object);
}

/**
 * Returns the integer index associated with this TypeHandle.  Each different
 * TypeHandle will have a different index.  However, you probably shouldn't be
 * using this method; you should just treat the TypeHandles as opaque classes.
 * This is provided for the convenience of non-C++ scripting languages to
 * build a hashtable of TypeHandles.
 */
INLINE int TypeHandle::
get_index() const {
  return _index;
}

/**
 *
 */
INLINE void TypeHandle::
output(std::ostream &out) const {
  out << get_name();
}

/**
 * TypeHandle::none() evaluates to false, everything else evaluates to true.
 */
INLINE TypeHandle::
operator bool () const {
  return (_index != 0);
}

/**
 * Private constructor for initializing a TypeHandle from an index, used by
 * none() and by from_index().
 */
constexpr TypeHandle::
TypeHandle(int index) : _index(index) {
}
