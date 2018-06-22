/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interrogateFunction.cxx
 * @author drose
 * @date 2000-08-01
 */

#include "interrogateFunction.h"
#include "indexRemapper.h"
#include "interrogate_datafile.h"
#include "interrogateDatabase.h"

/**
 *
 */
InterrogateFunction::
InterrogateFunction(InterrogateModuleDef *def) :
  InterrogateComponent(def)
{
  _flags = 0;
  _class = 0;
  _instances = nullptr;
}

/**
 *
 */
InterrogateFunction::
InterrogateFunction(const InterrogateFunction &copy) {
  (*this) = copy;
}

/**
 *
 */
void InterrogateFunction::
operator = (const InterrogateFunction &copy) {
  InterrogateComponent::operator = (copy);
  _flags = copy._flags;
  _scoped_name = copy._scoped_name;
  _comment = copy._comment;
  _prototype = copy._prototype;
  _class = copy._class;
  _c_wrappers = copy._c_wrappers;
  _python_wrappers = copy._python_wrappers;

  _instances = copy._instances;
  _expression = copy._expression;
}

/**
 * Formats the InterrogateFunction data for output to a data file.
 */
void InterrogateFunction::
output(std::ostream &out) const {
  InterrogateComponent::output(out);
  out << _flags << " "
      << _class << " ";
  idf_output_string(out, _scoped_name);
  idf_output_vector(out, _c_wrappers);
  idf_output_vector(out, _python_wrappers);
  idf_output_string(out, _comment, '\n');
  idf_output_string(out, _prototype, '\n');
}

/**
 * Reads the data file as previously formatted by output().
 */
void InterrogateFunction::
input(std::istream &in) {
  InterrogateComponent::input(in);
  in >> _flags >> _class;
  idf_input_string(in, _scoped_name);
  idf_input_vector(in, _c_wrappers);
  idf_input_vector(in, _python_wrappers);
  idf_input_string(in, _comment);
  idf_input_string(in, _prototype);
}

/**
 * Remaps all internal index numbers according to the indicated map.  This
 * called from InterrogateDatabase::remap_indices().
 */
void InterrogateFunction::
remap_indices(const IndexRemapper &remap) {
  _class = remap.map_from(_class);
  Wrappers::iterator wi;
  for (wi = _c_wrappers.begin(); wi != _c_wrappers.end(); ++wi) {
    (*wi) = remap.map_from(*wi);
  }
  for (wi = _python_wrappers.begin(); wi != _python_wrappers.end(); ++wi) {
    (*wi) = remap.map_from(*wi);
  }
}
