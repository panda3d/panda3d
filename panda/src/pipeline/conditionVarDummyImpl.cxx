/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file conditionVarDummyImpl.cxx
 * @author drose
 * @date 2002-08-09
 */

#include "selectThreadImpl.h"
#include "conditionVarDummyImpl.h"
#include "thread.h"

/**
 *
 */
void ConditionVarDummyImpl::
wait() {
  Thread::force_yield();
}

/**
 *
 */
void ConditionVarDummyImpl::
wait(double) {
  Thread::force_yield();
}
