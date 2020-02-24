/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcClass_ext.cxx
 * @author CFSworks
 * @date 2019-07-03
 */

#include "dcClass_ext.h"
#include "dcField_ext.h"
#include "dcAtomicField.h"
#include "dcPacker.h"
#include "dcmsgtypes.h"

#include "datagram.h"
#include "datagramIterator.h"
#include "pStatTimer.h"

#ifdef HAVE_PYTHON

/**
 * Returns true if the DCClass object has an associated Python class
 * definition, false otherwise.
 */
bool Extension<DCClass>::
has_class_def() const {
  return _this->_python_class_defs != nullptr
      && ((PythonClassDefsImpl *)_this->_python_class_defs.p())->_class_def != nullptr;
}

/**
 * Sets the class object associated with this DistributedClass.  This object
 * will be used to construct new instances of the class.
 */
void Extension<DCClass>::
set_class_def(PyObject *class_def) {
  PythonClassDefsImpl *defs = do_get_defs();

  Py_XINCREF(class_def);
  Py_XDECREF(defs->_class_def);
  defs->_class_def = class_def;
}

/**
 * Returns the class object that was previously associated with this
 * DistributedClass.  This will return a new reference to the object.
 */
PyObject *Extension<DCClass>::
get_class_def() const {
  if (!has_class_def()) {
    Py_INCREF(Py_None);
    return Py_None;
  }

  PythonClassDefsImpl *defs = do_get_defs();
  Py_INCREF(defs->_class_def);
  return defs->_class_def;
}

/**
 * Returns true if the DCClass object has an associated Python owner class
 * definition, false otherwise.
 */
bool Extension<DCClass>::
has_owner_class_def() const {
  return _this->_python_class_defs != nullptr
      && ((PythonClassDefsImpl *)_this->_python_class_defs.p())->_owner_class_def != nullptr;
}

/**
 * Sets the owner class object associated with this DistributedClass.  This
 * object will be used to construct new owner instances of the class.
 */
void Extension<DCClass>::
set_owner_class_def(PyObject *owner_class_def) {
  PythonClassDefsImpl *defs = do_get_defs();

  Py_XINCREF(owner_class_def);
  Py_XDECREF(defs->_owner_class_def);
  defs->_owner_class_def = owner_class_def;
}

/**
 * Returns the owner class object that was previously associated with this
 * DistributedClass.  This will return a new reference to the object.
 */
PyObject *Extension<DCClass>::
get_owner_class_def() const {
  if (!has_owner_class_def()) {
    Py_INCREF(Py_None);
    return Py_None;
  }

  PythonClassDefsImpl *defs = do_get_defs();
  Py_INCREF(defs->_owner_class_def);
  return defs->_owner_class_def;
}

/**
 * Extracts the update message out of the packer and applies it to the
 * indicated object by calling the appropriate method.
 */
void Extension<DCClass>::
receive_update(PyObject *distobj, DatagramIterator &di) const {
  PStatTimer timer(_this->_class_update_pcollector);
  DCPacker packer;
  const char *data = (const char *)di.get_datagram().get_data();
  packer.set_unpack_data(data + di.get_current_index(),
                         di.get_remaining_size(), false);

  int field_id = packer.raw_unpack_uint16();
  DCField *field = _this->get_field_by_index(field_id);
  if (field == nullptr) {
    ostringstream strm;
    strm
        << "Received update for field " << field_id << ", not in class "
        << _this->get_name();
    nassert_raise(strm.str());
    return;
  }

  packer.begin_unpack(field);
  invoke_extension(field).receive_update(packer, distobj);
  packer.end_unpack();

  di.skip_bytes(packer.get_num_unpacked_bytes());
}

/**
 * Processes a big datagram that includes all of the "required" fields that
 * are sent along with a normal "generate with required" message.  This is all
 * of the atomic fields that are marked "broadcast required".
 */
