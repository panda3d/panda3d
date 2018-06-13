/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file conditionVarFullDirect.cxx
 * @author drose
 * @date 2006-08-28
 */

#include "conditionVarFullDirect.h"

#ifndef DEBUG_THREADS

/**
 * This method is declared virtual in ConditionVarFullDebug, but non-virtual
 * in ConditionVarFullDirect.
 */
void ConditionVarFullDirect::
output(std::ostream &out) const {
  out << "ConditionVarFull " << (void *)this << " on " << _mutex;
}

#endif  // !DEBUG_THREADS
