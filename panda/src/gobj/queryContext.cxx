/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file queryContext.cxx
 * @author drose
 * @date 2006-03-27
 */

#include "queryContext.h"

TypeHandle QueryContext::_type_handle;

/**

 */
QueryContext::
~QueryContext() {
}

/**
 * Returns true if the query's answer is ready, false otherwise.  If this
 * returns false, the application must continue to poll until it returns true.
 * It is only valid to call this from the draw thread.
 */
bool QueryContext::
is_answer_ready() const {
  return false;
}

/**
 * Requests the graphics engine to expedite the pending answer--the application
 * is now waiting until the answer is ready.  It is only valid to call this from
 * the draw thread.
 */
void QueryContext::
waiting_for_answer() {
}
