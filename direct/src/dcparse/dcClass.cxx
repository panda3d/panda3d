// Filename: dcClass.cxx
// Created by:  drose (05Oct00)
// 
////////////////////////////////////////////////////////////////////

#include "dcClass.h"
#include "indent.h"

#include <assert.h>

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
  assert(has_parent());
  return _parents.front();
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::get_num_atomics
//       Access: Public
//  Description: Returns the number of atomic fields defined directly
//               in this class, ignoring inheritance.
////////////////////////////////////////////////////////////////////
int DCClass::
get_num_atomics() {
  return _atomic_fields.size();
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::get_atomic
//       Access: Public
//  Description: Returns the nth atomic field in the class.  This is
//               not necessarily the field with index n; this is the
//               nth field defined in the class directly, ignoring
//               inheritance.
////////////////////////////////////////////////////////////////////
DCAtomicField *DCClass::
get_atomic(int n) {
  assert(n >= 0 && n < (int)_atomic_fields.size());
  return _atomic_fields[n];
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::get_atomic_by_name
//       Access: Public
//  Description: Returns a pointer to the DCAtomicField that shares
//               the indicated name.  If the named field is not found
//               in the current class, the parent classes will be
//               searched, so the value returned may not actually be a
//               field within this class.  Returns NULL if there is no
//               such field defined.
////////////////////////////////////////////////////////////////////
DCAtomicField *DCClass::
get_atomic_by_name(const string &name) {
  AtomicsByName::const_iterator ni;
  ni = _atomics_by_name.find(name);
  if (ni != _atomics_by_name.end()) {
    return (*ni).second;
  }

  // We didn't have such a field, so check our parents.
  Parents::iterator pi;
  for (pi = _parents.begin(); pi != _parents.end(); ++pi) {
    DCAtomicField *result = (*pi)->get_atomic_by_name(name);
    if (result != (DCAtomicField *)NULL) {
      return result;
    }
  }

  // Nobody knew what this field is.
  return (DCAtomicField *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::get_num_inherited_atomics
//       Access: Public
//  Description: Returns the total number of atomic fields defined in
//               this class and all ancestor classes.
////////////////////////////////////////////////////////////////////
int DCClass::
get_num_inherited_atomics() {
  if (!_parents.empty()) {
    // This won't work for multiple dclass inheritance.
    return _parents.front()->get_num_atomics() + get_num_atomics();
  }
  return get_num_atomics();
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::get_inherited_atomic
//       Access: Public
//  Description: Returns the nth atomic field in the class and all of
//               its ancestors.  This *is* the field corresponding to
//               the given index number, since the fields are ordered
//               consecutively beginning at the earliest inherited
//               fields.
////////////////////////////////////////////////////////////////////
DCAtomicField *DCClass::
get_inherited_atomic(int n) {
  if (!_parents.empty()) {
    // This won't work for multiple dclass inheritance.
    n -= _parents.front()->get_num_inherited_atomics();
  }
  return get_atomic(n);
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::get_num_moleculars
//       Access: Public
//  Description: Returns the number of molecular fields defined directly
//               in this class, ignoring inheritance.
////////////////////////////////////////////////////////////////////
int DCClass::
get_num_moleculars() {
  return _molecular_fields.size();
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::get_molecular
//       Access: Public
//  Description: Returns the nth molecular field in the class.  This is
//               not necessarily the field with index n; this is the
//               nth field defined in the class directly, ignoring
//               inheritance.
////////////////////////////////////////////////////////////////////
DCMolecularField *DCClass::
get_molecular(int n) {
  assert(n >= 0 && n < (int)_molecular_fields.size());
  return _molecular_fields[n];
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::get_molecular_by_name
//       Access: Public
//  Description: Returns a pointer to the DCMolecularField that shares
//               the indicated name.  If the named field is not found
//               in the current class, the parent classes will be
//               searched, so the value returned may not actually be a
//               field within this class.  Returns NULL if there is no
//               such field defined.
////////////////////////////////////////////////////////////////////
DCMolecularField *DCClass::
get_molecular_by_name(const string &name) {
  MolecularsByName::const_iterator ni;
  ni = _moleculars_by_name.find(name);
  if (ni != _moleculars_by_name.end()) {
    return (*ni).second;
  }

  // We didn't have such a field, so check our parents.
  Parents::iterator pi;
  for (pi = _parents.begin(); pi != _parents.end(); ++pi) {
    DCMolecularField *result = (*pi)->get_molecular_by_name(name);
    if (result != (DCMolecularField *)NULL) {
      return result;
    }
  }

  // Nobody knew what this field is.
  return (DCMolecularField *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::get_num_inherited_moleculars
//       Access: Public
//  Description: Returns the total number of molecular fields defined in
//               this class and all ancestor classes.
////////////////////////////////////////////////////////////////////
int DCClass::
get_num_inherited_moleculars() {
  if (!_parents.empty()) {
    // This won't work for multiple dclass inheritance.
    return _parents.front()->get_num_moleculars() + get_num_moleculars();
  }
  return get_num_moleculars();
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::get_inherited_molecular
//       Access: Public
//  Description: Returns the nth molecular field in the class and all of
//               its ancestors.  This *is* the field corresponding to
//               the given index number, since the fields are ordered
//               consecutively beginning at the earliest inherited
//               fields.
////////////////////////////////////////////////////////////////////
DCMolecularField *DCClass::
get_inherited_molecular(int n) {
  if (!_parents.empty()) {
    // This won't work for multiple dclass inheritance.
    n -= _parents.front()->get_num_inherited_moleculars();
  }
  return get_molecular(n);
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
  AtomicFields::iterator ai;
  for (ai = _atomic_fields.begin(); ai != _atomic_fields.end(); ++ai) {
    delete (*ai);
  }
  MolecularFields::iterator mi;
  for (mi = _molecular_fields.begin(); mi != _molecular_fields.end(); ++mi) {
    delete (*mi);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::write
//       Access: Public
//  Description: Generates a parseable description of the object to
//               the indicated output stream.
////////////////////////////////////////////////////////////////////
void DCClass::
write(ostream &out, int indent_level) const {
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
  out << " {  // index " << _number << "\n";

  AtomicFields::const_iterator ai;
  for (ai = _atomic_fields.begin(); ai != _atomic_fields.end(); ++ai) {
    (*ai)->write(out, indent_level + 2);
  }

  MolecularFields::const_iterator mi;
  for (mi = _molecular_fields.begin(); mi != _molecular_fields.end(); ++mi) {
    (*mi)->write(out, indent_level + 2);
  }

  indent(out, indent_level) << "};\n";
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::add_field
//       Access: Public
//  Description: Adds the newly-allocated atomic field to the class.
//               The class becomes the owner of the pointer and will
//               delete it when it destructs.  Returns true if the
//               field is successfully added, or false if there was a
//               name conflict.
////////////////////////////////////////////////////////////////////
bool DCClass::
add_field(DCAtomicField *field) {
  bool inserted = _atomics_by_name.insert
    (AtomicsByName::value_type(field->_name, field)).second;

  if (!inserted) {
    return false;
  }

  field->_number = get_num_inherited_atomics();
  _atomic_fields.push_back(field);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::add_field
//       Access: Public
//  Description: Adds the newly-allocated molecular field to the class.
//               The class becomes the owner of the pointer and will
//               delete it when it destructs.  Returns true if the
//               field is successfully added, or false if there was a
//               name conflict.
////////////////////////////////////////////////////////////////////
bool DCClass::
add_field(DCMolecularField *field) {
  bool inserted = _moleculars_by_name.insert
    (MolecularsByName::value_type(field->_name, field)).second;

  if (!inserted) {
    return false;
  }

  field->_number = get_num_inherited_moleculars();
  _molecular_fields.push_back(field);
  return true;
}