void Extension<DCClass>::
receive_update_broadcast_required(PyObject *distobj, DatagramIterator &di) const {
  PStatTimer timer(_this->_class_update_pcollector);
  DCPacker packer;
  const char *data = (const char *)di.get_datagram().get_data();
  packer.set_unpack_data(data + di.get_current_index(),
                         di.get_remaining_size(), false);

  int num_fields = _this->get_num_inherited_fields();
  for (int i = 0; i < num_fields && !PyErr_Occurred(); ++i) {
    DCField *field = _this->get_inherited_field(i);
    if (field->as_molecular_field() == nullptr &&
        field->is_required() && field->is_broadcast()) {
      packer.begin_unpack(field);
      invoke_extension(field).receive_update(packer, distobj);
      if (!packer.end_unpack()) {
        break;
      }
    }
  }

  di.skip_bytes(packer.get_num_unpacked_bytes());
}

/**
 * Processes a big datagram that includes all of the "required" fields that
 * are sent along with a normal "generate with required" message.  This is all
 * of the atomic fields that are marked "broadcast ownrecv". Should be used
 * for 'owner-view' objects.
 */
void Extension<DCClass>::
receive_update_broadcast_required_owner(PyObject *distobj,
                                        DatagramIterator &di) const {
  PStatTimer timer(_this->_class_update_pcollector);
  DCPacker packer;
  const char *data = (const char *)di.get_datagram().get_data();
  packer.set_unpack_data(data + di.get_current_index(),
                         di.get_remaining_size(), false);

  int num_fields = _this->get_num_inherited_fields();
  for (int i = 0; i < num_fields && !PyErr_Occurred(); ++i) {
    DCField *field = _this->get_inherited_field(i);
    if (field->as_molecular_field() == nullptr &&
        field->is_required() && (field->is_ownrecv() || field->is_broadcast())) {
      packer.begin_unpack(field);
      invoke_extension(field).receive_update(packer, distobj);
      if (!packer.end_unpack()) {
        break;
      }
    }
  }

  di.skip_bytes(packer.get_num_unpacked_bytes());
}

/**
 * Processes a big datagram that includes all of the "required" fields that
 * are sent when an avatar is created.  This is all of the atomic fields that
 * are marked "required", whether they are broadcast or not.
 */
void Extension<DCClass>::
receive_update_all_required(PyObject *distobj, DatagramIterator &di) const {
  PStatTimer timer(_this->_class_update_pcollector);
  DCPacker packer;
  const char *data = (const char *)di.get_datagram().get_data();
  packer.set_unpack_data(data + di.get_current_index(),
                         di.get_remaining_size(), false);

  int num_fields = _this->get_num_inherited_fields();
  for (int i = 0; i < num_fields && !PyErr_Occurred(); ++i) {
    DCField *field = _this->get_inherited_field(i);
    if (field->as_molecular_field() == nullptr &&
        field->is_required()) {
      packer.begin_unpack(field);
      invoke_extension(field).receive_update(packer, distobj);
      if (!packer.end_unpack()) {
        break;
      }
    }
  }

  di.skip_bytes(packer.get_num_unpacked_bytes());
}

/**
 * Processes a datagram that lists some additional fields that are broadcast
 * in one chunk.
 */
void Extension<DCClass>::
receive_update_other(PyObject *distobj, DatagramIterator &di) const {
  PStatTimer timer(_this->_class_update_pcollector);
  int num_fields = di.get_uint16();
  for (int i = 0; i < num_fields && !PyErr_Occurred(); ++i) {
    receive_update(distobj, di);
  }
}

/**
 * Processes an update for a named field from a packed value blob.
 */
void Extension<DCClass>::
direct_update(PyObject *distobj, const std::string &field_name,
              const vector_uchar &value_blob) {
  DCField *field = _this->get_field_by_name(field_name);
  nassertv_always(field != nullptr);

  DCPacker packer;
  packer.set_unpack_data(value_blob);
  packer.begin_unpack(field);
  invoke_extension(field).receive_update(packer, distobj);
  packer.end_unpack();
}

