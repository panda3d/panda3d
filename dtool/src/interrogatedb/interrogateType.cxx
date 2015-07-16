// Filename: interrogateType.cxx
// Created by:  drose (31Jul00)
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

#include "interrogateType.h"
#include "indexRemapper.h"
#include "interrogate_datafile.h"
#include "interrogateDatabase.h"

#include <algorithm>

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
InterrogateType::
InterrogateType(InterrogateModuleDef *def) :
  InterrogateComponent(def)
{
  _flags = 0;
  _outer_class = 0;
  _atomic_token = AT_not_atomic;
  _wrapped_type = 0;
  _array_size = 1;
  _destructor = 0;

  _cpptype = (CPPType *)NULL;
  _cppscope = (CPPScope *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
InterrogateType::
InterrogateType(const InterrogateType &copy) {
  (*this) = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::Derivation::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void InterrogateType::Derivation::
output(ostream &out) const {
  out << _flags << " " << _base << " " << _upcast << " " << _downcast;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::Derivation::input
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void InterrogateType::Derivation::
input(istream &in) {
  in >> _flags >> _base >> _upcast >> _downcast;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::EnumValue::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void InterrogateType::EnumValue::
output(ostream &out) const {
  idf_output_string(out, _name);
  idf_output_string(out, _scoped_name);
  idf_output_string(out, _comment, '\n');
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::EnumValue::input
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void InterrogateType::EnumValue::
input(istream &in) {
  idf_input_string(in, _name);
  idf_input_string(in, _scoped_name);
  if (InterrogateDatabase::get_file_minor_version() >= 3) {
    idf_input_string(in, _comment);
  }
  in >> _value;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void InterrogateType::
operator = (const InterrogateType &copy) {
  InterrogateComponent::operator = (copy);
  _flags = copy._flags;
  _scoped_name = copy._scoped_name;
  _true_name = copy._true_name;
  _comment = copy._comment;
  _outer_class = copy._outer_class;
  _atomic_token = copy._atomic_token;
  _wrapped_type = copy._wrapped_type;
  _array_size = copy._array_size;
  _constructors = copy._constructors;
  _destructor = copy._destructor;
  _elements = copy._elements;
  _methods = copy._methods;
  _make_seqs = copy._make_seqs;
  _casts = copy._casts;
  _derivations = copy._derivations;
  _enum_values = copy._enum_values;
  _nested_types = copy._nested_types;

  _cpptype = copy._cpptype;
  _cppscope = copy._cppscope;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::merge_with
//       Access: Public
//  Description: Combines type with the other similar definition.  If
//               one type is "fully defined" and the other one isn't,
//               the fully-defined type wins.
////////////////////////////////////////////////////////////////////
void InterrogateType::
merge_with(const InterrogateType &other) {
  // The only thing we care about copying from the non-fully-defined
  // type right now is the global flag.

  if (is_fully_defined()) {
    // We win.
    _flags |= (other._flags & F_global);

  } else {
    // They win.
    int old_flags = (_flags & F_global);
    (*this) = other;
    _flags |= old_flags;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::output
//       Access: Public
//  Description: Formats the InterrogateType data for output to a data
//               file.
////////////////////////////////////////////////////////////////////
void InterrogateType::
output(ostream &out) const {
  InterrogateComponent::output(out);

  out << _flags << " ";
  idf_output_string(out, _scoped_name);
  idf_output_string(out, _true_name);
  out << _outer_class << " "
      << (int)_atomic_token << " "
      << _wrapped_type << " ";

  if (is_array()) {
    out << _array_size << " ";
  }

  idf_output_vector(out, _constructors);
  out << _destructor << " ";
  idf_output_vector(out, _elements);
  idf_output_vector(out, _methods);
  idf_output_vector(out, _make_seqs);
  idf_output_vector(out, _casts);
  idf_output_vector(out, _derivations);
  idf_output_vector(out, _enum_values);
  idf_output_vector(out, _nested_types);
  idf_output_string(out, _comment, '\n');
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::input
//       Access: Public
//  Description: Reads the data file as previously formatted by
//               output().
////////////////////////////////////////////////////////////////////
void InterrogateType::
input(istream &in) {
  InterrogateComponent::input(in);

  in >> _flags;
  idf_input_string(in, _scoped_name);
  idf_input_string(in, _true_name);

  in >> _outer_class;
  int token;
  in >> token;
  _atomic_token = (AtomicToken)token;
  in >> _wrapped_type;

  if (is_array()) {
    in >> _array_size;
  }

  idf_input_vector(in, _constructors);
  in >> _destructor;

  idf_input_vector(in, _elements);
  idf_input_vector(in, _methods);
  idf_input_vector(in, _make_seqs);
  idf_input_vector(in, _casts);
  idf_input_vector(in, _derivations);
  idf_input_vector(in, _enum_values);
  idf_input_vector(in, _nested_types);
  idf_input_string(in, _comment);
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateType::remap_indices
//       Access: Public
//  Description: Remaps all internal index numbers according to the
//               indicated map.  This called from
//               InterrogateDatabase::remap_indices().
////////////////////////////////////////////////////////////////////
void InterrogateType::
remap_indices(const IndexRemapper &remap) {
  _outer_class = remap.map_from(_outer_class);
  _wrapped_type = remap.map_from(_wrapped_type);

  Functions::iterator fi;
  for (fi = _constructors.begin(); fi != _constructors.end(); ++fi) {
    (*fi) = remap.map_from(*fi);
  }
  _destructor = remap.map_from(_destructor);

  Elements::iterator ei;
  for (ei = _elements.begin(); ei != _elements.end(); ++ei) {
    (*ei) = remap.map_from(*ei);
  }

  for (fi = _methods.begin(); fi != _methods.end(); ++fi) {
    (*fi) = remap.map_from(*fi);
  }
  for (fi = _casts.begin(); fi != _casts.end(); ++fi) {
    (*fi) = remap.map_from(*fi);
  }

  MakeSeqs::iterator si;
  for (si = _make_seqs.begin(); si != _make_seqs.end(); ++si) {
    (*si) = remap.map_from(*si);
  }

  Derivations::iterator di;
  for (di = _derivations.begin(); di != _derivations.end(); ++di) {
    (*di)._base = remap.map_from((*di)._base);
    (*di)._upcast = remap.map_from((*di)._upcast);
    (*di)._downcast = remap.map_from((*di)._downcast);
  }

  Types::iterator ti;
  for (ti = _nested_types.begin(); ti != _nested_types.end(); ++ti) {
    (*ti) = remap.map_from(*ti);
  }

}
