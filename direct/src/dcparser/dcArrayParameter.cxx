// Filename: dcArrayParameter.cxx
// Created by:  drose (17Jun04)
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

#include "dcArrayParameter.h"
#include "hashGenerator.h"

////////////////////////////////////////////////////////////////////
//     Function: DCArrayParameter::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
DCArrayParameter::
DCArrayParameter(DCParameter *element_type, const DCUnsignedIntRange &size) :
  _element_type(element_type),
  _array_size_range(size)
{
  set_name(_element_type->get_name());
  _element_type->set_name(string());

  _array_size = -1;
  if (_array_size_range.has_one_value()) {
    _array_size = _array_size_range.get_one_value();
  } else {
    _has_range_limits = true;
  }

  if (_array_size >= 0 && _element_type->has_fixed_byte_size()) {
    _has_fixed_byte_size = true;
    _fixed_byte_size = _array_size * _element_type->get_fixed_byte_size();
    _has_fixed_structure = true;
    
  } else {
    // We only need to store the length bytes if the array has a
    // variable size.
    _num_length_bytes = 2;
  }

  if (_element_type->has_range_limits()) {
    _has_range_limits = true;
  }

  if (_element_type->has_default_value()) {
    _has_default_value = true;
  }

  _has_nested_fields = true;
  _num_nested_fields = _array_size;
  _pack_type = PT_array;
}

////////////////////////////////////////////////////////////////////
//     Function: DCArrayParameter::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
DCArrayParameter::
DCArrayParameter(const DCArrayParameter &copy) :
  DCParameter(copy),
  _element_type(copy._element_type->make_copy()),
  _array_size(copy._array_size),
  _array_size_range(copy._array_size_range)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DCArrayParameter::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DCArrayParameter::
~DCArrayParameter() {
  delete _element_type;
}

////////////////////////////////////////////////////////////////////
//     Function: DCArrayParameter::as_array_parameter
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DCArrayParameter *DCArrayParameter::
as_array_parameter() {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: DCArrayParameter::as_array_parameter
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const DCArrayParameter *DCArrayParameter::
as_array_parameter() const {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: DCArrayParameter::make_copy
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DCParameter *DCArrayParameter::
make_copy() const {
  return new DCArrayParameter(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: DCArrayParameter::is_valid
//       Access: Published, Virtual
//  Description: Returns false if the type is an invalid type
//               (e.g. declared from an undefined typedef), true if
//               it is valid.
////////////////////////////////////////////////////////////////////
bool DCArrayParameter::
is_valid() const {
  return _element_type->is_valid();
}

////////////////////////////////////////////////////////////////////
//     Function: DCArrayParameter::get_element_type
//       Access: Published
//  Description: Returns the type of the individual elements of this
//               array.
////////////////////////////////////////////////////////////////////
DCParameter *DCArrayParameter::
get_element_type() const {
  return _element_type;
}

////////////////////////////////////////////////////////////////////
//     Function: DCArrayParameter::get_array_size
//       Access: Published
//  Description: Returns the fixed number of elements in this array,
//               or -1 if the array may contain a variable number of
//               elements.
////////////////////////////////////////////////////////////////////
int DCArrayParameter::
get_array_size() const {
  return _array_size;
}

////////////////////////////////////////////////////////////////////
//     Function: DCArrayParameter::calc_num_nested_fields
//       Access: Public, Virtual
//  Description: This flavor of get_num_nested_fields is used during
//               unpacking.  It returns the number of nested fields to
//               expect, given a certain length in bytes (as read from
//               the get_num_length_bytes() stored in the stream on the
//               pack).  This will only be called if
//               get_num_length_bytes() returns nonzero.
////////////////////////////////////////////////////////////////////
int DCArrayParameter::
calc_num_nested_fields(size_t length_bytes) const {
  if (_element_type->has_fixed_byte_size()) {
    return length_bytes / _element_type->get_fixed_byte_size();
  }
  return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: DCArrayParameter::get_nested_field
//       Access: Public, Virtual
//  Description: Returns the DCPackerInterface object that represents
//               the nth nested field.  This may return NULL if there
//               is no such field (but it shouldn't do this if n is in
//               the range 0 <= n < get_num_nested_fields()).
////////////////////////////////////////////////////////////////////
DCPackerInterface *DCArrayParameter::
get_nested_field(int) const {
  return _element_type;
}

////////////////////////////////////////////////////////////////////
//     Function: DCArrayParameter::validate_num_nested_fields
//       Access: Public, Virtual
//  Description: After a number of fields have been packed via push()
//               .. pack_*() .. pop(), this is called to confirm that
//               the number of nested fields that were added is valid
//               for this type.  This is primarily useful for array
//               types with dynamic ranges that can't validate the
//               number of fields any other way.
////////////////////////////////////////////////////////////////////
bool DCArrayParameter::
validate_num_nested_fields(int num_nested_fields) const {
  bool range_error = false;
  _array_size_range.validate(num_nested_fields, range_error);
       
  return !range_error;
}

////////////////////////////////////////////////////////////////////
//     Function: DCArrayParameter::output_instance
//       Access: Public, Virtual
//  Description: Formats the parameter in the C++-like dc syntax as a
//               typename and identifier.
////////////////////////////////////////////////////////////////////
void DCArrayParameter::
output_instance(ostream &out, bool brief, const string &prename,
                const string &name, const string &postname) const {
  if (get_typedef() != (DCTypedef *)NULL) {
    output_typedef_name(out, brief, prename, name, postname);

  } else {
    ostringstream strm;
    
    strm << "[";
    _array_size_range.output(strm);
    strm << "]";
    
    _element_type->output_instance(out, brief, prename, name, 
                                   strm.str() + postname);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCArrayParameter::generate_hash
//       Access: Public, Virtual
//  Description: Accumulates the properties of this type into the
//               hash.
////////////////////////////////////////////////////////////////////
void DCArrayParameter::
generate_hash(HashGenerator &hashgen) const {
  DCParameter::generate_hash(hashgen);
  _element_type->generate_hash(hashgen);
  _array_size_range.generate_hash(hashgen);
}

////////////////////////////////////////////////////////////////////
//     Function: DCArrayParameter::pack_default_value
//       Access: Public, Virtual
//  Description: Packs the arrayParameter's specified default value (or a
//               sensible default if no value is specified) into the
//               stream.  Returns true if the default value is packed,
//               false if the arrayParameter doesn't know how to pack its
//               default value.
////////////////////////////////////////////////////////////////////
bool DCArrayParameter::
pack_default_value(DCPackData &pack_data, bool &pack_error) const {
  if (has_default_value()) {
    return DCField::pack_default_value(pack_data, pack_error);
  }

  // If a default value is not specified for a variable-length array,
  // the default is the minimum array.
  unsigned int minimum_length = 0;
  if (!_array_size_range.is_empty()) {
    minimum_length = _array_size_range.get_min(0);
  }

  DCPacker packer;
  packer.begin_pack(this);
  packer.push();
  for (unsigned int i = 0; i < minimum_length; i++) {
    packer.pack_default_value();
  }
  packer.pop();
  if (!packer.end_pack()) {
    pack_error = true;

  } else {
    pack_data.append_data(packer.get_data(), packer.get_length());
  }

  return true;
}
