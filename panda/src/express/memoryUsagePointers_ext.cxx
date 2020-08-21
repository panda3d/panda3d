/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file memoryUsagePointers_ext.cxx
 * @author rdb
 * @date 2013-12-10
 */

#include "memoryUsagePointers_ext.h"

#if defined(HAVE_PYTHON) && defined(DO_MEMORY_USAGE)

#ifndef CPPPARSER
extern Dtool_PyTypedObject Dtool_TypedObject;
extern Dtool_PyTypedObject Dtool_TypedReferenceCount;
extern Dtool_PyTypedObject Dtool_ReferenceCount;
#endif  // CPPPARSER

/**
 * Returns the nth object, represented as a Python object of the appropriate
 * type.  Reference counting will be properly set on the Python object.
 *
 * get_typed_pointer() is almost as good as this, but (a) it does not set the
 * reference count, and (b) it does not work for objects that do not inherit
 * from TypedObject.  This will work for any object whose type is known, which
 * has a Python representation.
 */
PyObject *Extension<MemoryUsagePointers>::
get_python_pointer(size_t n) const {
  TypedObject *typed_ptr = _this->get_typed_pointer(n);
  ReferenceCount *ref_ptr = _this->get_pointer(n);

  bool memory_rules = false;
  if (ref_ptr != nullptr) {
    memory_rules = true;
    ref_ptr->ref();
  }

  if (typed_ptr != nullptr) {
    return DTool_CreatePyInstanceTyped(typed_ptr, Dtool_TypedObject,
                                       memory_rules, false,
                                       typed_ptr->get_type_index());
  }

  if (ref_ptr == nullptr) {
    return Py_BuildValue("");
  }

  TypeHandle type = _this->get_type(n);
  if (type != TypeHandle::none()) {
    // Trust that there is a downcast path to the actual type.
    return DTool_CreatePyInstanceTyped(ref_ptr, Dtool_ReferenceCount,
                                       memory_rules, false,
                                       type.get_index());
  }

  // If worse comes to worst, just return a ReferenceCount wrapper.
  return DTool_CreatePyInstance(ref_ptr, Dtool_ReferenceCount,
                                memory_rules, false);
}

#endif  // HAVE_PYTHON && DO_MEMORY_USAGE
