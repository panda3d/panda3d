// Filename: dcMolecularField.cxx
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

#include "dcMolecularField.h"
#include "dcAtomicField.h"
#include "hashGenerator.h"
#include "dcindent.h"



////////////////////////////////////////////////////////////////////
//     Function: DCMolecularField::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DCMolecularField::
DCMolecularField(const string &name, DCClass *dclass) : DCField(name, dclass) {
  _got_keywords = false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCMolecularField::as_molecular_field
//       Access: Published, Virtual
//  Description: Returns the same field pointer converted to a
//               molecular field pointer, if this is in fact a
//               molecular field; otherwise, returns NULL.
////////////////////////////////////////////////////////////////////
DCMolecularField *DCMolecularField::
as_molecular_field() {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: DCMolecularField::as_molecular_field
//       Access: Published, Virtual
//  Description: Returns the same field pointer converted to a
//               molecular field pointer, if this is in fact a
//               molecular field; otherwise, returns NULL.
////////////////////////////////////////////////////////////////////
const DCMolecularField *DCMolecularField::
as_molecular_field() const {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: DCMolecularField::get_num_atomics
//       Access: Published
//  Description: Returns the number of atomic fields that make up this
//               molecular field.
////////////////////////////////////////////////////////////////////
int DCMolecularField::
get_num_atomics() const {
  return _fields.size();
}

////////////////////////////////////////////////////////////////////
//     Function: DCMolecularField::get_atomic
//       Access: Published
//  Description: Returns the nth atomic field that makes up this
//               molecular field.  This may or may not be a field of
//               this particular class; it might be defined in a
//               parent class.
////////////////////////////////////////////////////////////////////
DCAtomicField *DCMolecularField::
get_atomic(int n) const {
  nassertr(n >= 0 && n < (int)_fields.size(), NULL);
  return _fields[n];
}

////////////////////////////////////////////////////////////////////
//     Function: DCMolecularField::add_atomic
//       Access: Public
//  Description: Adds the indicated atomic field to the end of the
//               list of atomic fields that make up the molecular
//               field.  This is normally called only during parsing
//               of the dc file.  The atomic field should be fully
//               defined by this point; you should not modify the
//               atomic field (e.g. by adding more elements) after
//               adding it to a molecular field.
////////////////////////////////////////////////////////////////////
void DCMolecularField::
add_atomic(DCAtomicField *atomic) {
  if (!atomic->is_bogus_field()) {
    if (!_got_keywords) {
      // The first non-bogus atomic field determines our keywords.
      copy_keywords(*atomic);
      _got_keywords = true;
    }
  }
  _fields.push_back(atomic);

  int num_atomic_fields = atomic->get_num_nested_fields();
  for (int i = 0; i < num_atomic_fields; i++) {
    _nested_fields.push_back(atomic->get_nested_field(i));
  }

  _num_nested_fields = _nested_fields.size();

  // See if we still have a fixed byte size.
  if (_has_fixed_byte_size) {
    _has_fixed_byte_size = atomic->has_fixed_byte_size();
    _fixed_byte_size += atomic->get_fixed_byte_size();
  }
  if (_has_fixed_structure) {
    _has_fixed_structure = atomic->has_fixed_structure();
  }
  if (!_has_range_limits) {
    _has_range_limits = atomic->has_range_limits();
  }
  if (!_has_default_value) {
    _has_default_value = atomic->has_default_value();
  }
  _default_value_stale = true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCMolecularField::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void DCMolecularField::
output(ostream &out, bool brief) const {
  out << _name;

  if (!_fields.empty()) {
    Fields::const_iterator fi = _fields.begin();
    out << " : " << (*fi)->get_name();
    ++fi;
    while (fi != _fields.end()) {
      out << ", " << (*fi)->get_name();
      ++fi;
    }
  }
  
  out << ";";
}

////////////////////////////////////////////////////////////////////
//     Function: DCMolecularField::write
//       Access: Public, Virtual
//  Description: Generates a parseable description of the object to
//               the indicated output stream.
////////////////////////////////////////////////////////////////////
void DCMolecularField::
write(ostream &out, bool brief, int indent_level) const {
  indent(out, indent_level);
  output(out, brief);
  if (!brief) {
    out << "  // field " << _number;
  }
  out << "\n";
}


////////////////////////////////////////////////////////////////////
//     Function: DCMolecularField::generate_hash
//       Access: Public, Virtual
//  Description: Accumulates the properties of this field into the
//               hash.
////////////////////////////////////////////////////////////////////
void DCMolecularField::
generate_hash(HashGenerator &hashgen) const {
  DCField::generate_hash(hashgen);

  hashgen.add_int(_fields.size());
  Fields::const_iterator fi;
  for (fi = _fields.begin(); fi != _fields.end(); ++fi) {
    (*fi)->generate_hash(hashgen);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCMolecularField::get_nested_field
//       Access: Public, Virtual
//  Description: Returns the DCPackerInterface object that represents
//               the nth nested field.  This may return NULL if there
//               is no such field (but it shouldn't do this if n is in
//               the range 0 <= n < get_num_nested_fields()).
////////////////////////////////////////////////////////////////////
DCPackerInterface *DCMolecularField::
get_nested_field(int n) const {
  nassertr(n >= 0 && n < (int)_nested_fields.size(), NULL);
  return _nested_fields[n];
}

////////////////////////////////////////////////////////////////////
//     Function: DCMolecularField::do_check_match
//       Access: Protected, Virtual
//  Description: Returns true if the other interface is bitwise the
//               same as this one--that is, a uint32 only matches a
//               uint32, etc. Names of components, and range limits,
//               are not compared.
////////////////////////////////////////////////////////////////////
bool DCMolecularField::
do_check_match(const DCPackerInterface *other) const {
  return other->do_check_match_molecular_field(this);
}

////////////////////////////////////////////////////////////////////
//     Function: DCMolecularField::do_check_match_molecular_field
//       Access: Protected, Virtual
//  Description: Returns true if this field matches the indicated
//               molecular field, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCMolecularField::
do_check_match_molecular_field(const DCMolecularField *other) const {
  if (_nested_fields.size() != other->_nested_fields.size()) {
    return false;
  }
  for (size_t i = 0; i < _nested_fields.size(); i++) {
    if (!_nested_fields[i]->check_match(other->_nested_fields[i])) {
      return false;
    }
  }

  return true;
}
