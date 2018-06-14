/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file savedContext.cxx
 * @author drose
 * @date 2001-06-11
 */

#include "savedContext.h"
#include "indent.h"

TypeHandle SavedContext::_type_handle;

/**
 *
 */
void SavedContext::
output(std::ostream &out) const {
  out << "SavedContext " << this;
}

/**
 *
 */
void SavedContext::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << "\n";
}
