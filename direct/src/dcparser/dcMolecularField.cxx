// Filename: dcMolecularField.cxx
// Created by:  drose (05Oct00)
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
DCMolecularField() {
  _number = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCMolecularField::write
//       Access: Public, Virtual
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

  out << ";  // field " << _number << "\n";
}


////////////////////////////////////////////////////////////////////
//     Function: DCMolecularField::generate_hash
//       Access: Public, Virtual
//  Description: Accumulates the properties of this field into the
//               hash.
////////////////////////////////////////////////////////////////////
void DCMolecularField::
generate_hash(HashGenerator &hash) const {
  DCField::generate_hash(hash);

  hash.add_int(_fields.size());
  Fields::const_iterator fi;
  for (fi = _fields.begin(); fi != _fields.end(); ++fi) {
    (*fi)->generate_hash(hash);
  }
}
