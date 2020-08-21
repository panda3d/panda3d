/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggComment.cxx
 * @author drose
 * @date 1999-01-20
 */

#include "eggComment.h"
#include "eggMiscFuncs.h"

#include "indent.h"
#include "string_utils.h"

TypeHandle EggComment::_type_handle;


/**
 * Writes the comment definition to the indicated output stream in Egg format.
 */
void EggComment::
write(std::ostream &out, int indent_level) const {
  write_header(out, indent_level, "<Comment>");
  enquote_string(out, get_comment(), indent_level + 2) << "\n";
  indent(out, indent_level) << "}\n";
}
