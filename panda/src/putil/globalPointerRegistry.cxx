/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file globalPointerRegistry.cxx
 * @author drose
 * @date 2000-02-03
 */

#include "globalPointerRegistry.h"
#include "config_putil.h"

// In general, we use the util_cat->info() syntax in this file (instead of
// util_cat.info()), because much of this work is done at static init time,
// and we must use the arrow syntax to force initialization of the util_cat
// category.

GlobalPointerRegistry *GlobalPointerRegistry::_global_ptr;

/**
 * Returns the pointer associated with the indicated TypeHandle, if any.  If
 * no pointer has yet been associated, returns NULL.
 */
void *GlobalPointerRegistry::
ns_get_pointer(TypeHandle type) const {
  if (type == TypeHandle::none()) {
    util_cat->error()
      << "GlobalPointerRegistry::get_pointer() called on empty TypeHandle\n";
  }
  Pointers::const_iterator pi;
  pi = _pointers.find(type);
  if (pi == _pointers.end()) {
    return nullptr;
  }

  return (*pi).second;
}

/**
 * Associates the given pointer with the indicated TypeHandle.  It is an error
 * to call this with a NULL pointer, or to call this function more than once
 * with a given TypeHandle (without first calling clear_pointer).
 */
void GlobalPointerRegistry::
ns_store_pointer(TypeHandle type, void *ptr) {
  if (type == TypeHandle::none()) {
    util_cat->error()
      << "GlobalPointerRegistry::store_pointer() called on empty TypeHandle\n";
  }
  if (ptr == nullptr) {
    util_cat->error()
      << "Invalid attempt to store a NULL pointer for " << type << "\n";
    clear_pointer(type);
    return;
  }
  std::pair<Pointers::iterator, bool> result =
    _pointers.insert(Pointers::value_type(type, ptr));

  if (!result.second) {
    // There was already a pointer in the map.
    if ((*result.first).second == ptr) {
      util_cat->error()
        << "Invalid attempt to store pointer " << ptr
        << " twice for " << type << "\n";
    } else {
      util_cat->error()
        << "Invalid attempt to store additional pointer " << ptr
        << " for " << type << "; " << (*result.first).second
        << " stored previously.\n";
    }
  }
}

/**
 * Removes the association of the given pointer with the indicated TypeHandle.
 * Subsequent calls to get_pointer() with this TypeHandle will return NULL,
 * until another call to store_pointer() is made.
 */
void GlobalPointerRegistry::
ns_clear_pointer(TypeHandle type) {
  if (type == TypeHandle::none()) {
    util_cat->error()
      << "GlobalPointerRegistry::clear_pointer() called on empty TypeHandle\n";
  }

  // It's not an error to clear_pointer() if it was already cleared.  Don't
  // bother checking that.
  _pointers.erase(type);
}
