/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcast.cxx
 * @author drose
 * @date 2001-08-07
 */

#include "dcast.h"
#include "config_express.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>  // for IsBadWritePtr()
#endif

/**
 * This function performs the actual check that the indicated TypedObject
 * pointer is of the intended type.
 */
bool
_dcast_verify(TypeHandle want_handle, size_t want_size,
              const TypedObject *ptr) {
#ifdef DO_DCAST
  if (get_verify_dcast()) {
    if (ptr == nullptr) {
      // This is allowed these days.  It used to be an error, but what the
      // heck.
      return true;
    }
#if defined(_DEBUG) && defined(_WIN32)
    if (IsBadWritePtr((TypedObject *)ptr, want_size)) {
      express_cat->warning()
        << "Attempt to cast invalid pointer to "
        << want_handle << "\n";
      return false;
    }
#endif
    if (!ptr->is_of_type(want_handle)) {
      express_cat->error()
        << "Attempt to cast pointer from " << ptr->get_type()
        << " to " << want_handle << "\n";
      if (ptr->get_type() == TypedObject::get_class_type()) {
        express_cat->error(false)
          << "Perhaps pointer was inadvertently deleted?\n";
      }
      return false;
    }
  }
#endif  // DO_DCAST

  return true;
}
