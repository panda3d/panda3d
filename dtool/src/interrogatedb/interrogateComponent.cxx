/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interrogateComponent.cxx
 * @author drose
 * @date 2000-08-08
 */

#include "interrogateComponent.h"
#include "interrogate_datafile.h"

// This static string is just kept around as a handy bogus return value for
// functions that must return a const string reference.
std::string InterrogateComponent::_empty_string;

/**
 * Formats the component for output to a data file.
 */
void InterrogateComponent::
output(std::ostream &out) const {
  idf_output_string(out, _name);
  out << _alt_names.size() << " ";

  Strings::const_iterator vi;
  for (vi = _alt_names.begin(); vi != _alt_names.end(); ++vi) {
    idf_output_string(out, *vi);
  }
}

/**
 * Reads the data file as previously formatted by output().
 */
void InterrogateComponent::
input(std::istream &in) {
  idf_input_string(in, _name);

  int num_alt_names;
  in >> num_alt_names;
  _alt_names.reserve(num_alt_names);
  for (int i = 0; i < num_alt_names; ++i) {
    std::string alt_name;
    idf_input_string(in, alt_name);
    _alt_names.push_back(alt_name);
  }
}
