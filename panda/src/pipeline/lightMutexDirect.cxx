/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightMutexDirect.cxx
 * @author drose
 * @date 2008-10-08
 */

#include "lightMutexDirect.h"

#ifndef DEBUG_THREADS

/**
 * This method is declared virtual in LightMutexDebug, but non-virtual in
 * LightMutexDirect.
 */
void LightMutexDirect::
output(std::ostream &out) const {
  out << "LightMutex " << (void *)this;
}

#endif  // !DEBUG_THREADS
