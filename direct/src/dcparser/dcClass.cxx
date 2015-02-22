// Filename: dcClass.cxx
// Created by:  drose (05Oct00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "dcClass.h"
#include "dcFile.h"
#include "dcAtomicField.h"
#include "hashGenerator.h"
#include "dcindent.h"
#include "dcmsgtypes.h"

#include "dcClassParameter.h"
#include <algorithm>

#ifdef WITHIN_PANDA
#include "pStatTimer.h"

#ifndef CPPPARSER
PStatCollector DCClass::_update_pcollector("App:Show code:readerPollTask:Update");
PStatCollector DCClass::_generate_pcollector("App:Show code:readerPollTask:Generate");
#endif  // CPPPARSER

ConfigVariableBool dc_multiple_inheritance
("dc-multiple-inheritance", true,
 PRC_DESC("Set this true to support multiple inheritance in the dc file.  "
          "If this is false, the old way, multiple inheritance is not "
          "supported, but field numbers will be numbered sequentially, "
          "which may be required to support old code that assumed this."));

ConfigVariableBool dc_virtual_inheritance
("dc-virtual-inheritance", true,
 PRC_DESC("Set this true to support proper virtual inheritance in the "
          "dc file, so that diamond-of-death type constructs can be used.  "
          "This also enables shadowing (overloading) of inherited method "
          "names from a base class."));

ConfigVariableBool dc_sort_inheritance_by_file
("dc-sort-inheritance-by-file", true,
 PRC_DESC("This is a temporary hack.  This should be true if you are using "
          "version 1.42 of the otp_server.exe binary, which sorted inherited "
          "fields based on the order of the classes within the DC file, "
          "rather than based on the order in which the references are made "
          "within the class."));


#endif  // WITHIN_PANDA

class SortFieldsByIndex {
public:
  inline bool operator ()(const DCField *a, const DCField *b) const {
    return a->get_number() < b->get_number();
  }
};

////////////////////////////////////////////////////////////////////
//     Function: DCClass::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DCClass::
DCClass(DCFile *dc_file, const string &name, bool is_struct, bool bogus_class) : 
#ifdef WITHIN_PANDA
  _class_update_pcollector(_update_pcollector, name),
  _class_generate_pcollector(_generate_pcollector, name),
