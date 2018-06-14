/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcField.cxx
 * @author drose
 * @date 2000-10-11
 */

#include "dcField.h"
#include "dcFile.h"
#include "dcPacker.h"
#include "dcClass.h"
#include "hashGenerator.h"
#include "dcmsgtypes.h"

#ifdef HAVE_PYTHON
#include "py_panda.h"
#endif

#ifdef WITHIN_PANDA
#include "pStatTimer.h"
#endif

using std::string;

/**
 *
 */
DCField::
DCField() :
  _dclass(nullptr)
#ifdef WITHIN_PANDA
  ,
  _field_update_pcollector("DCField")
#endif
{
  _number = -1;
  _default_value_stale = true;
  _has_default_value = false;

  _bogus_field = false;

  _has_nested_fields = true;
  _num_nested_fields = 0;
  _pack_type = PT_field;

  _has_fixed_byte_size = true;
  _fixed_byte_size = 0;
  _has_fixed_structure = true;
}

/**
 *
 */
DCField::
DCField(const string &name, DCClass *dclass) :
  DCPackerInterface(name),
  _dclass(dclass)
#ifdef WITHIN_PANDA
  ,
  _field_update_pcollector(dclass->_class_update_pcollector, name)
#endif
{
  _number = -1;
  _has_default_value = false;
  _default_value_stale = true;

  _bogus_field = false;

  _has_nested_fields = true;
  _num_nested_fields = 0;
  _pack_type = PT_field;

  _has_fixed_byte_size = true;
  _fixed_byte_size = 0;
  _has_fixed_structure = true;
}

/**
 *
 */
DCField::
~DCField() {
}

/**
 *
 */
DCField *DCField::
as_field() {
  return this;
}

/**
 *
 */
const DCField *DCField::
as_field() const {
  return this;
}

/**
 * Returns the same field pointer converted to an atomic field pointer, if
 * this is in fact an atomic field; otherwise, returns NULL.
 */
DCAtomicField *DCField::
as_atomic_field() {
  return nullptr;
}

/**
 * Returns the same field pointer converted to an atomic field pointer, if
 * this is in fact an atomic field; otherwise, returns NULL.
 */
const DCAtomicField *DCField::
as_atomic_field() const {
  return nullptr;
}

/**
 * Returns the same field pointer converted to a molecular field pointer, if
 * this is in fact a molecular field; otherwise, returns NULL.
 */
DCMolecularField *DCField::
as_molecular_field() {
  return nullptr;
}

/**
 * Returns the same field pointer converted to a molecular field pointer, if
 * this is in fact a molecular field; otherwise, returns NULL.
 */
const DCMolecularField *DCField::
as_molecular_field() const {
  return nullptr;
}

/**
 *
 */
DCParameter *DCField::
as_parameter() {
  return nullptr;
}

/**
 *
 */
const DCParameter *DCField::
as_parameter() const {
  return nullptr;
}

/**
 * Given a blob that represents the packed data for this field, returns a
 * string formatting it for human consumption.  Returns empty string if there
 * is an error.
 */
string DCField::
format_data(const string &packed_data, bool show_field_names) {
  DCPacker packer;
  packer.set_unpack_data(packed_data);
  packer.begin_unpack(this);
  string result = packer.unpack_and_format(show_field_names);
  if (!packer.end_unpack()) {
    return string();
  }
  return result;
}

/**
 * Given a human-formatted string (for instance, as returned by format_data(),
 * above) that represents the value of this field, parse the string and return
 * the corresponding packed data.  Returns empty string if there is an error.
 */
string DCField::
parse_string(const string &formatted_string) {
  DCPacker packer;
  packer.begin_pack(this);
  if (!packer.parse_and_pack(formatted_string)) {
    // Parse error.
    return string();
  }
  if (!packer.end_pack()) {
    // Data type mismatch.
    return string();
  }

  return packer.get_string();
}

/**
 * Verifies that all of the packed values in the field data are within the
 * specified ranges and that there are no extra bytes on the end of the
 * record.  Returns true if all fields are valid, false otherwise.
 */
bool DCField::
validate_ranges(const string &packed_data) const {
  DCPacker packer;
  packer.set_unpack_data(packed_data);
  packer.begin_unpack(this);
  packer.unpack_validate();
  if (!packer.end_unpack()) {
    return false;
  }

  return (packer.get_num_unpacked_bytes() == packed_data.length());
}

#ifdef HAVE_PYTHON
/**
 * Packs the Python arguments from the indicated tuple into the packer.
 * Returns true on success, false on failure.
 *
 * It is assumed that the packer is currently positioned on this field.
 */
