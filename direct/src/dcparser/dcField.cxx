// Filename: dcField.cxx
// Created by:  drose (11Oct00)
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

#include "dcField.h"
#include "hashGenerator.h"

////////////////////////////////////////////////////////////////////
//     Function: DCField::get_number
//       Access: Public
//  Description: Returns a unique index number associated with this
//               field.  This is defined implicitly when the .dc
//               file(s) are read.
////////////////////////////////////////////////////////////////////
int DCField::
get_number() const {
  return _number;
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::get_name
//       Access: Public
//  Description: Returns the name of this field.
////////////////////////////////////////////////////////////////////
const string &DCField::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::as_atomic_field
//       Access: Public, Virtual
//  Description: Returns the same field pointer converted to an atomic
//               field pointer, if this is in fact an atomic field;
//               otherwise, returns NULL.
////////////////////////////////////////////////////////////////////
DCAtomicField *DCField::
as_atomic_field() {
  return (DCAtomicField *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::as_molecular_field
//       Access: Public, Virtual
//  Description: Returns the same field pointer converted to a
//               molecular field pointer, if this is in fact a
//               molecular field; otherwise, returns NULL.
////////////////////////////////////////////////////////////////////
DCMolecularField *DCField::
as_molecular_field() {
  return (DCMolecularField *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
DCField::
~DCField() {
}

////////////////////////////////////////////////////////////////////
//     Function: DCField::generate_hash
//       Access: Public, Virtual
//  Description: Accumulates the properties of this field into the
//               hash.
////////////////////////////////////////////////////////////////////
void DCField::
generate_hash(HashGenerator &hashgen) const {
  // It shouldn't be necessary to explicitly add _number to the
  // hash--this is computed based on the relative position of this
  // field with the other fields, so adding it explicitly will be
  // redundant.  However, the field name is significant.
  hashgen.add_string(_name);
}
