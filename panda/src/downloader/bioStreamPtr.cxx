/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bioStreamPtr.cxx
 * @author drose
 * @date 2002-10-15
 */

#include "bioStreamPtr.h"

#ifdef HAVE_OPENSSL

/**
 *
 */
BioStreamPtr::
~BioStreamPtr() {
  if (_stream != nullptr) {
    delete _stream;
    _stream = nullptr;
  }
}

#endif  // HAVE_OPENSSL
