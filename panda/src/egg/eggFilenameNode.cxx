/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggFilenameNode.cxx
 * @author drose
 * @date 1999-02-11
 */

#include "eggFilenameNode.h"

TypeHandle EggFilenameNode::_type_handle;

/**
 * Returns the default extension for this filename type.
 */
std::string EggFilenameNode::
get_default_extension() const {
  return std::string();
}
