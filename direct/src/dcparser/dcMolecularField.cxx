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
//     Function: DCMolecularField::as_molecular_field
//       Access: Public, Virtual
//  Description: Returns the same field pointer converted to a
//               molecular field pointer, if this is in fact a
//               molecular field; otherwise, returns NULL.
////////////////////////////////////////////////////////////////////
DCMolecularField *DCMolecularField::
as_molecular_field() {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: DCMolecularField::get_num_atomics
//       Access: Public
//  Description: Returns the number of atomic fields that make up this
//               molecular field.
////////////////////////////////////////////////////////////////////
int DCMolecularField::
get_num_atomics() const {
  return _fields.size();
}

////////////////////////////////////////////////////////////////////
//     Function: DCMolecularField::get_atomic
//       Access: Public
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
//     Function: DCMolecularField::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DCMolecularField::
DCMolecularField(const string &name) : DCField(name) {
}

////////////////////////////////////////////////////////////////////
//     Function: DCMolecularField::write
//       Access: Public, Virtual
//  Description: Generates a parseable description of the object to
//               the indicated output stream.
////////////////////////////////////////////////////////////////////
void DCMolecularField::
write(ostream &out, bool brief, int indent_level) const {
  indent(out, indent_level) << _name;

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

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCMolecularField::do_pack_args
//       Access: Public, Virtual
//  Description: Packs the Python arguments beginning from the
//               indicated index of the indicated tuple into the
//               datagram, appending to the end of the datagram.
//               Increments index according to the number of arguments
//               packed.  Returns true if the tuple contained at least
//               enough arguments to match the field, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCMolecularField::
do_pack_args(Datagram &datagram, PyObject *tuple, int &index) const {
  Fields::const_iterator fi;
  for (fi = _fields.begin(); fi != _fields.end(); ++fi) {
    DCField *field = (*fi);
    if (!field->do_pack_args(datagram, tuple, index)) {
      return false;
    }
  }

  return true;
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCMolecularField::do_unpack_args
//       Access: Public, Virtual
//  Description: Unpacks the values from the datagram, beginning at
//               the current point in the interator, into a vector of
//               Python objects (each with its own reference count).
//               Returns true if there are enough values in the
//               datagram, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCMolecularField::
do_unpack_args(pvector<PyObject *> &args, DatagramIterator &iterator) const {
  Fields::const_iterator fi;
  for (fi = _fields.begin(); fi != _fields.end(); ++fi) {
    if (!(*fi)->do_unpack_args(args, iterator)) {
      return false;
    }
  }

  return true;
}
#endif  // HAVE_PYTHON
