/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgMouseWatcherParameter.cxx
 * @author drose
 * @date 2001-07-05
 */

#include "pgMouseWatcherParameter.h"

TypeHandle PGMouseWatcherParameter::_type_handle;

/**
 *
 */
PGMouseWatcherParameter::
~PGMouseWatcherParameter() {
}

/**
 *
 */
void PGMouseWatcherParameter::
output(std::ostream &out) const {
  MouseWatcherParameter::output(out);
}
