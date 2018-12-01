/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interrogateMakeSeq.cxx
 * @author drose
 * @date 2009-09-15
 */

#include "interrogateMakeSeq.h"
#include "indexRemapper.h"
#include "interrogate_datafile.h"

/**
 * Formats the InterrogateMakeSeq data for output to a data file.
 */
void InterrogateMakeSeq::
output(std::ostream &out) const {
  InterrogateComponent::output(out);
  out << _length_getter << " "
      << _element_getter << " ";
  idf_output_string(out, _scoped_name);
  idf_output_string(out, _comment, '\n');
}

/**
 * Reads the data file as previously formatted by output().
 */
void InterrogateMakeSeq::
input(std::istream &in) {
  InterrogateComponent::input(in);

  in >> _length_getter >> _element_getter;
  idf_input_string(in, _scoped_name);
  idf_input_string(in, _comment);
}

/**
 * Remaps all internal index numbers according to the indicated map.  This
 * called from InterrogateDatabase::remap_indices().
 */
void InterrogateMakeSeq::
remap_indices(const IndexRemapper &remap) {
  _length_getter = remap.map_from(_length_getter);
  _element_getter = remap.map_from(_element_getter);
}
