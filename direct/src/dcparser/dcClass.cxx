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

#include "dcClassParameter.h"

#ifdef WITHIN_PANDA
#include "pStatTimer.h"

#ifndef CPPPARSER
PStatCollector DCClass::_update_pcollector("App:Show code:readerPollTask:Update");
PStatCollector DCClass::_generate_pcollector("App:Show code:readerPollTask:Generate");
#endif  // CPPPARSER
#endif  // WITHIN_PANDA

////////////////////////////////////////////////////////////////////
//     Function: DCClass::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DCClass::
DCClass(const string &name, bool is_struct, bool bogus_class) : 
#ifdef WITHIN_PANDA
  _class_update_pcollector(_update_pcollector, name),
  _class_generate_pcollector(_generate_pcollector, name),
#endif
  _name(name),
  _is_struct(is_struct),
  _bogus_class(bogus_class)
{
  _number = -1;
  _constructor = NULL;
      
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
  if (_constructor != (DCField *)NULL) {
    delete _constructor;
  }

  Fields::iterator fi;
  for (fi = _fields.begin(); fi != _fields.end(); ++fi) {
    delete (*fi);
  }

#ifdef HAVE_PYTHON
  Py_XDECREF(_class_def);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::as_class
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DCClass *DCClass::
as_class() {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::as_class
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
const DCClass *DCClass::
as_class() const {
  return this;
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
//     Function: DCClass::has_constructor
//       Access: Published
//  Description: Returns true if this class has a constructor method,
//               false if it just uses the default constructor.
////////////////////////////////////////////////////////////////////
bool DCClass::
has_constructor() const {
  return (_constructor != (DCField *)NULL);
}
  
////////////////////////////////////////////////////////////////////
//     Function: DCClass::get_constructor
//       Access: Published
//  Description: Returns the constructor method for this class if it
//               is defined, or NULL if the class uses the default
//               constructor.
////////////////////////////////////////////////////////////////////
DCField *DCClass::
get_constructor() const {
  return _constructor;
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
  #ifndef NDEBUG //[
  if (n < 0 || n >= (int)_fields.size()) {
    cerr << *this << " "
         << "n:" << n << " _fields.size():"
         << (int)_fields.size() << endl;
    // __asm { int 3 }
  }
  #endif //]
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
//     Function : DCClass::output
//       Access : Published, Virtual
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void DCClass::
output(ostream &out) const {
  if (_is_struct) {
    out << "struct";
  } else {
    out << "dclass";
  }
  if (!_name.empty()) {
    out << " " << _name;
  }
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
    return Py_None;
  }

  Py_INCREF(_class_def);
  return _class_def;
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCClass::receive_update
//       Access: Published
//  Description: Extracts the update message out of the packer and
//               applies it to the indicated object by calling the
//               appropriate method.
////////////////////////////////////////////////////////////////////
void DCClass::
receive_update(PyObject *distobj, DatagramIterator &di) const {
#ifdef WITHIN_PANDA
  PStatTimer timer(((DCClass *)this)->_class_update_pcollector);
#endif
  DCPacker packer;
  packer.set_unpack_data(di.get_remaining_bytes());

  int field_id = packer.raw_unpack_uint16();
  DCField *field = get_inherited_field(field_id);
  nassertv_always(field != NULL);
  packer.begin_unpack(field);
  field->receive_update(packer, distobj);
  packer.end_unpack();

  di.skip_bytes(packer.get_num_unpacked_bytes());
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
receive_update_broadcast_required(PyObject *distobj, DatagramIterator &di) const {
#ifdef WITHIN_PANDA
  PStatTimer timer(((DCClass *)this)->_class_update_pcollector);
#endif
  DCPacker packer;
  packer.set_unpack_data(di.get_remaining_bytes());

  int num_fields = get_num_inherited_fields();
  for (int i = 0; i < num_fields && !PyErr_Occurred(); ++i) {
    DCField *field = get_inherited_field(i);
    if (field->as_molecular_field() == (DCMolecularField *)NULL &&
        field->is_required() && field->is_broadcast()) {
      packer.begin_unpack(field);
      field->receive_update(packer, distobj);
      if (!packer.end_unpack()) {
        break;
      }
    }
  }

  di.skip_bytes(packer.get_num_unpacked_bytes());
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
receive_update_all_required(PyObject *distobj, DatagramIterator &di) const {
#ifdef WITHIN_PANDA
  PStatTimer timer(((DCClass *)this)->_class_update_pcollector);
#endif
  DCPacker packer;
  packer.set_unpack_data(di.get_remaining_bytes());

  int num_fields = get_num_inherited_fields();
  for (int i = 0; i < num_fields && !PyErr_Occurred(); ++i) {
    DCField *field = get_inherited_field(i);
    if (field->as_molecular_field() == (DCMolecularField *)NULL &&
        field->is_required()) {
      packer.begin_unpack(field);
      field->receive_update(packer, distobj);
      if (!packer.end_unpack()) {
        break;
      }
    }
  }

  di.skip_bytes(packer.get_num_unpacked_bytes());
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
receive_update_other(PyObject *distobj, DatagramIterator &di) const {
#ifdef WITHIN_PANDA
  PStatTimer timer(((DCClass *)this)->_class_update_pcollector);
#endif
  int num_fields = di.get_uint16();
  for (int i = 0; i < num_fields && !PyErr_Occurred(); ++i) {
    receive_update(distobj, di);
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
  DCField *field = get_field_by_name(field_name);
  nassertv_always(field != NULL);

  DCPacker packer;
  packer.set_unpack_data(value_blob);
  packer.begin_unpack(field);
  field->receive_update(packer, distobj);
  packer.end_unpack();
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
  direct_update(distobj, field_name, datagram.get_message());
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
pack_required_field(Datagram &datagram, PyObject *distobj, 
                    const DCField *field) const {
  DCPacker packer;
  packer.begin_pack(field);
  if (!pack_required_field(packer, distobj, field)) {
    return false;
  }
  if (!packer.end_pack()) {
    return false;
  }

  datagram.append_data(packer.get_data(), packer.get_length());
  return true;
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCClass::pack_required_field
//       Access: Published
//  Description: Looks up the current value of the indicated field by
//               calling the appropriate get*() function, then packs
//               that value into the packer.  This field is
//               presumably either a required field or a specified
//               optional field, and we are building up a datagram for
//               the generate-with-required message.
//
//               Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCClass::
pack_required_field(DCPacker &packer, PyObject *distobj, 
                    const DCField *field) const {
  const DCParameter *parameter = field->as_parameter();
  if (parameter != (DCParameter *)NULL) {
    // This is the easy case: to pack a parameter, we just look on the
    // class object for the data element.
    string field_name = field->get_name();

    if (!PyObject_HasAttrString(distobj, (char *)field_name.c_str())) {
      // If the attribute is not defined, but the field has a default
      // value specified, quietly pack the default value.
      if (field->has_default_value()) {
        packer.pack_default_value();
        return true;
      }

      // If there is no default value specified, it's an error.
      ostringstream strm;
      strm << "Data element " << field_name
           << ", required by dc file for dclass " << get_name()
           << ", not defined on object";
      nassert_raise(strm.str());
      return false;
    }
    PyObject *result = 
      PyObject_GetAttrString(distobj, (char *)field_name.c_str());
    nassertr(result != (PyObject *)NULL, false);

    // Now pack the value into the datagram.
    bool pack_ok = parameter->pack_args(packer, result);
    Py_DECREF(result);
    
    return pack_ok;
  }

  if (field->as_molecular_field() != (DCMolecularField *)NULL) {
    ostringstream strm;
    strm << "Cannot pack molecular field " << field->get_name()
         << " for generate";
    nassert_raise(strm.str());
    return false;
  }

  const DCAtomicField *atom = field->as_atomic_field();
  nassertr(atom != (DCAtomicField *)NULL, false);

  // We need to get the initial value of this field.  There isn't a
  // good, robust way to get this; presently, we just mangle the
  // "setFoo()" name of the required field into "getFoo()" and call
  // that.
  string setter_name = atom->get_name();

  if (setter_name.empty()) {
    ostringstream strm;
    strm << "Required field is unnamed!";
    nassert_raise(strm.str());
    return false;
  }

  if (atom->get_num_elements() == 0) {
    // It sure doesn't make sense to have a required field with no
    // parameters.  What data, exactly, is required?
    ostringstream strm;
    strm << "Required field " << setter_name << " has no parameters!";
    nassert_raise(strm.str());
    return false;
  }
  
  string getter_name = setter_name;
  if (setter_name.substr(0, 3) == "set") {
    // If the original method started with "set", we mangle this
    // directly to "get".
    getter_name[0] = 'g';

  } else {
    // Otherwise, we add a "get" prefix, and capitalize the next
    // letter.
    getter_name = "get" + setter_name;
    getter_name[3] = toupper(getter_name[3]);
  }
  
  // Now we have to look up the getter on the distributed object
  // and call it.
  if (!PyObject_HasAttrString(distobj, (char *)getter_name.c_str())) {
    // As above, if there's no getter but the field has a default
    // value specified, quietly pack the default value.
    if (field->has_default_value()) {
      packer.pack_default_value();
      return true;
    }

    // Otherwise, with no default value it's an error.
    ostringstream strm;
    strm << "Distributed class " << get_name()
         << " doesn't have getter named " << getter_name
         << " to match required field " << setter_name;
    nassert_raise(strm.str());
    return false;
  }
  PyObject *func = 
    PyObject_GetAttrString(distobj, (char *)getter_name.c_str());
  nassertr(func != (PyObject *)NULL, false);
  
  PyObject *empty_args = PyTuple_New(0);
  PyObject *result = PyObject_CallObject(func, empty_args);
  Py_DECREF(empty_args);
  Py_DECREF(func);
  if (result == (PyObject *)NULL) {
    // We don't set this as an exception, since presumably the Python
    // method itself has already triggered a Python exception.
    cerr << "Error when calling " << getter_name << "\n";
    return false;
  }
  
  if (atom->get_num_elements() == 1) {
    // In this case, we expect the getter to return one object,
    // which we wrap up in a tuple.
    PyObject *tuple = PyTuple_New(1);
    PyTuple_SET_ITEM(tuple, 0, result);
    result = tuple;

  } else {
    // Otherwise, it had better already be a sequence or tuple of some
    // sort.
    if (!PySequence_Check(result)) {
      ostringstream strm;
      strm << "Since dclass " << get_name() << " method " << setter_name
           << " is declared to have multiple parameters, Python function " 
           << getter_name << " must return a list or tuple.\n";
      nassert_raise(strm.str());
      return false;
    }
  }
  
  // Now pack the arguments into the datagram.
  bool pack_ok = atom->pack_args(packer, result);
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
//     Function: DCClass::ai_format_update
//       Access: Published
//  Description: Generates a datagram containing the message necessary
//               to send an update for the indicated distributed
//               object from the AI.
////////////////////////////////////////////////////////////////////
Datagram DCClass::
ai_format_update(const string &field_name, int do_id, 
                 CHANNEL_TYPE to_id, CHANNEL_TYPE from_id, PyObject *args) const {
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
//     Function: DCClass::client_format_generate
//       Access: Published
//  Description: Generates a datagram containing the message necessary
//               to generate a new distributed object from the client.
//               This requires querying the object for the initial
//               value of its required fields.
//
//               optional_fields is a list of fieldNames to generate
//               in addition to the normal required fields.
////////////////////////////////////////////////////////////////////
Datagram DCClass::
client_format_generate(PyObject *distobj, int do_id, 
                   int zone_id, PyObject *optional_fields) const {
  DCPacker packer;

  //packer.raw_pack_uint8('A');

  bool has_optional_fields = (PyObject_IsTrue(optional_fields) != 0);

  if (has_optional_fields) {
    packer.raw_pack_uint16(CLIENT_CREATE_OBJECT_REQUIRED_OTHER);
  } else {
    packer.raw_pack_uint16(CLIENT_CREATE_OBJECT_REQUIRED);
  }

  packer.raw_pack_uint32(zone_id);
  packer.raw_pack_uint16(_number);
  packer.raw_pack_uint32(do_id);

  // Specify all of the required fields.
  int num_fields = get_num_inherited_fields();
  for (int i = 0; i < num_fields; i++) {
    DCField *field = get_inherited_field(i);
    if (field->is_required() && field->as_molecular_field() == NULL) {
      packer.begin_pack(field);
      if (!pack_required_field(packer, distobj, field)) {
        return Datagram();
      }
      packer.end_pack();
    }
  }

  // Also specify the optional fields.
  if (has_optional_fields) {
    int num_optional_fields = PySequence_Size(optional_fields);
    packer.raw_pack_uint16(num_optional_fields);

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
      packer.raw_pack_uint16(field->get_number());
      packer.begin_pack(field);
      if (!pack_required_field(packer, distobj, field)) {
        return Datagram();
      }
      packer.end_pack();
    }
  }

  return Datagram(packer.get_data(), packer.get_length());
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
                   int parent_id, int zone_id,
                   CHANNEL_TYPE district_channel_id, CHANNEL_TYPE from_channel_id,
                   PyObject *optional_fields) const {
  DCPacker packer;

  packer.RAW_PACK_CHANNEL(district_channel_id);
  packer.RAW_PACK_CHANNEL(from_channel_id);
  packer.raw_pack_uint8('A');

  bool has_optional_fields = (PyObject_IsTrue(optional_fields) != 0);

  if (has_optional_fields) {
    packer.raw_pack_uint16(STATESERVER_OBJECT_GENERATE_WITH_REQUIRED_OTHER);
  } else {
    packer.raw_pack_uint16(STATESERVER_OBJECT_GENERATE_WITH_REQUIRED);
  }
  
  // Parent is a bit overloaded; this parent is not about inheritance,
  // this one is about the visibility container parent, i.e. the zone
  // parent:
  if (parent_id) { // if wantOtpServer:
    packer.raw_pack_uint32(parent_id);
  }
  packer.raw_pack_uint32(zone_id);
  packer.raw_pack_uint16(_number);
  packer.raw_pack_uint32(do_id);

  // Specify all of the required fields.
  int num_fields = get_num_inherited_fields();
  for (int i = 0; i < num_fields; ++i) {
    DCField *field = get_inherited_field(i);
    if (field->is_required() && field->as_molecular_field() == NULL) {
      packer.begin_pack(field);
      if (!pack_required_field(packer, distobj, field)) {
        return Datagram();
      }
      packer.end_pack();
    }
  }

  // Also specify the optional fields.
  if (has_optional_fields) {
    int num_optional_fields = PySequence_Size(optional_fields);
    packer.raw_pack_uint16(num_optional_fields);

    for (int i = 0; i < num_optional_fields; ++i) {
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

      packer.raw_pack_uint16(field->get_number());

      packer.begin_pack(field);
      if (!pack_required_field(packer, distobj, field)) {
        return Datagram();
      }
      packer.end_pack();
    }
  }

  return Datagram(packer.get_data(), packer.get_length());
}
#endif  // HAVE_PYTHON
#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCClass::ai_database_generate
//       Access: Published
//  Description: Generates a datagram containing the message necessary
//               to create a new database distributed object from the AI.
//
// First Pass is to only incldue required values(with Defaults).                   
////////////////////////////////////////////////////////////////////
Datagram DCClass::
ai_database_generate_context(int context_id, 
                   int parent_id, int zone_id,
                   CHANNEL_TYPE database_server_id, CHANNEL_TYPE from_channel_id) const 
{
  DCPacker packer;
  packer.RAW_PACK_CHANNEL(database_server_id);
  packer.RAW_PACK_CHANNEL(from_channel_id);
  packer.raw_pack_uint8('A');
  packer.raw_pack_uint16(STATESERVER_OBJECT_CREATE_WITH_REQUIRED_CONTEXT);
  packer.raw_pack_uint32(parent_id);  
  packer.raw_pack_uint32(zone_id);
  packer.raw_pack_uint16(_number); // DCD class ID
  packer.raw_pack_uint32(context_id);

  // Specify all of the required fields.
  int num_fields = get_num_inherited_fields();
  for (int i = 0; i < num_fields; ++i) {
    DCField *field = get_inherited_field(i);
    if (field->is_required() && field->as_molecular_field() == NULL) 
    {
      packer.begin_pack(field);
      packer.pack_default_value();
      packer.end_pack();
    }
  }

  return Datagram(packer.get_data(), packer.get_length());
}
#endif  // HAVE_PYTHON

////////////////////////////////////////////////////////////////////
//     Function : DCClass::output
//       Access : Public, Virtual
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void DCClass::
output(ostream &out, bool brief) const {
  output_instance(out, brief, "", "", "");
}

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
    out << "struct";
  } else {
    out << "dclass";
  }
  if (!_name.empty()) {
    out << " " << _name;
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

  if (_constructor != (DCField *)NULL) {
    _constructor->write(out, brief, indent_level + 2);
  }

  Fields::const_iterator fi;
  for (fi = _fields.begin(); fi != _fields.end(); ++fi) {
    (*fi)->write(out, brief, indent_level + 2);

    /*
    if (true || (*fi)->has_default_value()) {
      indent(out, indent_level + 2) << "// = ";
      DCPacker packer;
      packer.set_unpack_data((*fi)->get_default_value());
      packer.begin_unpack(*fi);
      packer.unpack_and_format(out);
      if (!packer.end_unpack()) {
        out << "<error>";
      }
      out << "\n";
    }
    */
  }

  indent(out, indent_level) << "};\n";
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::output_instance
//       Access: Public
//  Description: Generates a parseable description of the object to
//               the indicated output stream.
////////////////////////////////////////////////////////////////////
void DCClass::
output_instance(ostream &out, bool brief, const string &prename, 
                const string &name, const string &postname) const {
  if (_is_struct) {
    out << "struct";
  } else {
    out << "dclass";
  }
  if (!_name.empty()) {
    out << " " << _name;
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

  if (_constructor != (DCField *)NULL) {
    _constructor->output(out, brief);
    out << "; ";
  }

  Fields::const_iterator fi;
  for (fi = _fields.begin(); fi != _fields.end(); ++fi) {
    (*fi)->output(out, brief);
    out << "; ";
  }

  out << "}";
  if (!prename.empty() || !name.empty() || !postname.empty()) {
    out << " " << prename << name << postname;
  }
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

  if (is_struct()) {
    hashgen.add_int(1);
  }

  hashgen.add_int(_parents.size());
  Parents::const_iterator pi;
  for (pi = _parents.begin(); pi != _parents.end(); ++pi) {
    hashgen.add_int((*pi)->get_number());
  }

  if (_constructor != (DCField *)NULL) {
    _constructor->generate_hash(hashgen);
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
  if (!field->get_name().empty()) {
    if (field->get_name() == _name) {
      // This field is a constructor.
      if (_constructor != (DCField *)NULL) {
        // We already have a constructor.
        return false;
      }
      if (field->as_atomic_field() == (DCAtomicField *)NULL) {
        // The constructor must be an atomic field.
        return false;
      }
      _constructor = field;
      _fields_by_name.insert
        (FieldsByName::value_type(field->get_name(), field));
      return true;
    }

    bool inserted = _fields_by_name.insert
      (FieldsByName::value_type(field->get_name(), field)).second;

    if (!inserted) {
      return false;
    }
  }

  if (!is_struct()) {
    field->set_number(get_num_inherited_fields());
  }
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

