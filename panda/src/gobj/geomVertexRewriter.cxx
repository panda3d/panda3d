/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomVertexRewriter.cxx
 * @author drose
 * @date 2005-03-28
 */

#include "geomVertexRewriter.h"

/**
 *
 */
void GeomVertexRewriter::
output(std::ostream &out) const {
  const GeomVertexColumn *column = get_column();
  if (column == nullptr) {
    out << "GeomVertexRewriter()";

  } else {
    out << "GeomVertexRewriter, array = " << get_array_data()
        << ", column = " << column->get_name()
        << " (" << GeomVertexReader::get_packer()->get_name()
        << "), read row " << get_read_row()
        << ", write row " << get_write_row();
  }
}
