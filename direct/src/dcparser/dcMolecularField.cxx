// Filename: dcMolecularField.cxx
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
DCMolecularField(const string &name) : DCField(name) {
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
  if (_fields.empty()) {
    set_flags(atomic->get_flags());
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
