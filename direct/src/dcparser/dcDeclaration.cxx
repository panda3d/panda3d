/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcDeclaration.cxx
 * @author drose
 * @date 2004-06-18
 */

#include "dcDeclaration.h"


/**
 *
 */
DCDeclaration::
~DCDeclaration() {
}

/**
 *
 */
DCClass *DCDeclaration::
as_class() {
  return nullptr;
}

/**
 *
 */
const DCClass *DCDeclaration::
as_class() const {
  return nullptr;
}

/**
 *
 */
DCSwitch *DCDeclaration::
as_switch() {
  return nullptr;
}

/**
 *
 */
const DCSwitch *DCDeclaration::
as_switch() const {
  return nullptr;
}

/**
 * Write a string representation of this instance to <out>.
 */
void DCDeclaration::
output(std::ostream &out) const {
  output(out, true);
}

/**
 * Write a string representation of this instance to <out>.
 */
void DCDeclaration::
write(std::ostream &out, int indent_level) const {
  write(out, false, indent_level);
}
