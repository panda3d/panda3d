// Filename: dcAtomicField.cxx
// Created by:  drose (05Oct00)
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
DCAtomicField(const string &name) : DCField(name) {
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
  return _elements[n]._param;
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
////////////////////////////////////////////////////////////////////
string DCAtomicField::
get_element_default(int n) const {
  nassertr(has_element_default(n), string());
  nassertr(n >= 0 && n < (int)_elements.size(), string());

  return _elements[n]._default_value;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::has_element_default
//       Access: Published
//  Description: Returns true if the nth element of the field has a
//               default value specified, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCAtomicField::
has_element_default(int n) const {
  nassertr(n >= 0 && n < (int)_elements.size(), false);
  return _elements[n]._has_default_value;
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
  return _elements[n]._param->get_name();
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
  DCSimpleParameter *simple_parameter = _elements[n]._param->as_simple_parameter();
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
  DCSimpleParameter *simple_parameter = _elements[n]._param->as_simple_parameter();
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
    (*ei).output(out, brief);
    ++ei;
    while (ei != _elements.end()) {
      out << ", ";
      (*ei).output(out, brief);
      ++ei;
    }
  }
  out << ")";

  output_flags(out);
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
    const ElementType &element = (*ei);
    element._param->generate_hash(hashgen);
  }
  hashgen.add_int(get_flags());
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
  return _elements[n]._param;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::add_element
//       Access: Public
//  Description: Adds a new element (parameter) to the field.
//               Normally this is called only during parsing.
////////////////////////////////////////////////////////////////////
void DCAtomicField::
add_element(const DCAtomicField::ElementType &element) {
  _elements.push_back(element);
  _num_nested_fields = (int)_elements.size();

  // See if we still have a fixed byte size.
  if (_has_fixed_byte_size) {
    _has_fixed_byte_size = element._param->has_fixed_byte_size();
    _fixed_byte_size += element._param->get_fixed_byte_size();
  }
  if (_has_fixed_structure) {
    _has_fixed_structure = element._param->has_fixed_structure();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::ElementType::Constructor
//       Access: Public
//  Description: The type parameter should be a newly-allocated DCParameter
//               object; it will eventually be freed with delete when
//               this object destructs.
////////////////////////////////////////////////////////////////////
DCAtomicField::ElementType::
ElementType(DCParameter *param) {
  _param = param;
  _has_default_value = false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::ElementType::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DCAtomicField::ElementType::
ElementType(const DCAtomicField::ElementType &copy) :
  _param(copy._param->make_copy()),
  _default_value(copy._default_value),
  _has_default_value(copy._has_default_value)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::ElementType::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void DCAtomicField::ElementType::
operator = (const DCAtomicField::ElementType &copy) {
  delete _param;
  _param = copy._param->make_copy();
  _default_value = copy._default_value;
  _has_default_value = copy._has_default_value;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::ElementType::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
DCAtomicField::ElementType::
~ElementType() {
  delete _param;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::ElementType::set_default_value
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void DCAtomicField::ElementType::
set_default_value(const string &default_value) {
  _default_value = default_value;
  _has_default_value = true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::ElementType::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void DCAtomicField::ElementType::
output(ostream &out, bool brief) const {
  _param->output(out, brief);

  if (!brief && _has_default_value) {
    out << " = ";
    DCPacker packer;
    packer.set_unpack_data(_default_value);
    packer.begin_unpack(_param);
    packer.unpack_and_format(out);
    packer.end_unpack();
  }
}
