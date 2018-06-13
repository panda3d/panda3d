/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interrogateManifest.cxx
 * @author drose
 * @date 2000-08-11
 */

#include "interrogateManifest.h"
#include "indexRemapper.h"
#include "interrogate_datafile.h"

/**
 * Formats the InterrogateManifest data for output to a data file.
 */
void InterrogateManifest::
output(std::ostream &out) const {
  InterrogateComponent::output(out);
  out << _flags << " "
      << _int_value << " "
      << _type << " "
      << _getter << " ";
  idf_output_string(out, _definition);
}

/**
 * Reads the data file as previously formatted by output().
 */
void InterrogateManifest::
input(std::istream &in) {
  InterrogateComponent::input(in);
  in >> _flags >> _int_value >> _type >> _getter;
  idf_input_string(in, _definition);
}

/**
 * Remaps all internal index numbers according to the indicated map.  This
 * called from InterrogateDatabase::remap_indices().
 */
void InterrogateManifest::
remap_indices(const IndexRemapper &remap) {
  _type = remap.map_from(_type);
  _getter = remap.map_from(_getter);
}
