// Filename: dcClassParameter.cxx
// Created by:  drose (18Jun04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "dcClassParameter.h"
#include "dcClass.h"
#include "hashGenerator.h"

////////////////////////////////////////////////////////////////////
//     Function: DCClassParameter::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
DCClassParameter::
DCClassParameter(DCClass *dclass) :
  _dclass(dclass)
{
  set_name(dclass->get_name());

  _has_nested_fields = true;
  _num_nested_fields = _dclass->get_num_inherited_fields();
  _pack_type = PT_class;

  // If all of the nested fields have a fixed byte size, then so does
  // the class (and its byte size is the sum of all of the nested
  // fields).
  _has_fixed_byte_size = true;
  _fixed_byte_size = 0;
  _has_fixed_structure = true;
  for (int i = 0; i < _num_nested_fields && _has_fixed_byte_size; i++) {
    DCPackerInterface *field = get_nested_field(i);
    _has_fixed_byte_size = _has_fixed_byte_size && field->has_fixed_byte_size();
    _fixed_byte_size += field->get_fixed_byte_size();
    _has_fixed_structure = _has_fixed_structure && field->has_fixed_structure();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCClassParameter::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
DCClassParameter::
DCClassParameter(const DCClassParameter &copy) :
  DCParameter(copy),
  _dclass(copy._dclass)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DCClassParameter::as_class_parameter
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DCClassParameter *DCClassParameter::
as_class_parameter() {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: DCClassParameter::make_copy
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DCParameter *DCClassParameter::
make_copy() const {
  return new DCClassParameter(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: DCClassParameter::is_valid
//       Access: Published, Virtual
//  Description: Returns false if the type is an invalid type
//               (e.g. declared from an undefined typedef), true if
//               it is valid.
////////////////////////////////////////////////////////////////////
bool DCClassParameter::
is_valid() const {
  return !_dclass->is_bogus_class();
}

////////////////////////////////////////////////////////////////////
//     Function: DCClassParameter::get_class
//       Access: Published
//  Description: Returns the class object this parameter represents.
////////////////////////////////////////////////////////////////////
DCClass *DCClassParameter::
get_class() const {
  return _dclass;
}

////////////////////////////////////////////////////////////////////
//     Function: DCClassParameter::get_nested_field
//       Access: Public, Virtual
//  Description: Returns the DCPackerInterface object that represents
//               the nth nested field.  This may return NULL if there
//               is no such field (but it shouldn't do this if n is in
//               the range 0 <= n < get_num_nested_fields()).
////////////////////////////////////////////////////////////////////
DCPackerInterface *DCClassParameter::
get_nested_field(int n) const {
  return _dclass->get_inherited_field(n);
}

////////////////////////////////////////////////////////////////////
//     Function: DCClassParameter::output_instance
//       Access: Public, Virtual
//  Description: Formats the parameter in the C++-like dc syntax as a
//               typename and identifier.
////////////////////////////////////////////////////////////////////
void DCClassParameter::
output_instance(ostream &out, const string &prename, const string &name,
                const string &postname) const {
  if (get_typedef() != (DCTypedef *)NULL) {
    output_typedef_name(out, prename, name, postname);

  } else {
    out << get_name();
    if (!prename.empty() || !name.empty() || !postname.empty()) {
      out << " " << prename << name << postname;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCClassParameter::generate_hash
//       Access: Public, Virtual
//  Description: Accumulates the properties of this type into the
//               hash.
////////////////////////////////////////////////////////////////////
void DCClassParameter::
generate_hash(HashGenerator &hashgen) const {
  DCParameter::generate_hash(hashgen);
  _dclass->generate_hash(hashgen);
}
