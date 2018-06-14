/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggSAnimData.cxx
 * @author drose
 * @date 1999-02-19
 */

#include "eggSAnimData.h"
#include "eggMiscFuncs.h"
#include "eggParameters.h"

#include "indent.h"

#include <math.h>

TypeHandle EggSAnimData::_type_handle;

/**
 * Optimizes the data by collapsing a long table of duplicate values into a
 * single value.
 */
void EggSAnimData::
optimize() {
  if (get_num_rows() > 1) {
    double value = get_value(0);
    for (int row = 1; row < get_num_rows(); row++) {
      if (fabs(get_value(row) - value) > egg_parameters->_table_threshold) {
        return;
      }
    }

    // Ok, all the rows had the same value.  Collapse them.

    // We have to use the call to v() to work around an apparent compiler bug
    // with Win64.
    _data.v().erase(_data.v().begin() + 1, _data.v().end());
  }
}


/**
 * Writes the data to the indicated output stream in Egg format.
 */
void EggSAnimData::
write(std::ostream &out, int indent_level) const {
  if (get_num_rows() <= 1) {
    // We get a lot of these little tiny tables.  For brevity, we'll write
    // these all on one line, because we can.  This just makes it easier for a
    // human to scan the egg file.

    indent(out, indent_level) << "<S$Anim> ";
    if (has_name()) {
      enquote_string(out, get_name()) << " {";
    } else {
      out << "{";
    }

    if (has_fps()) {
      out << " <Scalar> fps { " << get_fps() << " }";
    }

    if (get_num_rows() == 1) {
      out << " <V> { " << get_value(0) << " }";
    } else {
      out << " <V> { }";
    }

    out << " }\n";

  } else {
    // If there are at least two values in the table, we'll write it out over
    // multiple lines.

    write_header(out, indent_level, "<S$Anim>");

    if (has_fps()) {
      indent(out, indent_level + 2)
        << "<Scalar> fps { " << get_fps() << " }\n";
    }
    indent(out, indent_level + 2) << "<V> {\n";
    write_long_list(out, indent_level + 4, _data.begin(), _data.end(),
        "", "", 72);
    indent(out, indent_level + 2) << "}\n";
    indent(out, indent_level) << "}\n";
  }
}
