/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file memoryUsagePointers.cxx
 * @author drose
 * @date 2000-05-25
 */

#include "memoryUsagePointers.h"

#ifdef DO_MEMORY_USAGE

#include "config_express.h"
#include "referenceCount.h"
#include "typedReferenceCount.h"

/**

 */
MemoryUsagePointers::
MemoryUsagePointers() {
}

/**

 */
MemoryUsagePointers::
~MemoryUsagePointers() {
}

/**
 * Returns the number of pointers in the set.
 */
size_t MemoryUsagePointers::
get_num_pointers() const {
  return _entries.size();
}

/**
 * Returns the nth pointer of the set.
 */
ReferenceCount *MemoryUsagePointers::
get_pointer(size_t n) const {
  nassertr(n < get_num_pointers(), NULL);
  return _entries[n]._ref_ptr;
}

/**
 * Returns the nth pointer of the set, typecast to a TypedObject if possible.
 * If the pointer is not a TypedObject or if the cast cannot be made, returns
 * NULL.
 */
TypedObject *MemoryUsagePointers::
get_typed_pointer(size_t n) const {
  nassertr(n < get_num_pointers(), NULL);
  TypedObject *typed_ptr = _entries[n]._typed_ptr;

  if (typed_ptr != (TypedObject *)NULL) {
    return typed_ptr;
  }

  ReferenceCount *ref_ptr = _entries[n]._ref_ptr;

  TypeHandle type = _entries[n]._type;

  // We can only cast-across to a TypedObject when we explicitly know
  // the inheritance path.  Most of the time, this will be via
  // TypedReferenceCount.  There are classes defined in other packages
  // that inherit from TypedObject and ReferenceCount separately (like
  // Node), but we can't do anything about that here without knowing
  // about the particular class.  (Actually, we couldn't do anything
  // about Node anyway, because it inherits virtually from
  // ReferenceCount.)

  // RTTI can't help us here, because ReferenceCount has no virtual
  // functions, so we can't use C++'s new dynamic_cast feature.

  if (type != TypeHandle::none() &&
      type.is_derived_from(TypedReferenceCount::get_class_type())) {
    return (TypedReferenceCount *)ref_ptr;
  }
  return NULL;
}

/**
 * Returns the actual type of the nth pointer, if it is known.
 */
TypeHandle MemoryUsagePointers::
get_type(size_t n) const {
  nassertr(n < get_num_pointers(), TypeHandle::none());
  return _entries[n]._type;
}

/**
 * Returns the type name of the nth pointer, if it is known.
 */
string MemoryUsagePointers::
get_type_name(size_t n) const {
  nassertr(n < get_num_pointers(), "");
  return get_type(n).get_name();
}

/**
 * Returns the age of the nth pointer: the number of seconds elapsed between the
 * time it was allocated and the time it was added to this set via a call to
 * MemoryUsage::get_pointers().
 */
double MemoryUsagePointers::
get_age(size_t n) const {
  nassertr(n < get_num_pointers(), 0.0);
  return _entries[n]._age;
}

/**
 * Empties the set of pointers.
 */
void MemoryUsagePointers::
clear() {
  _entries.clear();
}

/**

 */
void MemoryUsagePointers::
output(ostream &out) const {
  out << _entries.size() << " pointers.";
}

/**
 * Adds a new entry to the set.  Intended to be called only by MemoryUsage.
 */
void MemoryUsagePointers::
add_entry(ReferenceCount *ref_ptr, TypedObject *typed_ptr,
          TypeHandle type, double age) {
  // We can't safely add pointers with a zero reference count.  They
  // might be statically-allocated or something, and if we try to add
  // them they'll try to destruct when the PointerTo later goes away.
  if (ref_ptr->get_ref_count() != 0) {
    _entries.push_back(Entry(ref_ptr, typed_ptr, type, age));
  }
}


#endif  // DO_MEMORY_USAGE
