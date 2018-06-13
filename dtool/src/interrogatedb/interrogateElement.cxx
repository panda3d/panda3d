/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interrogateElement.cxx
 * @author drose
 * @date 2000-08-11
 */

#include "interrogateElement.h"
#include "interrogateDatabase.h"
#include "indexRemapper.h"
#include "interrogate_datafile.h"

/**
 * Formats the InterrogateElement data for output to a data file.
 */
void InterrogateElement::
output(std::ostream &out) const {
  InterrogateComponent::output(out);
  out << _flags << " "
      << _type << " "
      << _getter << " "
      << _setter << " "
      << _has_function << " "
      << _clear_function << " "
      << _del_function << " "
      << _length_function << " "
      << _insert_function << " "
      << _getkey_function << " ";
  idf_output_string(out, _scoped_name);
  idf_output_string(out, _comment, '\n');
}

/**
 * Reads the data file as previously formatted by output().
 */
void InterrogateElement::
input(std::istream &in) {
  InterrogateComponent::input(in);
  in >> _flags >> _type >> _getter >> _setter;
  if (InterrogateDatabase::get_file_minor_version() >= 1) {
    in >> _has_function >> _clear_function;
    if (InterrogateDatabase::get_file_minor_version() >= 2) {
      in >> _del_function >> _length_function;
      if (InterrogateDatabase::get_file_minor_version() >= 3) {
        in >> _insert_function >> _getkey_function;
      }
    }
  }
  idf_input_string(in, _scoped_name);
  idf_input_string(in, _comment);
}

/**
 * Remaps all internal index numbers according to the indicated map.  This
 * called from InterrogateDatabase::remap_indices().
 */
void InterrogateElement::
remap_indices(const IndexRemapper &remap) {
  _type = remap.map_from(_type);
  _getter = remap.map_from(_getter);
  _setter = remap.map_from(_setter);
  _has_function = remap.map_from(_has_function);
  _clear_function = remap.map_from(_clear_function);
  _del_function = remap.map_from(_del_function);
  _insert_function = remap.map_from(_insert_function);
  _getkey_function = remap.map_from(_getkey_function);
  _length_function = remap.map_from(_length_function);
}