/**
 * Processes an update for a named field from a packed datagram.
 */
void Extension<DCClass>::
direct_update(PyObject *distobj, const std::string &field_name,
              const Datagram &datagram) {
  DCField *field = _this->get_field_by_name(field_name);
  nassertv_always(field != nullptr);

  DCPacker packer;
  packer.set_unpack_data((const char *)datagram.get_data(), datagram.get_length(), false);
  packer.begin_unpack(field);
  invoke_extension(field).receive_update(packer, distobj);
  packer.end_unpack();
}

/**
 * Looks up the current value of the indicated field by calling the
 * appropriate get*() function, then packs that value into the datagram.  This
 * field is presumably either a required field or a specified optional field,
 * and we are building up a datagram for the generate-with-required message.
 *
 * Returns true on success, false on failure.
 */
bool Extension<DCClass>::
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

/**
 * Looks up the current value of the indicated field by calling the
 * appropriate get*() function, then packs that value into the packer.  This
 * field is presumably either a required field or a specified optional field,
 * and we are building up a datagram for the generate-with-required message.
 *
 * Returns true on success, false on failure.
 */
bool Extension<DCClass>::
pack_required_field(DCPacker &packer, PyObject *distobj,
                    const DCField *field) const {
  using std::ostringstream;

  const DCParameter *parameter = field->as_parameter();
  if (parameter != nullptr) {
    // This is the easy case: to pack a parameter, we just look on the class
    // object for the data element.
    std::string field_name = field->get_name();

    if (!PyObject_HasAttrString(distobj, (char *)field_name.c_str())) {
      // If the attribute is not defined, but the field has a default value
      // specified, quietly pack the default value.
      if (field->has_default_value()) {
        packer.pack_default_value();
        return true;
      }

      // If there is no default value specified, it's an error.
      ostringstream strm;
      strm << "Data element " << field_name
           << ", required by dc file for dclass " << _this->get_name()
           << ", not defined on object";
      nassert_raise(strm.str());
      return false;
    }
    PyObject *result =
      PyObject_GetAttrString(distobj, (char *)field_name.c_str());
    nassertr(result != nullptr, false);

    // Now pack the value into the datagram.
    bool pack_ok = invoke_extension((DCField *)parameter).pack_args(packer, result);
    Py_DECREF(result);

    return pack_ok;
  }

  if (field->as_molecular_field() != nullptr) {
    ostringstream strm;
    strm << "Cannot pack molecular field " << field->get_name()
         << " for generate";
    nassert_raise(strm.str());
    return false;
  }

  const DCAtomicField *atom = field->as_atomic_field();
  nassertr(atom != nullptr, false);

  // We need to get the initial value of this field.  There isn't a good,
  // robust way to get this; presently, we just mangle the "setFoo()" name of
  // the required field into "getFoo()" and call that.
  std::string setter_name = atom->get_name();

  if (setter_name.empty()) {
    ostringstream strm;
    strm << "Required field is unnamed!";
    nassert_raise(strm.str());
    return false;
  }

  if (atom->get_num_elements() == 0) {
    // It sure doesn't make sense to have a required field with no parameters.
    // What data, exactly, is required?
    ostringstream strm;
    strm << "Required field " << setter_name << " has no parameters!";
    nassert_raise(strm.str());
    return false;
  }

  std::string getter_name = setter_name;
  if (setter_name.substr(0, 3) == "set") {
    // If the original method started with "set", we mangle this directly to
    // "get".
    getter_name[0] = 'g';

  } else {
    // Otherwise, we add a "get" prefix, and capitalize the next letter.
    getter_name = "get" + setter_name;
    getter_name[3] = toupper(getter_name[3]);
  }

  // Now we have to look up the getter on the distributed object and call it.
  if (!PyObject_HasAttrString(distobj, (char *)getter_name.c_str())) {
    // As above, if there's no getter but the field has a default value
    // specified, quietly pack the default value.
    if (field->has_default_value()) {
      packer.pack_default_value();
      return true;
    }

    // Otherwise, with no default value it's an error.
    ostringstream strm;
    strm << "Distributed class " << _this->get_name()
         << " doesn't have getter named " << getter_name
         << " to match required field " << setter_name;
    nassert_raise(strm.str());
    return false;
  }
  PyObject *func =
    PyObject_GetAttrString(distobj, (char *)getter_name.c_str());
  nassertr(func != nullptr, false);

  PyObject *empty_args = PyTuple_New(0);
  PyObject *result = PyObject_CallObject(func, empty_args);
  Py_DECREF(empty_args);
  Py_DECREF(func);
  if (result == nullptr) {
    // We don't set this as an exception, since presumably the Python method
    // itself has already triggered a Python exception.
    std::cerr << "Error when calling " << getter_name << "\n";
    return false;
  }

  if (atom->get_num_elements() == 1) {
    // In this case, we expect the getter to return one object, which we wrap
    // up in a tuple.
    PyObject *tuple = PyTuple_New(1);
    PyTuple_SET_ITEM(tuple, 0, result);
    result = tuple;

  } else {
    // Otherwise, it had better already be a sequence or tuple of some sort.
    if (!PySequence_Check(result)) {
      ostringstream strm;
      strm << "Since dclass " << _this->get_name() << " method " << setter_name
           << " is declared to have multiple parameters, Python function "
           << getter_name << " must return a list or tuple.\n";
      nassert_raise(strm.str());
      return false;
    }
  }

  // Now pack the arguments into the datagram.
  bool pack_ok = invoke_extension((DCField *)atom).pack_args(packer, result);
  Py_DECREF(result);

  return pack_ok;
}

