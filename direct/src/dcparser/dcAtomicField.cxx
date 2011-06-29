// Filename: dcAtomicField.cxx
// Created by:  drose (05Oct00)
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

#include "dcAtomicField.h"
#include "hashGenerator.h"
#include "dcindent.h"
#include "dcSimpleParameter.h"
#include "dcPacker.h"

#include <math.h>

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DCAtomicField::
DCAtomicField(const string &name, DCClass *dclass,
              bool bogus_field) : 
  DCField(name, dclass)
{
  _bogus_field = bogus_field;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
DCAtomicField::
~DCAtomicField() {
  Elements::iterator ei;  
  for (ei = _elements.begin(); ei != _elements.end(); ++ei) {
    delete (*ei);
  }
  _elements.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::as_atomic_field
//       Access: Published, Virtual
//  Description: Returns the same field pointer converted to an atomic
//               field pointer, if this is in fact an atomic field;
//               otherwise, returns NULL.
////////////////////////////////////////////////////////////////////
DCAtomicField *DCAtomicField::
as_atomic_field() {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::as_atomic_field
//       Access: Published, Virtual
//  Description: Returns the same field pointer converted to an atomic
//               field pointer, if this is in fact an atomic field;
//               otherwise, returns NULL.
////////////////////////////////////////////////////////////////////
const DCAtomicField *DCAtomicField::
as_atomic_field() const {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::get_num_elements
//       Access: Published
//  Description: Returns the number of elements (parameters) of the
//               atomic field.
////////////////////////////////////////////////////////////////////
int DCAtomicField::
get_num_elements() const {
  return _elements.size();
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::get_element
//       Access: Published
//  Description: Returns the parameter object describing the
//               nth element.
////////////////////////////////////////////////////////////////////
DCParameter *DCAtomicField::
get_element(int n) const {
  nassertr(n >= 0 && n < (int)_elements.size(), NULL);
  return _elements[n];
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::get_element_default
//       Access: Published
//  Description: Returns the pre-formatted default value associated
//               with the nth element of the field.  This is only
//               valid if has_element_default() returns true, in which
//               case this string represents the bytes that should be
//               assigned to the field as a default value.
//
//               If the element is an array-type element, the returned
//               value will include the two-byte length preceding the
//               array data.
//
//               This is deprecated; use get_element() instead.
////////////////////////////////////////////////////////////////////
string DCAtomicField::
get_element_default(int n) const {
  nassertr(n >= 0 && n < (int)_elements.size(), string());
  return _elements[n]->get_default_value();
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::has_element_default
//       Access: Published
//  Description: Returns true if the nth element of the field has a
//               default value specified, false otherwise.
//
//               This is deprecated; use get_element() instead.
////////////////////////////////////////////////////////////////////
bool DCAtomicField::
has_element_default(int n) const {
  nassertr(n >= 0 && n < (int)_elements.size(), false);
  return _elements[n]->has_default_value();
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::get_element_name
//       Access: Published
//  Description: Returns the name of the nth element of the field.
//               This name is strictly for documentary purposes; it
//               does not generally affect operation.  If a name is
//               not specified, this will be the empty string.
//
//               This method is deprecated; use
//               get_element()->get_name() instead.
////////////////////////////////////////////////////////////////////
string DCAtomicField::
get_element_name(int n) const {
  nassertr(n >= 0 && n < (int)_elements.size(), string());
  return _elements[n]->get_name();
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::get_element_type
//       Access: Published
//  Description: Returns the numeric type of the nth element of the
//               field.  This method is deprecated; use
//               get_element() instead.
////////////////////////////////////////////////////////////////////
DCSubatomicType DCAtomicField::
get_element_type(int n) const {
  nassertr(n >= 0 && n < (int)_elements.size(), ST_invalid);
  DCSimpleParameter *simple_parameter = _elements[n]->as_simple_parameter();
  nassertr(simple_parameter != (DCSimpleParameter *)NULL, ST_invalid);
  return simple_parameter->get_type();
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::get_element_divisor
//       Access: Published
//  Description: Returns the divisor associated with the nth element
//               of the field.  This implements an implicit
//               fixed-point system; floating-point values are to be
//               multiplied by this value before encoding into a
//               packet, and divided by this number after decoding.
//
//               This method is deprecated; use
//               get_element()->get_divisor() instead.
////////////////////////////////////////////////////////////////////
int DCAtomicField::
get_element_divisor(int n) const {
  nassertr(n >= 0 && n < (int)_elements.size(), 1);
  DCSimpleParameter *simple_parameter = _elements[n]->as_simple_parameter();
  nassertr(simple_parameter != (DCSimpleParameter *)NULL, 1);
  return simple_parameter->get_divisor();
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void DCAtomicField::
output(ostream &out, bool brief) const {
  out << _name << "(";

  if (!_elements.empty()) {
    Elements::const_iterator ei = _elements.begin();
    output_element(out, brief, *ei);
    ++ei;
    while (ei != _elements.end()) {
      out << ", ";
      output_element(out, brief, *ei);
      ++ei;
    }
  }
  out << ")";

  output_keywords(out);
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::write
//       Access: Public, Virtual
//  Description: Generates a parseable description of the object to
//               the indicated output stream.
////////////////////////////////////////////////////////////////////
void DCAtomicField::
write(ostream &out, bool brief, int indent_level) const {
  indent(out, indent_level);
  output(out, brief);
  out << ";";
  if (!brief && _number >= 0) {
    out << "  // field " << _number;
  }
  out << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::generate_hash
//       Access: Public, Virtual
//  Description: Accumulates the properties of this field into the
//               hash.
////////////////////////////////////////////////////////////////////
void DCAtomicField::
generate_hash(HashGenerator &hashgen) const {
  DCField::generate_hash(hashgen);

  hashgen.add_int(_elements.size());
  Elements::const_iterator ei;
  for (ei = _elements.begin(); ei != _elements.end(); ++ei) {
    (*ei)->generate_hash(hashgen);
  }

  DCKeywordList::generate_hash(hashgen);
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::get_nested_field
//       Access: Public, Virtual
//  Description: Returns the DCPackerInterface object that represents
//               the nth nested field.  This may return NULL if there
//               is no such field (but it shouldn't do this if n is in
//               the range 0 <= n < get_num_nested_fields()).
////////////////////////////////////////////////////////////////////
DCPackerInterface *DCAtomicField::
get_nested_field(int n) const {
  nassertr(n >= 0 && n < (int)_elements.size(), NULL);
  return _elements[n];
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::add_element
//       Access: Public
//  Description: Adds a new element (parameter) to the field.
//               Normally this is called only during parsing.  The
//               DCAtomicField object becomes the owner of the new
//               pointer and will delete it upon destruction.
////////////////////////////////////////////////////////////////////
void DCAtomicField::
add_element(DCParameter *element) {
  _elements.push_back(element);
  _num_nested_fields = (int)_elements.size();

  // See if we still have a fixed byte size.
  if (_has_fixed_byte_size) {
    _has_fixed_byte_size = element->has_fixed_byte_size();
    _fixed_byte_size += element->get_fixed_byte_size();
  }
  if (_has_fixed_structure) {
    _has_fixed_structure = element->has_fixed_structure();
  }
  if (!_has_range_limits) {
    _has_range_limits = element->has_range_limits();
  }
  if (!_has_default_value) {
    _has_default_value = element->has_default_value();
  }
  _default_value_stale = true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::do_check_match
//       Access: Protected, Virtual
//  Description: Returns true if the other interface is bitwise the
//               same as this one--that is, a uint32 only matches a
//               uint32, etc. Names of components, and range limits,
//               are not compared.
////////////////////////////////////////////////////////////////////
bool DCAtomicField::
do_check_match(const DCPackerInterface *other) const {
  return other->do_check_match_atomic_field(this);
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::do_check_match_atomic_field
//       Access: Protected, Virtual
//  Description: Returns true if this field matches the indicated
//               atomic field, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCAtomicField::
do_check_match_atomic_field(const DCAtomicField *other) const {
  if (_elements.size() != other->_elements.size()) {
    return false;
  }
  for (size_t i = 0; i < _elements.size(); i++) {
    if (!_elements[i]->check_match(other->_elements[i])) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::output_element
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void DCAtomicField::
output_element(ostream &out, bool brief, DCParameter *element) const {
  element->output(out, brief);

  if (!brief && element->has_default_value()) {
    out << " = ";
    DCPacker packer;
    packer.set_unpack_data(element->get_default_value());
    packer.begin_unpack(element);
    packer.unpack_and_format(out, false);
    packer.end_unpack();
  }
}
