/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file binCullHandler.cxx
 * @author drose
 * @date 2002-02-28
 */

#include "binCullHandler.h"
#include "pStatTimer.h"

/**
 * This callback function is intended to be overridden by a derived class.
 * This is called as each Geom is discovered by the CullTraverser.
 */
void BinCullHandler::
record_object(CullableObject &&object, const CullTraverser *traverser) {
  _cull_result->add_object(std::move(object), traverser);
}