/**
 * Generates a datagram containing the message necessary to send an update for
 * the indicated distributed object from the client.
 */
Datagram Extension<DCClass>::
client_format_update(const std::string &field_name, DOID_TYPE do_id,
                     PyObject *args) const {
  DCField *field = _this->get_field_by_name(field_name);
  if (field == nullptr) {
    std::ostringstream strm;
    strm << "No field named " << field_name << " in class " << _this->get_name()
         << "\n";
    nassert_raise(strm.str());
    return Datagram();
  }

  return invoke_extension(field).client_format_update(do_id, args);
}

/**
 * Generates a datagram containing the message necessary to send an update for
 * the indicated distributed object from the AI.
 */
Datagram Extension<DCClass>::
ai_format_update(const std::string &field_name, DOID_TYPE do_id,
                 CHANNEL_TYPE to_id, CHANNEL_TYPE from_id, PyObject *args) const {
  DCField *field = _this->get_field_by_name(field_name);
  if (field == nullptr) {
    std::ostringstream strm;
    strm << "No field named " << field_name << " in class " << _this->get_name()
         << "\n";
    nassert_raise(strm.str());
    return Datagram();
  }

  return invoke_extension(field).ai_format_update(do_id, to_id, from_id, args);
}

/**
 * Generates a datagram containing the message necessary to send an update,
 * using the indicated msg type for the indicated distributed object from the
 * AI.
 */
Datagram Extension<DCClass>::
ai_format_update_msg_type(const std::string &field_name, DOID_TYPE do_id,
                          CHANNEL_TYPE to_id, CHANNEL_TYPE from_id,
                          int msg_type, PyObject *args) const {
  DCField *field = _this->get_field_by_name(field_name);
  if (field == nullptr) {
    std::ostringstream strm;
    strm << "No field named " << field_name << " in class " << _this->get_name()
         << "\n";
    nassert_raise(strm.str());
    return Datagram();
  }

  return invoke_extension(field).ai_format_update_msg_type(do_id, to_id, from_id, msg_type, args);
}

/**
 * Generates a datagram containing the message necessary to generate a new
 * distributed object from the client.  This requires querying the object for
 * the initial value of its required fields.
 *
 * optional_fields is a list of fieldNames to generate in addition to the
 * normal required fields.
 *
 * This method is only called by the CMU implementation.
 */
