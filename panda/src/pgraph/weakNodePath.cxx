/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file weakNodePath.cxx
 * @author drose
 * @date 2004-09-29
 */

#include "weakNodePath.h"

/**
 *
 */
void WeakNodePath::
output(std::ostream &out) const {
  if (was_deleted()) {
    out << "deleted";
  } else {
    get_node_path().output(out);
  }
}
