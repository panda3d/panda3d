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
#include "dcAtomicField.h"
#include "hashGenerator.h"
#include "dcindent.h"
#include "dcmsgtypes.h"

////////////////////////////////////////////////////////////////////
//     Function: DCClass::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DCClass::
DCClass(const string &name, bool is_struct, bool bogus_class) : 
  _name(name),
  _is_struct(is_struct),
  _bogus_class(bogus_class)
{
  _number = -1;
      
#ifdef HAVE_PYTHON
  _class_def = NULL;
#endif
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

#ifdef HAVE_PYTHON
  Py_XDECREF(_class_def);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::get_name
//       Access: Published
//  Description: Returns the name of this class.
////////////////////////////////////////////////////////////////////
const string &DCClass::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::get_number
//       Access: Published
//  Description: Returns a unique index number associated with this
//               class.  This is defined implicitly when the .dc
//               file(s) are read.
////////////////////////////////////////////////////////////////////
int DCClass::
get_number() const {
  return _number;
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::has_parent
//       Access: Published
//  Description: Returns true if this class inherits from some other
//               class, false if it does not.
////////////////////////////////////////////////////////////////////
bool DCClass::
has_parent() const {
  return !_parents.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::get_parent
//       Access: Published
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
//       Access: Published
//  Description: Returns the number of fields defined directly in this
//               class, ignoring inheritance.
////////////////////////////////////////////////////////////////////
int DCClass::
get_num_fields() const {
  return _fields.size();
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::get_field
//       Access: Published
//  Description: Returns the nth field in the class.  This is not
//               necessarily the field with index n; this is the nth
//               field defined in the class directly, ignoring
//               inheritance.
////////////////////////////////////////////////////////////////////
DCField *DCClass::
get_field(int n) const {
  nassertr_always(n >= 0 && n < (int)_fields.size(), NULL);
  return _fields[n];
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::get_field_by_name
//       Access: Published
//  Description: Returns a pointer to the DCField that shares the
//               indicated name.  If the named field is not found in
//               the current class, the parent classes will be
//               searched, so the value returned may not actually be a
//               field within this class.  Returns NULL if there is no
//               such field defined.
////////////////////////////////////////////////////////////////////
DCField *DCClass::
get_field_by_name(const string &name) const {
  FieldsByName::const_iterator ni;
  ni = _fields_by_name.find(name);
  if (ni != _fields_by_name.end()) {
    return (*ni).second;
  }

  // We didn't have such a field, so check our parents.
  Parents::const_iterator pi;
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
//       Access: Published
//  Description: Returns the total number of field fields defined in
//               this class and all ancestor classes.
////////////////////////////////////////////////////////////////////
int DCClass::
get_num_inherited_fields() const {
  if (!_parents.empty()) {
    // This won't work for multiple dclass inheritance.
    return _parents.front()->get_num_inherited_fields() + get_num_fields();
  }
  return get_num_fields();
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::get_inherited_field
//       Access: Published
//  Description: Returns the nth field field in the class and all of
//               its ancestors.  This *is* the field corresponding to
//               the given index number, since the fields are ordered
//               consecutively beginning at the earliest inherited
//               fields.
////////////////////////////////////////////////////////////////////
DCField *DCClass::
get_inherited_field(int n) const {
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
//     Function: DCClass::is_struct
//       Access: Public
//  Description: Returns true if the class has been identified with
//               the "struct" keyword in the dc file, false if it was
//               declared with "dclass".
////////////////////////////////////////////////////////////////////
bool DCClass::
is_struct() const {
  return _is_struct;
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::is_bogus_class
//       Access: Public
//  Description: Returns true if the class has been flagged as a bogus
//               class.  This is set for classes that are generated by
//               the parser as placeholder for missing classes, as
//               when reading a partial file; it should not occur in a
//               normal valid dc file.
////////////////////////////////////////////////////////////////////
bool DCClass::
is_bogus_class() const {
  return _bogus_class;
}

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCClass::has_class_def
//       Access: Published
//  Description: Returns true if the DCClass object has an associated
//               Python class definition, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCClass::
has_class_def() const {
  return (_class_def != NULL);
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCClass::set_class_def
//       Access: Published
//  Description: Sets the class object associated with this
//               DistributedClass.  This object will be used to
//               construct new instances of the class.
////////////////////////////////////////////////////////////////////
void DCClass::
set_class_def(PyObject *class_def) {
  Py_XINCREF(class_def);
  Py_XDECREF(_class_def);
  _class_def = class_def;
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCClass::get_class_def
//       Access: Published
//  Description: Returns the class object that was previously
//               associated with this DistributedClass.  This will
//               return a new reference to the object.
////////////////////////////////////////////////////////////////////
PyObject *DCClass::
get_class_def() const {
  if (_class_def == NULL) {
    return Py_BuildValue("");
  }

  Py_INCREF(_class_def);
  return _class_def;
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCClass::receive_update
//       Access: Published
//  Description: Extracts the update message out of the datagram and
//               applies it to the indicated object by calling the
//               appropriate method.
////////////////////////////////////////////////////////////////////
void DCClass::
receive_update(PyObject *distobj, DatagramIterator &iterator) const {
  int field_id = iterator.get_uint16();
  DCField *field = get_inherited_field(field_id);
  nassertv_always(field != NULL);
  field->receive_update(distobj, iterator);
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCClass::receive_update_broadcast_required
//       Access: Published
//  Description: Processes a big datagram that includes all of the
//               "required" fields that are sent along with a normal
//               "generate with required" message.  This is all of the
//               atomic fields that are marked "broadcast required".
////////////////////////////////////////////////////////////////////
void DCClass::
receive_update_broadcast_required(PyObject *distobj, DatagramIterator &iterator) const {
  int num_fields = get_num_inherited_fields();
  for (int i = 0; i < num_fields && !PyErr_Occurred(); i++) {
    DCField *field = get_inherited_field(i);
    DCAtomicField *atom = field->as_atomic_field();
    if (atom != (DCAtomicField *)NULL &&
        atom->is_required() && atom->is_broadcast()) {
      atom->receive_update(distobj, iterator);
    }
  }
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCClass::receive_update_all_required
//       Access: Published
//  Description: Processes a big datagram that includes all of the
//               "required" fields that are sent when an avatar is
//               created.  This is all of the atomic fields that are
//               marked "required", whether they are broadcast or not.
////////////////////////////////////////////////////////////////////
void DCClass::
receive_update_all_required(PyObject *distobj, DatagramIterator &iterator) const {
  int num_fields = get_num_inherited_fields();
  for (int i = 0; i < num_fields && !PyErr_Occurred(); i++) {
    DCField *field = get_inherited_field(i);
    DCAtomicField *atom = field->as_atomic_field();
    if (atom != (DCAtomicField *)NULL && atom->is_required()) {
      atom->receive_update(distobj, iterator);
    }
  }
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCClass::receive_update_other
//       Access: Published
//  Description: Processes a datagram that lists some additional
//               fields that are broadcast in one chunk.
////////////////////////////////////////////////////////////////////
void DCClass::
receive_update_other(PyObject *distobj, DatagramIterator &iterator) const {
  int num_fields = iterator.get_uint16();
  for (int i = 0; i < num_fields && !PyErr_Occurred(); i++) {
    receive_update(distobj, iterator);
  }
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCClass::direct_update
//       Access: Published
//  Description: Processes an update for a named field from a packed
//               value blob.
////////////////////////////////////////////////////////////////////
void DCClass::
direct_update(PyObject *distobj, const string &field_name, 
              const string &value_blob) {
  Datagram datagram(value_blob);
  direct_update(distobj, field_name, datagram);
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCClass::direct_update
//       Access: Published
//  Description: Processes an update for a named field from a packed
//               datagram.
////////////////////////////////////////////////////////////////////
void DCClass::
direct_update(PyObject *distobj, const string &field_name, 
              const Datagram &datagram) {
  DCField *field = get_field_by_name(field_name);
  nassertv_always(field != NULL);
  DatagramIterator iterator(datagram);
  field->receive_update(distobj, iterator);
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCClass::pack_required_field
//       Access: Published
//  Description: Looks up the current value of the indicated field by
//               calling the appropriate get*() function, then packs
//               that value into the datagram.  This field is
//               presumably either a required field or a specified
//               optional field, and we are building up a datagram for
//               the generate-with-required message.
//
//               Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCClass::
pack_required_field(Datagram &dg, PyObject *distobj, DCField *field) const {
  DCAtomicField *atom = field->as_atomic_field();
  if (atom == (DCAtomicField *)NULL) {
    ostringstream strm;
    strm << "Cannot pack non-atomic field " << field->get_name()
         << " for generate";
    nassert_raise(strm.str());
    return false;
  }

  // We need to get the initial value of this field.  There isn't a
  // good, robust way to get this; presently, we just mangle the
  // "setFoo()" name of the required field into "getFoo()" and call
  // that.
  string set_name = atom->get_name();

  if (atom->get_num_elements() == 0) {
    // It sure doesn't make sense to have a required field with no
    // parameters.  What data, exactly, is required?
    ostringstream strm;
    strm << "Required field " << set_name << " has no parameters!";
    nassert_raise(strm.str());
    return false;
  }
  
  if (set_name.substr(0, 3) != string("set")) {
    // This is required to suit our set/get mangling convention.
    ostringstream strm;
    strm << "Required field " << set_name << " does not begin with 'set'";
    nassert_raise(strm.str());
    return false;
  }
  string get_name = set_name;
  get_name[0] = 'g';
  
  // Now we have to look up the getter on the distributed object
  // and call it.
  if (!PyObject_HasAttrString(distobj, (char *)get_name.c_str())) {
    ostringstream strm;
    strm << "Required field " << set_name
         << " doesn't have matching field named " << get_name;
    nassert_raise(strm.str());
    return false;
  }
  PyObject *func = 
    PyObject_GetAttrString(distobj, (char *)get_name.c_str());
  nassertr(func != (PyObject *)NULL, false);
  
  PyObject *empty_args = PyTuple_New(0);
  PyObject *result = PyObject_CallObject(func, empty_args);
  Py_DECREF(empty_args);
  Py_DECREF(func);
  if (result == (PyObject *)NULL) {
    cerr << "Error when calling " << get_name << "\n";
    return false;
  }
  
  if (atom->get_num_elements() == 1) {
    // In this case, we expect the getter to return one object,
    // which we wrap up in a tuple.
    PyObject *tuple = PyTuple_New(1);
    PyTuple_SET_ITEM(tuple, 0, result);
    result = tuple;
  }        
  
  // Now pack the arguments into the datagram.
  bool pack_ok = atom->pack_args(dg, result);
  Py_DECREF(result);

  return pack_ok;
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCClass::client_format_update
//       Access: Published
//  Description: Generates a datagram containing the message necessary
//               to send an update for the indicated distributed
//               object from the client.
////////////////////////////////////////////////////////////////////
Datagram DCClass::
client_format_update(const string &field_name, int do_id, 
                     PyObject *args) const {
  DCField *field = get_field_by_name(field_name);
  if (field == (DCField *)NULL) {
    ostringstream strm;
    strm << "No field named " << field_name << " in class " << get_name()
         << "\n";
    nassert_raise(strm.str());
    return Datagram();
  }
  return field->client_format_update(do_id, args);
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCClass::al_format_update
//       Access: Published
//  Description: Generates a datagram containing the message necessary
//               to send an update for the indicated distributed
//               object from the AI.
////////////////////////////////////////////////////////////////////
Datagram DCClass::
ai_format_update(const string &field_name, int do_id, 
                 int to_id, int from_id, PyObject *args) const {
  DCField *field = get_field_by_name(field_name);
  if (field == (DCField *)NULL) {
    ostringstream strm;
    strm << "No field named " << field_name << " in class " << get_name()
         << "\n";
    nassert_raise(strm.str());
    return Datagram();
  }
  return field->ai_format_update(do_id, to_id, from_id, args);
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCClass::ai_format_generate
//       Access: Published
//  Description: Generates a datagram containing the message necessary
//               to generate a new distributed object from the AI.
//               This requires querying the object for the initial
//               value of its required fields.
//
//               optional_fields is a list of fieldNames to generate
//               in addition to the normal required fields.
////////////////////////////////////////////////////////////////////
Datagram DCClass::
ai_format_generate(PyObject *distobj, int do_id, 
                   int zone_id, int district_id, int from_channel_id,
                   PyObject *optional_fields) const {
  Datagram dg;
  dg.add_uint32(district_id);
  dg.add_uint32(from_channel_id);
  dg.add_uint8('A');

  bool has_optional_fields = (PyObject_IsTrue(optional_fields) != 0);

  if (has_optional_fields) {
    dg.add_uint16(STATESERVER_OBJECT_GENERATE_WITH_REQUIRED_OTHER);
  } else {
    dg.add_uint16(STATESERVER_OBJECT_GENERATE_WITH_REQUIRED);
  }

  dg.add_uint32(zone_id);
  dg.add_uint16(_number);
  dg.add_uint32(do_id);

  // Specify all of the required fields.
  int num_fields = get_num_inherited_fields();
  for (int i = 0; i < num_fields; i++) {
    DCField *field = get_inherited_field(i);
    DCAtomicField *atom = field->as_atomic_field();
    if (atom != (DCAtomicField *)NULL && atom->is_required()) {
      if (!pack_required_field(dg, distobj, atom)) {
        return Datagram();
      }
    }
  }

  // Also specify the optional fields.
  if (has_optional_fields) {
    int num_optional_fields = PySequence_Size(optional_fields);
    dg.add_uint16(num_optional_fields);

    for (int i = 0; i < num_optional_fields; i++) {
      PyObject *py_field_name = PySequence_GetItem(optional_fields, i);
      string field_name = PyString_AsString(py_field_name);
      Py_XDECREF(py_field_name);

      DCField *field = get_field_by_name(field_name);
      if (field == (DCField *)NULL) {
        ostringstream strm;
        strm << "No field named " << field_name << " in class " << get_name()
             << "\n";
        nassert_raise(strm.str());
        return Datagram();
      }

      if (!pack_required_field(dg, distobj, field)) {
        return Datagram();
      }
    }
  }

  return dg;
}
#endif  // HAVE_PYTHON

////////////////////////////////////////////////////////////////////
//     Function: DCClass::write
//       Access: Public, Virtual
//  Description: Generates a parseable description of the object to
//               the indicated output stream.
////////////////////////////////////////////////////////////////////
void DCClass::
write(ostream &out, bool brief, int indent_level) const {
  indent(out, indent_level);
  if (_is_struct) {
    out << "struct " << _name;
  } else {
    out << "dclass " << _name;
  }

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
  if (!brief && _number >= 0) {
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
    (FieldsByName::value_type(field->get_name(), field)).second;

  if (!inserted) {
    return false;
  }

  field->set_number(get_num_inherited_fields());
  _fields.push_back(field);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::add_parent
//       Access: Public
//  Description: Adds a new parent to the inheritance hierarchy of the
//               class.  This is normally called only during parsing.
////////////////////////////////////////////////////////////////////
void DCClass::
add_parent(DCClass *parent) {
  _parents.push_back(parent);
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::set_number
//       Access: Public
//  Description: Assigns the unique number to this class.  This is
//               normally called only by the DCFile interface as the
//               class is added.
////////////////////////////////////////////////////////////////////
void DCClass::
set_number(int number) {
  _number = number;
}