Datagram Extension<DCClass>::
client_format_generate_CMU(PyObject *distobj, DOID_TYPE do_id,
                           ZONEID_TYPE zone_id,
                           PyObject *optional_fields) const {
  DCPacker packer;

  packer.raw_pack_uint16(CLIENT_OBJECT_GENERATE_CMU);

  packer.raw_pack_uint32(zone_id);
  packer.raw_pack_uint16(_this->_number);
  packer.raw_pack_uint32(do_id);

  // Specify all of the required fields.
  int num_fields = _this->get_num_inherited_fields();
  for (int i = 0; i < num_fields; ++i) {
    DCField *field = _this->get_inherited_field(i);
    if (field->is_required() && field->as_molecular_field() == nullptr) {
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
    std::string field_name = PyUnicode_AsUTF8(py_field_name);
#else
    std::string field_name = PyString_AsString(py_field_name);
#endif
    Py_XDECREF(py_field_name);

    DCField *field = _this->get_field_by_name(field_name);
    if (field == nullptr) {
      std::ostringstream strm;
      strm << "No field named " << field_name << " in class " << _this->get_name()
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

/**
 * Generates a datagram containing the message necessary to generate a new
 * distributed object from the AI. This requires querying the object for the
 * initial value of its required fields.
 *
 * optional_fields is a list of fieldNames to generate in addition to the
 * normal required fields.
 */
Datagram Extension<DCClass>::
ai_format_generate(PyObject *distobj, DOID_TYPE do_id,
                   DOID_TYPE parent_id, ZONEID_TYPE zone_id,
                   CHANNEL_TYPE district_channel_id, CHANNEL_TYPE from_channel_id,
                   PyObject *optional_fields) const {
  DCPacker packer;

  packer.raw_pack_uint8(1);
  packer.RAW_PACK_CHANNEL(district_channel_id);
  packer.RAW_PACK_CHANNEL(from_channel_id);
    // packer.raw_pack_uint8('A');

  bool has_optional_fields = (PyObject_IsTrue(optional_fields) != 0);

  if (has_optional_fields) {
    packer.raw_pack_uint16(STATESERVER_CREATE_OBJECT_WITH_REQUIRED_OTHER);
  } else {
    packer.raw_pack_uint16(STATESERVER_CREATE_OBJECT_WITH_REQUIRED);
  }

  packer.raw_pack_uint32(do_id);
  // Parent is a bit overloaded; this parent is not about inheritance, this
  // one is about the visibility container parent, i.e.  the zone parent:
  packer.raw_pack_uint32(parent_id);
  packer.raw_pack_uint32(zone_id);
  packer.raw_pack_uint16(_this->_number);

  // Specify all of the required fields.
  int num_fields = _this->get_num_inherited_fields();
  for (int i = 0; i < num_fields; ++i) {
    DCField *field = _this->get_inherited_field(i);
    if (field->is_required() && field->as_molecular_field() == nullptr) {
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
      std::string field_name = PyUnicode_AsUTF8(py_field_name);
#else
      std::string field_name = PyString_AsString(py_field_name);
#endif
      Py_XDECREF(py_field_name);

      DCField *field = _this->get_field_by_name(field_name);
      if (field == nullptr) {
        std::ostringstream strm;
        strm << "No field named " << field_name << " in class "
             << _this->get_name() << "\n";
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

/**
 * Returns the PythonClassDefsImpl object stored on the DCClass object,
 * creating it if it didn't yet exist.
 */
Extension<DCClass>::PythonClassDefsImpl *Extension<DCClass>::
do_get_defs() const {
  if (!_this->_python_class_defs) {
    _this->_python_class_defs = new PythonClassDefsImpl();
  }
  return (PythonClassDefsImpl *)_this->_python_class_defs.p();
}

#endif  // HAVE_PYTHON
