// Filename: interrogateFunction.cxx
// Created by:  drose (01Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "interrogateFunction.h"
#include "indexRemapper.h"
#include "interrogate_datafile.h"
#include "interrogateDatabase.h"

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunction::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
InterrogateFunction::
InterrogateFunction(InterrogateModuleDef *def) :
  InterrogateComponent(def)
{
  _flags = 0;
  _class = 0;
  _instances = (Instances *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunction::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
InterrogateFunction::
InterrogateFunction(const InterrogateFunction &copy) {
  (*this) = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunction::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunction::output
//       Access: Public
//  Description: Formats the InterrogateFunction data for output to a data
//               file.
////////////////////////////////////////////////////////////////////
void InterrogateFunction::
output(ostream &out) const {
  InterrogateComponent::output(out);
  out << _flags << " "
      << _class << " ";
  idf_output_string(out, _scoped_name);
  idf_output_vector(out, _c_wrappers);
  idf_output_vector(out, _python_wrappers);
  idf_output_string(out, _comment, '\n');
  idf_output_string(out, _prototype, '\n');
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunction::input
//       Access: Public
//  Description: Reads the data file as previously formatted by
//               output().
////////////////////////////////////////////////////////////////////
void InterrogateFunction::
input(istream &in) {
  InterrogateComponent::input(in);
  in >> _flags >> _class;
  idf_input_string(in, _scoped_name);
  idf_input_vector(in, _c_wrappers);
  idf_input_vector(in, _python_wrappers);
  idf_input_string(in, _comment);

  if (InterrogateDatabase::get_file_minor_version() >= 2) {
    idf_input_string(in, _prototype);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunction::remap_indices
//       Access: Public
//  Description: Remaps all internal index numbers according to the
//               indicated map.  This called from
//               InterrogateDatabase::remap_indices().
////////////////////////////////////////////////////////////////////
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
