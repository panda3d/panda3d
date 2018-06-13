/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggExternalReference.cxx
 * @author drose
 * @date 1999-02-11
 */

#include "eggExternalReference.h"
#include "eggMiscFuncs.h"

#include "indent.h"
#include "string_utils.h"

TypeHandle EggExternalReference::_type_handle;


/**
 *
 */
EggExternalReference::
EggExternalReference(const std::string &node_name, const std::string &filename)
  : EggFilenameNode(node_name, filename) {
}

/**
 *
 */
EggExternalReference::
EggExternalReference(const EggExternalReference &copy)
  : EggFilenameNode(copy) {
}

/**
 *
 */
EggExternalReference &EggExternalReference::
operator = (const EggExternalReference &copy) {
  EggFilenameNode::operator = (copy);
  return *this;
}

/**
 * Writes the reference to the indicated output stream in Egg format.
 */
void EggExternalReference::
write(std::ostream &out, int indent_level) const {
  write_header(out, indent_level, "<File>");
  enquote_string(out, get_filename(), indent_level + 2) << "\n";
  indent(out, indent_level) << "}\n";
}

/**
 * Returns the default extension for this filename type.
 */
std::string EggExternalReference::
get_default_extension() const {
  return std::string("egg");
}
