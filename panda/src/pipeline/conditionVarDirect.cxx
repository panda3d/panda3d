/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file conditionVarDirect.cxx
 * @author drose
 * @date 2006-02-13
 */

#include "conditionVarDirect.h"

#ifndef DEBUG_THREADS

/**
 * This method is declared virtual in ConditionVarDebug, but non-virtual in
 * ConditionVarDirect.
 */
void ConditionVarDirect::
output(std::ostream &out) const {
  out << "ConditionVar " << (void *)this << " on " << _mutex;
}

#endif  // !DEBUG_THREADS
