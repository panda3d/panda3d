/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file auxSceneData.cxx
 * @author drose
 * @date 2004-09-27
 */

#include "auxSceneData.h"
#include "indent.h"

TypeHandle AuxSceneData::_type_handle;

/**
 *
 */
void AuxSceneData::
output(std::ostream &out) const {
  out << get_type() << " expires " << get_expiration_time();
}

/**
 *
 */
void AuxSceneData::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << "\n";
}
