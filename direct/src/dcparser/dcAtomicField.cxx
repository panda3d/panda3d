// Filename: dcAtomicField.cxx
// Created by:  drose (05Oct00)
// 
////////////////////////////////////////////////////////////////////

#include "dcAtomicField.h"
#include "hashGenerator.h"
#include "dcindent.h"


ostream &
operator << (ostream &out, const DCAtomicField::ElementType &et) {
  out << et._type;
  if (et._divisor != 1) {
    out << " / " << et._divisor;
  }
  return out;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::as_atomic_field
//       Access: Public, Virtual
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
//       Access: Public
//  Description: Returns the number of elements of the atomic field.
////////////////////////////////////////////////////////////////////
int DCAtomicField::
get_num_elements() const {
  return _elements.size();
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::get_element_type
//       Access: Public
//  Description: Returns the numeric type of the nth element of the
//               field.
////////////////////////////////////////////////////////////////////
DCSubatomicType DCAtomicField::
get_element_type(int n) const {
  nassertr(n >= 0 && n < (int)_elements.size(), ST_invalid);
  return _elements[n]._type;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::get_element_divisor
//       Access: Public
//  Description: Returns the divisor associated with the nth element
//               of the field.  This implements an implicit
//               fixed-point system; floating-point values are to be
//               multiplied by this value before encoding into a
//               packet, and divided by this number after decoding.
////////////////////////////////////////////////////////////////////
int DCAtomicField::
get_element_divisor(int n) const {
  nassertr(n >= 0 && n < (int)_elements.size(), 1);
  return _elements[n]._divisor;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::is_required
//       Access: Public
//  Description: Returns true if the "required" flag is set for this
//               field, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCAtomicField::
is_required() const {
  return (_flags & F_required) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::is_broadcast
//       Access: Public
//  Description: Returns true if the "broadcast" flag is set for this
//               field, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCAtomicField::
is_broadcast() const {
  return (_flags & F_broadcast) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::is_p2p
//       Access: Public
//  Description: Returns true if the "p2p" flag is set for this
//               field, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCAtomicField::
is_p2p() const {
  return (_flags & F_p2p) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::is_ram
//       Access: Public
//  Description: Returns true if the "ram" flag is set for this
//               field, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCAtomicField::
is_ram() const {
  return (_flags & F_ram) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::is_db
//       Access: Public
//  Description: Returns true if the "db" flag is set for this
//               field, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCAtomicField::
is_db() const {
  return (_flags & F_db) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::is_clsend
//       Access: Public
//  Description: Returns true if the "clsend" flag is set for this
//               field, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCAtomicField::
is_clsend() const {
  return (_flags & F_clsend) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::is_clrecv
//       Access: Public
//  Description: Returns true if the "clrecv" flag is set for this
//               field, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCAtomicField::
is_clrecv() const {
  return (_flags & F_clrecv) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::is_ownsend
//       Access: Public
//  Description: Returns true if the "ownsend" flag is set for this
//               field, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCAtomicField::
is_ownsend() const {
  return (_flags & F_ownsend) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
DCAtomicField::
DCAtomicField() {
  _number = 0;
  _flags = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::write
//       Access: Public, Virtual
//  Description: Generates a parseable description of the object to
//               the indicated output stream.
////////////////////////////////////////////////////////////////////
void DCAtomicField::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << _name << "(";

  if (!_elements.empty()) {
    Elements::const_iterator ei = _elements.begin();
    out << (*ei);
    ++ei;
    while (ei != _elements.end()) {
      out << ", " << (*ei);
      ++ei;
    }
  }
  out << ")";

  if ((_flags & F_required) != 0) {
    out << " required";
  }
  if ((_flags & F_broadcast) != 0) {
    out << " broadcast";
  }
  if ((_flags & F_p2p) != 0) {
    out << " p2p";
  }
  if ((_flags & F_ram) != 0) {
    out << " ram";
  }
  if ((_flags & F_db) != 0) {
    out << " db";
  }
  if ((_flags & F_clsend) != 0) {
    out << " clsend";
  }
  if ((_flags & F_clrecv) != 0) {
    out << " clrecv";
  }
  if ((_flags & F_ownsend) != 0) {
    out << " ownsend";
  }

  out << ";  // field " << _number << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: DCAtomicField::generate_hash
//       Access: Public, Virtual
//  Description: Accumulates the properties of this field into the
//               hash.
////////////////////////////////////////////////////////////////////
void DCAtomicField::
generate_hash(HashGenerator &hash) const {
  DCField::generate_hash(hash);

  hash.add_int(_elements.size());
  Elements::const_iterator ei;
  for (ei = _elements.begin(); ei != _elements.end(); ++ei) {
    const ElementType &element = (*ei);
    hash.add_int(element._type);
    hash.add_int(element._divisor);
  }
  hash.add_int(_flags);
}
