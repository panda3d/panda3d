/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggPatch.cxx
 * @author drose
 * @date 2012-04-27
 */

#include "eggPatch.h"
#include "eggGroupNode.h"
#include "plane.h"

#include "indent.h"

#include <algorithm>

TypeHandle EggPatch::_type_handle;

/**
 * Makes a copy of this object.
 */
EggPatch *EggPatch::
make_copy() const {
  return new EggPatch(*this);
}

/**
 * Writes the patch to the indicated output stream in Egg format.
 */
void EggPatch::
write(std::ostream &out, int indent_level) const {
  write_header(out, indent_level, "<Patch>");
  write_body(out, indent_level+2);
  indent(out, indent_level) << "}\n";
}
