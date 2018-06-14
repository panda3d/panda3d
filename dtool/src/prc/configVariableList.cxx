/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariableList.cxx
 * @author drose
 * @date 2004-10-20
 */

#include "configVariableList.h"

/**
 *
 */
void ConfigVariableList::
output(std::ostream &out) const {
  out << get_num_values() << " values.";
}

/**
 *
 */
void ConfigVariableList::
write(std::ostream &out) const {
  size_t num_values = get_num_values();
  for (size_t i = 0; i < num_values; ++i) {
    out << get_string_value(i) << "\n";
  }
}
