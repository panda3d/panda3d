// Filename: interrogateFunctionWrapper.cxx
// Created by:  drose (06Aug00)
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

#include "interrogateFunctionWrapper.h"
#include "indexRemapper.h"
#include "interrogate_datafile.h"

#include <algorithm>

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunctionWrapper::Parameter::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void InterrogateFunctionWrapper::Parameter::
output(ostream &out) const {
  idf_output_string(out, _name);
  out << _parameter_flags << " " << _type << " ";
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunctionWrapper::Parameter::input
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void InterrogateFunctionWrapper::Parameter::
input(istream &in) {
  idf_input_string(in, _name);
  in >> _parameter_flags >> _type;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunctionWrapper::output
//       Access: Public
//  Description: Formats the InterrogateFunctionWrapper data for
//               output to a data file.
////////////////////////////////////////////////////////////////////
void InterrogateFunctionWrapper::
output(ostream &out) const {
  InterrogateComponent::output(out);
  out << _flags << " "
      << _function << " "
      << _return_type << " "
      << _return_value_destructor << " ";
  idf_output_string(out, _unique_name);
  idf_output_string(out, _comment);
  idf_output_vector(out, _parameters);
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunctionWrapper::input
//       Access: Public
//  Description: Reads the data file as previously formatted by
//               output().
////////////////////////////////////////////////////////////////////
void InterrogateFunctionWrapper::
input(istream &in) {
  InterrogateComponent::input(in);
  in >> _flags
     >> _function
     >> _return_type
     >> _return_value_destructor;
  idf_input_string(in, _unique_name);
  idf_input_string(in, _comment);
  idf_input_vector(in, _parameters);
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunctionWrapper::remap_indices
//       Access: Public
//  Description: Remaps all internal index numbers according to the
//               indicated map.  This called from
//               InterrogateDatabase::remap_indices().
////////////////////////////////////////////////////////////////////
void InterrogateFunctionWrapper::
remap_indices(const IndexRemapper &remap) {
  _return_value_destructor = remap.map_from(_return_value_destructor);
  _return_type = remap.map_from(_return_type);

  Parameters::iterator pi;
  for (pi = _parameters.begin(); pi != _parameters.end(); ++pi) {
    (*pi)._type = remap.map_from((*pi)._type);
  }
}