#endif
  _dc_file(dc_file),
  _name(name),
  _is_struct(is_struct),
  _bogus_class(bogus_class)
{
  _number = -1;
  _constructor = NULL;
      
#ifdef HAVE_PYTHON
  _class_def = NULL;
  _owner_class_def = NULL;
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
  Py_XDECREF(_owner_class_def);
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
//     Function: DCClass::get_num_parents
//       Access: Published
//  Description: Returns the number of base classes this class
//               inherits from.
////////////////////////////////////////////////////////////////////
int DCClass::
get_num_parents() const {
  return _parents.size();
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::get_parent
//       Access: Published
//  Description: Returns the nth parent class this class inherits
//               from.
////////////////////////////////////////////////////////////////////
DCClass *DCClass::
get_parent(int n) const {
  nassertr(n >= 0 && n < (int)_parents.size(), NULL);
  return _parents[n];
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
//     Function: DCClass::get_field_by_index
//       Access: Published
//  Description: Returns a pointer to the DCField that has the
//               indicated index number.  If the numbered field is not
//               found in the current class, the parent classes will
//               be searched, so the value returned may not actually
//               be a field within this class.  Returns NULL if there
//               is no such field defined.
////////////////////////////////////////////////////////////////////
DCField *DCClass::
get_field_by_index(int index_number) const {
  FieldsByIndex::const_iterator ni;
  ni = _fields_by_index.find(index_number);
  if (ni != _fields_by_index.end()) {
    return (*ni).second;
  }

  // We didn't have such a field, so check our parents.
  Parents::const_iterator pi;
  for (pi = _parents.begin(); pi != _parents.end(); ++pi) {
    DCField *result = (*pi)->get_field_by_index(index_number);
    if (result != (DCField *)NULL) {
      // Cache this result for future lookups.
      ((DCClass *)this)->_fields_by_index[index_number] = result;
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
  if (dc_multiple_inheritance && dc_virtual_inheritance && 
      _dc_file != (DCFile *)NULL) {
    _dc_file->check_inherited_fields();
    if (_inherited_fields.empty()) {
      ((DCClass *)this)->rebuild_inherited_fields();
    }

    // This assertion causes trouble when we are only parsing an
    // incomplete DC file.
    //nassertr(is_bogus_class() || !_inherited_fields.empty(), 0);
    return (int)_inherited_fields.size();

  } else {
    int num_fields = get_num_fields();

    Parents::const_iterator pi;
    for (pi = _parents.begin(); pi != _parents.end(); ++pi) {
      num_fields += (*pi)->get_num_inherited_fields();
    }
    
    return num_fields;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::get_inherited_field
//       Access: Published
//  Description: Returns the nth field field in the class and all of
//               its ancestors.  
//
//               This *used* to be the same thing as
//               get_field_by_index(), back when the fields were
//               numbered sequentially within a class's inheritance
//               hierarchy.  Now that fields have a globally unique
//               index number, this is no longer true.
////////////////////////////////////////////////////////////////////
DCField *DCClass::
get_inherited_field(int n) const {
  if (dc_multiple_inheritance && dc_virtual_inheritance && 
      _dc_file != (DCFile *)NULL) {
    _dc_file->check_inherited_fields();
    if (_inherited_fields.empty()) {
      ((DCClass *)this)->rebuild_inherited_fields();
    }
    nassertr(n >= 0 && n < (int)_inherited_fields.size(), NULL);
    return _inherited_fields[n];

  } else {
    Parents::const_iterator pi;
    for (pi = _parents.begin(); pi != _parents.end(); ++pi) {
      int psize = (*pi)->get_num_inherited_fields();
      if (n < psize) {
        return (*pi)->get_inherited_field(n);
      }
      
      n -= psize;
    }
    
    return get_field(n);
  }
}

////////////////////////////////////////////////////////////////////
//     Function : DCClass::inherits_from_bogus_class
//       Access : Published
//  Description : Returns true if this class, or any class in the
//                inheritance heirarchy for this class, is a "bogus"
//                class--a forward reference to an as-yet-undefined
//                class.
////////////////////////////////////////////////////////////////////
bool DCClass::
inherits_from_bogus_class() const {
  if (is_bogus_class()) {
    return true;
  }

  Parents::const_iterator pi;
  for (pi = _parents.begin(); pi != _parents.end(); ++pi) {
    if ((*pi)->inherits_from_bogus_class()) {
      return true;
    }
  }

  return false;
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
    Py_INCREF(Py_None);
    return Py_None;
  }

  Py_INCREF(_class_def);
  return _class_def;
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCClass::has_owner_class_def
//       Access: Published
//  Description: Returns true if the DCClass object has an associated
//               Python owner class definition, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCClass::
has_owner_class_def() const {
  return (_owner_class_def != NULL);
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCClass::set_owner_class_def
//       Access: Published
//  Description: Sets the owner class object associated with this
//               DistributedClass.  This object will be used to
//               construct new owner instances of the class.
////////////////////////////////////////////////////////////////////
void DCClass::
set_owner_class_def(PyObject *owner_class_def) {
  Py_XINCREF(owner_class_def);
  Py_XDECREF(_owner_class_def);
  _owner_class_def = owner_class_def;
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCClass::get_owner_class_def
//       Access: Published
//  Description: Returns the owner class object that was previously
//               associated with this DistributedClass.  This will
//               return a new reference to the object.
////////////////////////////////////////////////////////////////////
PyObject *DCClass::
get_owner_class_def() const {
  if (_owner_class_def == NULL) {
    Py_INCREF(Py_None);
    return Py_None;
  }

  Py_INCREF(_owner_class_def);
  return _owner_class_def;
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
    const char *data = (const char *)di.get_datagram().get_data();
    packer.set_unpack_data(data + di.get_current_index(),
                           di.get_remaining_size(), false);

    int field_id = packer.raw_unpack_uint16();
    DCField *field = get_field_by_index(field_id);
    if (field == (DCField *)NULL) {
            ostringstream strm;
            strm
                << "Received update for field " << field_id << ", not in class "
                << get_name();
            nassert_raise(strm.str());
            return;
    }

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
  const char *data = (const char *)di.get_datagram().get_data();
  packer.set_unpack_data(data + di.get_current_index(),
                         di.get_remaining_size(), false);

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
//     Function: DCClass::receive_update_broadcast_required_owner
//       Access: Published
//  Description: Processes a big datagram that includes all of the
//               "required" fields that are sent along with a normal
//               "generate with required" message.  This is all of the
//               atomic fields that are marked "broadcast ownrecv". Should
//               be used for 'owner-view' objects.
////////////////////////////////////////////////////////////////////
void DCClass::
receive_update_broadcast_required_owner(PyObject *distobj,
                                        DatagramIterator &di) const {
#ifdef WITHIN_PANDA
  PStatTimer timer(((DCClass *)this)->_class_update_pcollector);
#endif
  DCPacker packer;
  const char *data = (const char *)di.get_datagram().get_data();
  packer.set_unpack_data(data + di.get_current_index(),
                         di.get_remaining_size(), false);

  int num_fields = get_num_inherited_fields();
  for (int i = 0; i < num_fields && !PyErr_Occurred(); ++i) {
    DCField *field = get_inherited_field(i);
    if (field->as_molecular_field() == (DCMolecularField *)NULL &&
        field->is_required() && (field->is_ownrecv() || field->is_broadcast())) {
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
  const char *data = (const char *)di.get_datagram().get_data();
  packer.set_unpack_data(data + di.get_current_index(),
                         di.get_remaining_size(), false);

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
client_format_update(const string &field_name, DOID_TYPE do_id, 
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
ai_format_update(const string &field_name, DOID_TYPE do_id, 
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
//     Function: DCClass::ai_format_update_msg_type
//       Access: Published
//  Description: Generates a datagram containing the message necessary
//               to send an update, using the indicated msg type
//               for the indicated distributed
//               object from the AI.
////////////////////////////////////////////////////////////////////
Datagram DCClass::
ai_format_update_msg_type(const string &field_name, DOID_TYPE do_id, 
                 CHANNEL_TYPE to_id, CHANNEL_TYPE from_id, int msg_type, PyObject *args) const {
  DCField *field = get_field_by_name(field_name);
  if (field == (DCField *)NULL) {
    ostringstream strm;
    strm << "No field named " << field_name << " in class " << get_name()
         << "\n";
    nassert_raise(strm.str());
    return Datagram();
  }

  return field->ai_format_update_msg_type(do_id, to_id, from_id, msg_type, args);
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: DCClass::client_format_generate_CMU
//       Access: Published
//  Description: Generates a datagram containing the message necessary
//               to generate a new distributed object from the client.
//               This requires querying the object for the initial
//               value of its required fields.
//
//               optional_fields is a list of fieldNames to generate
//               in addition to the normal required fields.
//
//               This method is only called by the CMU implementation.
////////////////////////////////////////////////////////////////////
Datagram DCClass::
client_format_generate_CMU(PyObject *distobj, DOID_TYPE do_id, 
                           ZONEID_TYPE zone_id,
                           PyObject *optional_fields) const {
  DCPacker packer;

  packer.raw_pack_uint16(CLIENT_OBJECT_GENERATE_CMU);

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
  int num_optional_fields = 0;
  if (PyObject_IsTrue(optional_fields)) {
    num_optional_fields = PySequence_Size(optional_fields);
  }
  packer.raw_pack_uint16(num_optional_fields);

  for (int i = 0; i < num_optional_fields; i++) {
    PyObject *py_field_name = PySequence_GetItem(optional_fields, i);
#if PY_MAJOR_VERSION >= 3
    string field_name = PyUnicode_AsUTF8(py_field_name);
#else
    string field_name = PyString_AsString(py_field_name);
#endif
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
ai_format_generate(PyObject *distobj, DOID_TYPE do_id, 
                   DOID_TYPE parent_id, ZONEID_TYPE zone_id,
                   CHANNEL_TYPE district_channel_id, CHANNEL_TYPE from_channel_id,
                   PyObject *optional_fields) const {
  DCPacker packer;

  packer.raw_pack_uint8(1);
  packer.RAW_PACK_CHANNEL(district_channel_id);
  packer.RAW_PACK_CHANNEL(from_channel_id);
    //packer.raw_pack_uint8('A');

  bool has_optional_fields = (PyObject_IsTrue(optional_fields) != 0);

  if (has_optional_fields) {
    packer.raw_pack_uint16(STATESERVER_CREATE_OBJECT_WITH_REQUIRED_OTHER);
  } else {
    packer.raw_pack_uint16(STATESERVER_CREATE_OBJECT_WITH_REQUIRED);
  }

  packer.raw_pack_uint32(do_id);
  // Parent is a bit overloaded; this parent is not about inheritance,
  // this one is about the visibility container parent, i.e. the zone
  // parent:
  packer.raw_pack_uint32(parent_id);
  packer.raw_pack_uint32(zone_id);
  packer.raw_pack_uint16(_number);

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
#if PY_MAJOR_VERSION >= 3
      string field_name = PyUnicode_AsUTF8(py_field_name);
#else
      string field_name = PyString_AsString(py_field_name);
#endif
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
    if (!(*fi)->is_bogus_field()) {
      (*fi)->write(out, brief, indent_level + 2);

      /*
      if (true || (*fi)->has_default_value()) {
        indent(out, indent_level + 2) << "// = ";
        DCPacker packer;
        packer.set_unpack_data((*fi)->get_default_value());
        packer.begin_unpack(*fi);
        packer.unpack_and_format(out, false);
        if (!packer.end_unpack()) {
          out << "<error>";
        }
        out << "\n";
      }
      */
    }
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
    if (!(*fi)->is_bogus_field()) {
      (*fi)->output(out, brief);
      out << "; ";
    }
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
//     Function: DCClass::clear_inherited_fields
//       Access: Public
//  Description: Empties the list of inherited fields for the class,
//               so that it may be rebuilt.  This is normally only
//               called by DCFile::rebuild_inherited_fields().
////////////////////////////////////////////////////////////////////
void DCClass::
clear_inherited_fields() {
  _inherited_fields.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::rebuild_inherited_fields
//       Access: Public
//  Description: Recomputes the list of inherited fields for the class.
////////////////////////////////////////////////////////////////////
void DCClass::
rebuild_inherited_fields() {
  typedef pset<string> Names;
  Names names;

  _inherited_fields.clear();
  
  // First, all of the inherited fields from our parent are at the top
  // of the list.
  Parents::const_iterator pi;
  for (pi = _parents.begin(); pi != _parents.end(); ++pi) {
    const DCClass *parent = (*pi);
    int num_inherited_fields = parent->get_num_inherited_fields();
    for (int i = 0; i < num_inherited_fields; ++i) {
      DCField *field = parent->get_inherited_field(i);
      if (field->get_name().empty()) {
        // Unnamed fields are always inherited.  Except in the hack case.
        if (!dc_sort_inheritance_by_file) {
          _inherited_fields.push_back(field);
        }

      } else {
        bool inserted = names.insert(field->get_name()).second;
        if (inserted) {
          // The earlier parent shadows the later parent.
          _inherited_fields.push_back(field);
        }
      }
    }
  }

  // Now add the local fields at the end of the list.  If any fields
  // in this list were already defined by a parent, we will shadow the
  // parent definition (that is, remove the parent's field from our
  // list of inherited fields).
  Fields::const_iterator fi;
  for (fi = _fields.begin(); fi != _fields.end(); ++fi) {
    DCField *field = (*fi);
    if (field->get_name().empty()) {
      // Unnamed fields are always added. 
     _inherited_fields.push_back(field);

    } else {
      bool inserted = names.insert(field->get_name()).second;
      if (!inserted) {
        // This local field shadows an inherited field.  Remove the
        // parent's field from our list.
        shadow_inherited_field(field->get_name());
      }

      // Now add the local field.
      _inherited_fields.push_back(field);
    }
  }

  if (dc_sort_inheritance_by_file) {
    // Temporary hack.
    sort(_inherited_fields.begin(), _inherited_fields.end(), SortFieldsByIndex());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::shadow_inherited_field
//       Access: Private
//  Description: This is called only by rebuild_inherited_fields().
//               It removes the named field from the list of
//               _inherited_fields, presumably in preparation for
//               adding a new definition below.
////////////////////////////////////////////////////////////////////
void DCClass::
shadow_inherited_field(const string &name) {
  Fields::iterator fi;
  for (fi = _inherited_fields.begin(); fi != _inherited_fields.end(); ++fi) {
    DCField *field = (*fi);
    if (field->get_name() == name) {
      _inherited_fields.erase(fi);
      return;
    }
  }

  // If we get here, the named field wasn't in the list.  Huh.
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: DCClass::add_field
//       Access: Public
//  Description: Adds the newly-allocated field to the class.  The
//               class becomes the owner of the pointer and will
//               delete it when it destructs.  Returns true if the
//               field is successfully added, or false if there was a
//               name conflict or some other problem.
////////////////////////////////////////////////////////////////////
bool DCClass::
add_field(DCField *field) {
  nassertr(field->get_class() == this || field->get_class() == NULL, false);
  field->set_class(this);
  if (_dc_file != (DCFile *)NULL) {
    _dc_file->mark_inherited_fields_stale();
  }

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

  if (_dc_file != (DCFile *)NULL && 
      ((dc_virtual_inheritance && dc_sort_inheritance_by_file) || !is_struct())) {
    if (dc_multiple_inheritance) {
      _dc_file->set_new_index_number(field);
    } else {
      field->set_number(get_num_inherited_fields());
    }

    bool inserted = _fields_by_index.insert
      (FieldsByIndex::value_type(field->get_number(), field)).second;

    // It shouldn't be possible for that to fail.
    nassertr(inserted, false);
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
  _dc_file->mark_inherited_fields_stale();
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