bool DCField::
pack_args(DCPacker &packer, PyObject *sequence) const {
  nassertr(!packer.had_error(), false);
  nassertr(packer.get_current_field() == this, false);

  packer.pack_object(sequence);
  if (!packer.had_error()) {
    /*
    cerr << "pack " << get_name() << get_pystr(sequence) << "\n";
    */

    return true;
  }

  if (!Notify::ptr()->has_assert_failed()) {
    std::ostringstream strm;
    PyObject *exc_type = PyExc_Exception;

    if (as_parameter() != nullptr) {
      // If it's a parameter-type field, the value may or may not be a
      // sequence.
      if (packer.had_pack_error()) {
        strm << "Incorrect arguments to field: " << get_name()
             << " = " << get_pystr(sequence);
        exc_type = PyExc_TypeError;
      } else {
        strm << "Value out of range on field: " << get_name()
             << " = " << get_pystr(sequence);
        exc_type = PyExc_ValueError;
      }

    } else {
      // If it's a molecular or atomic field, the value should be a sequence.
      PyObject *tuple = PySequence_Tuple(sequence);
      if (tuple == nullptr) {
        strm << "Value for " << get_name() << " not a sequence: " \
             << get_pystr(sequence);
        exc_type = PyExc_TypeError;

      } else {
        if (packer.had_pack_error()) {
          strm << "Incorrect arguments to field: " << get_name()
               << get_pystr(sequence);
          exc_type = PyExc_TypeError;
        } else {
          strm << "Value out of range on field: " << get_name()
               << get_pystr(sequence);
          exc_type = PyExc_ValueError;
        }

        Py_DECREF(tuple);
      }
    }

    string message = strm.str();
    PyErr_SetString(exc_type, message.c_str());
  }
  return false;
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
/**
 * Unpacks the values from the packer, beginning at the current point in the
 * unpack_buffer, into a Python tuple and returns the tuple.
 *
 * It is assumed that the packer is currently positioned on this field.
 */
PyObject *DCField::
unpack_args(DCPacker &packer) const {
  nassertr(!packer.had_error(), nullptr);
  nassertr(packer.get_current_field() == this, nullptr);

  size_t start_byte = packer.get_num_unpacked_bytes();
  PyObject *object = packer.unpack_object();

  if (!packer.had_error()) {
    // Successfully unpacked.
    /*
    cerr << "recv " << get_name() << get_pystr(object) << "\n";
    */

    return object;
  }

  if (!Notify::ptr()->has_assert_failed()) {
    std::ostringstream strm;
    PyObject *exc_type = PyExc_Exception;

    if (packer.had_pack_error()) {
      strm << "Data error unpacking field ";
      output(strm, true);
      size_t length = packer.get_unpack_length() - start_byte;
      strm << "\nGot data (" << (int)length << " bytes):\n";
      Datagram dg(packer.get_unpack_data() + start_byte, length);
      dg.dump_hex(strm);
      size_t error_byte = packer.get_num_unpacked_bytes() - start_byte;
      strm << "Error detected on byte " << error_byte
           << " (" << std::hex << error_byte << std::dec << " hex)";

      exc_type = PyExc_RuntimeError;
    } else {
      strm << "Value outside specified range when unpacking field "
           << get_name() << ": " << get_pystr(object);
      exc_type = PyExc_ValueError;
    }

    string message = strm.str();
    PyErr_SetString(exc_type, message.c_str());
  }

  Py_XDECREF(object);
  return nullptr;
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
/**
 * Extracts the update message out of the datagram and applies it to the
 * indicated object by calling the appropriate method.
 */
void DCField::
receive_update(DCPacker &packer, PyObject *distobj) const {
  if (as_parameter() != nullptr) {
    // If it's a parameter-type field, just store a new value on the object.
    PyObject *value = unpack_args(packer);
    if (value != nullptr) {
      PyObject_SetAttrString(distobj, (char *)_name.c_str(), value);
    }
    Py_DECREF(value);

  } else {
    // Otherwise, it must be an atomic or molecular field, so call the
    // corresponding method.

    if (!PyObject_HasAttrString(distobj, (char *)_name.c_str())) {
      // If there's no Python method to receive this message, don't bother
      // unpacking it to a Python tuple--just skip past the message.
      packer.unpack_skip();

    } else {
      // Otherwise, get a Python tuple from the args and call the Python
      // method.
      PyObject *args = unpack_args(packer);

      if (args != nullptr) {
        PyObject *func = PyObject_GetAttrString(distobj, (char *)_name.c_str());
        nassertv(func != nullptr);

        PyObject *result;
        {
#ifdef WITHIN_PANDA
          PStatTimer timer(((DCField *)this)->_field_update_pcollector);
#endif
          result = PyObject_CallObject(func, args);
        }
        Py_XDECREF(result);
        Py_DECREF(func);
        Py_DECREF(args);
      }
    }
  }
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
/**
 * Generates a datagram containing the message necessary to send an update for
 * the indicated distributed object from the client.
 */
Datagram DCField::
client_format_update(DOID_TYPE do_id, PyObject *args) const {
  DCPacker packer;

  packer.raw_pack_uint16(CLIENT_OBJECT_SET_FIELD);
  packer.raw_pack_uint32(do_id);
  packer.raw_pack_uint16(_number);

  packer.begin_pack(this);
  pack_args(packer, args);
  if (!packer.end_pack()) {
    return Datagram();
  }

  return Datagram(packer.get_data(), packer.get_length());
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
/**
 * Generates a datagram containing the message necessary to send an update for
 * the indicated distributed object from the AI.
 */
Datagram DCField::
ai_format_update(DOID_TYPE do_id, CHANNEL_TYPE to_id, CHANNEL_TYPE from_id, PyObject *args) const {
  DCPacker packer;

  packer.raw_pack_uint8(1);
  packer.RAW_PACK_CHANNEL(to_id);
  packer.RAW_PACK_CHANNEL(from_id);
  packer.raw_pack_uint16(STATESERVER_OBJECT_SET_FIELD);
  packer.raw_pack_uint32(do_id);
  packer.raw_pack_uint16(_number);

  packer.begin_pack(this);
  pack_args(packer, args);
  if (!packer.end_pack()) {
    return Datagram();
  }

  return Datagram(packer.get_data(), packer.get_length());
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
/**
 * Generates a datagram containing the message necessary to send an update,
 * with the msg type, for the indicated distributed object from the AI.
 */
Datagram DCField::
ai_format_update_msg_type(DOID_TYPE do_id, CHANNEL_TYPE to_id, CHANNEL_TYPE from_id, int msg_type, PyObject *args) const {
  DCPacker packer;

  packer.raw_pack_uint8(1);
  packer.RAW_PACK_CHANNEL(to_id);
  packer.RAW_PACK_CHANNEL(from_id);
  packer.raw_pack_uint16(msg_type);
  packer.raw_pack_uint32(do_id);
  packer.raw_pack_uint16(_number);

  packer.begin_pack(this);
  pack_args(packer, args);
  if (!packer.end_pack()) {
    return Datagram();
  }

  return Datagram(packer.get_data(), packer.get_length());
}
#endif  // HAVE_PYTHON


/**
 * Accumulates the properties of this field into the hash.
 */
void DCField::
generate_hash(HashGenerator &hashgen) const {
  // It shouldn't be necessary to explicitly add _number to the hash--this is
  // computed based on the relative position of this field with the other
  // fields, so adding it explicitly will be redundant.  However, the field
  // name is significant.
  hashgen.add_string(_name);

  // Actually, we add _number anyway, since we need to ensure the hash code
  // comes out different in the dc_multiple_inheritance case.
  if (dc_multiple_inheritance) {
    hashgen.add_int(_number);
  }
}

/**
 * Packs the field's specified default value (or a sensible default if no
 * value is specified) into the stream.  Returns true if the default value is
 * packed, false if the field doesn't know how to pack its default value.
 */
bool DCField::
pack_default_value(DCPackData &pack_data, bool &) const {
  // The default behavior is to pack the default value if we got it;
  // otherwise, to return false and let the packer visit our nested elements.
  if (!_default_value_stale) {
    pack_data.append_data(_default_value.data(), _default_value.length());
    return true;
  }

  return false;
}

/**
 * Sets the name of this field.
 */
void DCField::
set_name(const string &name) {
  DCPackerInterface::set_name(name);
  if (_dclass != nullptr) {
    _dclass->_dc_file->mark_inherited_fields_stale();
  }
}

#ifdef HAVE_PYTHON
/**
 * Returns the string representation of the indicated Python object.
 */
string DCField::
get_pystr(PyObject *value) {
  if (value == nullptr) {
    return "(null)";
  }

  PyObject *str = PyObject_Str(value);
  if (str != nullptr) {
#if PY_MAJOR_VERSION >= 3
    string result = PyUnicode_AsUTF8(str);
#else
    string result = PyString_AsString(str);
#endif
    Py_DECREF(str);
    return result;
  }

  PyObject *repr = PyObject_Repr(value);
  if (repr != nullptr) {
#if PY_MAJOR_VERSION >= 3
    string result = PyUnicode_AsUTF8(repr);
#else
    string result = PyString_AsString(repr);
#endif
    Py_DECREF(repr);
    return result;
  }

  if (value->ob_type != nullptr) {
    PyObject *typestr = PyObject_Str((PyObject *)(value->ob_type));
    if (typestr != nullptr) {
#if PY_MAJOR_VERSION >= 3
      string result = PyUnicode_AsUTF8(typestr);
#else
      string result = PyString_AsString(typestr);
#endif
      Py_DECREF(typestr);
      return result;
    }
  }

  return "(invalid object)";
}
#endif  // HAVE_PYTHON

/**
 * Recomputes the default value of the field by repacking it.
 */
void DCField::
refresh_default_value() {
  DCPacker packer;
  packer.begin_pack(this);
  packer.pack_default_value();
  if (!packer.end_pack()) {
    std::cerr << "Error while packing default value for " << get_name() << "\n";
  } else {
    _default_value.assign(packer.get_data(), packer.get_length());
  }
  _default_value_stale = false;
}
