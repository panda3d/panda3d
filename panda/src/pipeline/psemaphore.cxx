/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file psemaphore.cxx
 * @author drose
 * @date 2008-10-13
 */

#include "psemaphore.h"

/**
 *
 */
void Semaphore::
output(std::ostream &out) const {
  MutexHolder holder(_lock);
  out << "Semaphore, count = " << _count;
}
