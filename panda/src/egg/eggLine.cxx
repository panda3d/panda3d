/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggLine.cxx
 * @author drose
 * @date 2003-10-14
 */

#include "eggLine.h"

#include "indent.h"

TypeHandle EggLine::_type_handle;

/**
 *
 */
EggLine::
~EggLine() {
  clear();
}

/**
 * Makes a copy of this object.
 */
EggLine *EggLine::
make_copy() const {
  return new EggLine(*this);
}

/**
 * Writes the point to the indicated output stream in Egg format.
 */
void EggLine::
write(std::ostream &out, int indent_level) const {
  write_header(out, indent_level, "<Line>");

  if (has_thick()) {
    indent(out, indent_level + 2)
      << "<Scalar> thick { " << get_thick() << " }\n";
  }

  write_body(out, indent_level + 2);
  indent(out, indent_level) << "}\n";
}

/**
 * Returns the number of initial vertices that are not used in defining any
 * component; the first component is defined by the (n + 1)th vertex, and then
 * a new component at each vertex thereafter.
 */
int EggLine::
get_num_lead_vertices() const {
  return 1;
}
