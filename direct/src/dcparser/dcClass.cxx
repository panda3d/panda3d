// Filename: dcClass.cxx
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

#include "dcClass.h"
#include "hashGenerator.h"
#include "dcindent.h"

////////////////////////////////////////////////////////////////////
//     Function: DCClass::get_number
//       Access: Public
//  Description: Returns a unique index number associated with this
//               class.  This is defined implicitly when the .dc
//               file(s) are read.
////////////////////////////////////////////////////////////////////
int DCClass::
get_number() const {
  return _number;
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::get_name
//       Access: Public
//  Description: Returns the name of this class.
////////////////////////////////////////////////////////////////////
string DCClass::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::has_parent
//       Access: Public
//  Description: Returns true if this class inherits from some other
//               class, false if it does not.
////////////////////////////////////////////////////////////////////
bool DCClass::
has_parent() const {
  return !_parents.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::get_parent
//       Access: Public
//  Description: Returns the parent class this class inherits from, if
//               any.  It is an error to call this unless has_parent()
//               returned true.
////////////////////////////////////////////////////////////////////
DCClass *DCClass::
get_parent() const {
  nassertr(has_parent(), NULL);
  return _parents.front();
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::get_num_fields
//       Access: Public
//  Description: Returns the number of fields defined directly in this
//               class, ignoring inheritance.
////////////////////////////////////////////////////////////////////
int DCClass::
get_num_fields() {
  return _fields.size();
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::get_field
//       Access: Public
//  Description: Returns the nth field in the class.  This is not
//               necessarily the field with index n; this is the nth
//               field defined in the class directly, ignoring
//               inheritance.
////////////////////////////////////////////////////////////////////
DCField *DCClass::
get_field(int n) {
  nassertr(n >= 0 && n < (int)_fields.size(), NULL);
  return _fields[n];
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::get_field_by_name
//       Access: Public
//  Description: Returns a pointer to the DCField that shares the
//               indicated name.  If the named field is not found in
//               the current class, the parent classes will be
//               searched, so the value returned may not actually be a
//               field within this class.  Returns NULL if there is no
//               such field defined.
////////////////////////////////////////////////////////////////////
DCField *DCClass::
get_field_by_name(const string &name) {
  FieldsByName::const_iterator ni;
  ni = _fields_by_name.find(name);
  if (ni != _fields_by_name.end()) {
    return (*ni).second;
  }

  // We didn't have such a field, so check our parents.
  Parents::iterator pi;
  for (pi = _parents.begin(); pi != _parents.end(); ++pi) {
    DCField *result = (*pi)->get_field_by_name(name);
    if (result != (DCField *)NULL) {
      return result;
    }
  }

  // Nobody knew what this field is.
  return (DCField *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::get_num_inherited_fields
//       Access: Public
//  Description: Returns the total number of field fields defined in
//               this class and all ancestor classes.
////////////////////////////////////////////////////////////////////
int DCClass::
get_num_inherited_fields() {
  if (!_parents.empty()) {
    // This won't work for multiple dclass inheritance.
    return _parents.front()->get_num_inherited_fields() + get_num_fields();
  }
  return get_num_fields();
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::get_inherited_field
//       Access: Public
//  Description: Returns the nth field field in the class and all of
//               its ancestors.  This *is* the field corresponding to
//               the given index number, since the fields are ordered
//               consecutively beginning at the earliest inherited
//               fields.
////////////////////////////////////////////////////////////////////
DCField *DCClass::
get_inherited_field(int n) {
  if (!_parents.empty()) {
    // This won't work for multiple dclass inheritance.
    int psize = _parents.front()->get_num_inherited_fields();
    if (n < psize) {
      return _parents.front()->get_inherited_field(n);
    }

    n -= psize;
  }
  return get_field(n);
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DCClass::
DCClass() {
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DCClass::
~DCClass() {
  Fields::iterator fi;
  for (fi = _fields.begin(); fi != _fields.end(); ++fi) {
    delete (*fi);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::write
//       Access: Public
//  Description: Generates a parseable description of the object to
//               the indicated output stream.
////////////////////////////////////////////////////////////////////
void DCClass::
write(ostream &out, bool brief, int indent_level) const {
  indent(out, indent_level)
    << "dclass " << _name;

  if (!_parents.empty()) {
    Parents::const_iterator pi = _parents.begin();
    out << " : " << (*pi)->_name;
    ++pi;
    while (pi != _parents.end()) {
      out << ", " << (*pi)->_name;
      ++pi;
    }
  }

  out << " {";
  if (!brief) {
    out << "  // index " << _number;
  }
  out << "\n";

  Fields::const_iterator fi;
  for (fi = _fields.begin(); fi != _fields.end(); ++fi) {
    (*fi)->write(out, brief, indent_level + 2);
  }

  indent(out, indent_level) << "};\n";
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::generate_hash
//       Access: Public, Virtual
//  Description: Accumulates the properties of this class into the
//               hash.
////////////////////////////////////////////////////////////////////
void DCClass::
generate_hash(HashGenerator &hashgen) const {
  hashgen.add_string(_name);

  hashgen.add_int(_parents.size());
  Parents::const_iterator pi;
  for (pi = _parents.begin(); pi != _parents.end(); ++pi) {
    hashgen.add_int((*pi)->get_number());
  }

  hashgen.add_int(_fields.size());
  Fields::const_iterator fi;
  for (fi = _fields.begin(); fi != _fields.end(); ++fi) {
    (*fi)->generate_hash(hashgen);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::add_field
//       Access: Public
//  Description: Adds the newly-allocated field to the class.  The
//               class becomes the owner of the pointer and will
//               delete it when it destructs.  Returns true if the
//               field is successfully added, or false if there was a
//               name conflict.
////////////////////////////////////////////////////////////////////
bool DCClass::
add_field(DCField *field) {
  bool inserted = _fields_by_name.insert
    (FieldsByName::value_type(field->_name, field)).second;

  if (!inserted) {
    return false;
  }

  field->_number = get_num_inherited_fields();
  _fields.push_back(field);
  return true;
}
