/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightReMutexDirect.cxx
 * @author drose
 * @date 2008-10-08
 */

#include "lightReMutexDirect.h"
#include "thread.h"

#ifndef DEBUG_THREADS

/**
 * This method is declared virtual in MutexDebug, but non-virtual in
 * LightReMutexDirect.
 */
void LightReMutexDirect::
output(std::ostream &out) const {
  out << "LightReMutex " << (void *)this;
}

#endif  // !DEBUG_THREADS
