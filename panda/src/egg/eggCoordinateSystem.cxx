/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggCoordinateSystem.cxx
 * @author drose
 * @date 1999-01-20
 */

#include "eggCoordinateSystem.h"

#include "indent.h"

TypeHandle EggCoordinateSystem::_type_handle;


/**
 * Writes the coordinate system definition to the indicated output stream in
 * Egg format.
 */
void EggCoordinateSystem::
write(std::ostream &out, int indent_level) const {
  if (get_value() != CS_default &&
      get_value() != CS_invalid) {
    indent(out, indent_level)
      << "<CoordinateSystem> { ";
    switch (get_value()) {
    case CS_zup_right:
      out << "Z-Up";
      break;

    case CS_yup_right:
      out << "Y-Up";
      break;

    case CS_zup_left:
      out << "Z-Up-Left";
      break;

    case CS_yup_left:
      out << "Y-Up-Left";
      break;

    default:
      out << "/* Invalid coordinate system " << (int)get_value() << " */";
    }
    out << " }\n\n";
  }
}
