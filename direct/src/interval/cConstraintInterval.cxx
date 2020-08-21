/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cConstraintInterval.cxx
 * @author pratt
 * @date 2006-09-29
 */

#include "cConstraintInterval.h"

TypeHandle CConstraintInterval::_type_handle;

/**
 *
 */
CConstraintInterval::
CConstraintInterval(const std::string &name, double duration) :
  CInterval(name, duration, true)
{
}
