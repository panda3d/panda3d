// Filename: dcMolecularField.cxx
// Created by:  drose (05Oct00)
// 
////////////////////////////////////////////////////////////////////

#include "dcMolecularField.h"
#include "dcAtomicField.h"
#include "indent.h"

#include <assert.h>


////////////////////////////////////////////////////////////////////
//     Function: DCMolecularField::get_number
//       Access: Public
//  Description: Returns a unique index number associated with this
//               field.  This is defined implicitly when the .dc
//               file(s) are read.
////////////////////////////////////////////////////////////////////
int DCMolecularField::
get_number() const {
  return _number;
}

////////////////////////////////////////////////////////////////////
//     Function: DCMolecularField::get_name
//       Access: Public
//  Description: Returns the name of this field.
////////////////////////////////////////////////////////////////////
string DCMolecularField::
get_name() const {
  return _name;
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
  assert(n >= 0 && n < (int)_fields.size());
  return _fields[n];
}

////////////////////////////////////////////////////////////////////
//     Function: DCMolecularField::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
DCMolecularField::
DCMolecularField() {
  _number = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCMolecularField::write
//       Access: Public
//  Description: Generates a parseable description of the object to
//               the indicated output stream.
////////////////////////////////////////////////////////////////////
void DCMolecularField::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << _name;

  if (!_fields.empty()) {
    Fields::const_iterator fi = _fields.begin();
    out << " : " << (*fi)->_name;
    ++fi;
    while (fi != _fields.end()) {
      out << ", " << (*fi)->_name;
      ++fi;
    }
  }

  out << ";  // molecular " << _number << "\n";
}

