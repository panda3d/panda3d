/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcField_ext.cxx
 * @author CFSworks
 * @date 2019-07-03
 */

#include "dcField_ext.h"
#include "dcPacker_ext.h"
#include "dcmsgtypes.h"

#include "datagram.h"
#include "pStatTimer.h"

#ifdef HAVE_PYTHON

/**
 * Packs the Python arguments from the indicated tuple into the packer.
 * Returns true on success, false on failure.
 *
 * It is assumed that the packer is currently positioned on this field.
 */
bool Extension<DCField>::
pack_args(DCPacker &packer, PyObject *sequence) const {
  nassertr(!packer.had_error(), false);
  nassertr(packer.get_current_field() == _this, false);

  invoke_extension(&packer).pack_object(sequence);
  if (!packer.had_error()) {
    /*
    cerr << "pack " << _this->get_name() << get_pystr(sequence) << "\n";
    */

    return true;
  }

  if (!Notify::ptr()->has_assert_failed()) {
    std::ostringstream strm;
    PyObject *exc_type = PyExc_Exception;

    if (_this->as_parameter() != nullptr) {
      // If it's a parameter-type field, the value may or may not be a
      // sequence.
      if (packer.had_pack_error()) {
        strm << "Incorrect arguments to field: " << _this->get_name()
             << " = " << get_pystr(sequence);
        exc_type = PyExc_TypeError;
      } else {
        strm << "Value out of range on field: " << _this->get_name()
             << " = " << get_pystr(sequence);
        exc_type = PyExc_ValueError;
      }

    } else {
      // If it's a molecular or atomic field, the value should be a sequence.
      PyObject *tuple = PySequence_Tuple(sequence);
      if (tuple == nullptr) {
        strm << "Value for " << _this->get_name() << " not a sequence: " \
             << get_pystr(sequence);
        exc_type = PyExc_TypeError;

      } else {
        if (packer.had_pack_error()) {
          strm << "Incorrect arguments to field: " << _this->get_name()
               << get_pystr(sequence);
          exc_type = PyExc_TypeError;
        } else {
          strm << "Value out of range on field: " << _this->get_name()
               << get_pystr(sequence);
          exc_type = PyExc_ValueError;
        }

        Py_DECREF(tuple);
      }
    }

    std::string message = strm.str();
    PyErr_SetString(exc_type, message.c_str());
  }
  return false;
}

/**
 * Unpacks the values from the packer, beginning at the current point in the
 * unpack_buffer, into a Python tuple and returns the tuple.
 *
 * It is assumed that the packer is currently positioned on this field.
 */
PyObject *Extension<DCField>::
unpack_args(DCPacker &packer) const {
  nassertr(!packer.had_error(), nullptr);
  nassertr(packer.get_current_field() == _this, nullptr);

  size_t start_byte = packer.get_num_unpacked_bytes();
  PyObject *object = invoke_extension(&packer).unpack_object();

  if (!packer.had_error()) {
    // Successfully unpacked.
    /*
    cerr << "recv " << _this->get_name() << get_pystr(object) << "\n";
    */

    return object;
  }

  if (!Notify::ptr()->has_assert_failed()) {
    std::ostringstream strm;
    PyObject *exc_type = PyExc_Exception;

    if (packer.had_pack_error()) {
      strm << "Data error unpacking field ";
      _this->output(strm, true);
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
           << _this->get_name() << ": " << get_pystr(object);
      exc_type = PyExc_ValueError;
    }

    std::string message = strm.str();
    PyErr_SetString(exc_type, message.c_str());
  }

  Py_XDECREF(object);
  return nullptr;
}

/**
 * Extracts the update message out of the datagram and applies it to the
 * indicated object by calling the appropriate method.
 */
void Extension<DCField>::
receive_update(DCPacker &packer, PyObject *distobj) const {
  if (_this->as_parameter() != nullptr) {
    // If it's a parameter-type field, just store a new value on the object.
    PyObject *value = unpack_args(packer);
    if (value != nullptr) {
      PyObject_SetAttrString(distobj, (char *)_this->_name.c_str(), value);
    }
    Py_DECREF(value);

  } else {
    // Otherwise, it must be an atomic or molecular field, so call the
    // corresponding method.

    if (!PyObject_HasAttrString(distobj, (char *)_this->_name.c_str())) {
      // If there's no Python method to receive this message, don't bother
      // unpacking it to a Python tuple--just skip past the message.
      packer.unpack_skip();

    } else {
      // Otherwise, get a Python tuple from the args and call the Python
      // method.
      PyObject *args = unpack_args(packer);

      if (args != nullptr) {
        PyObject *func = PyObject_GetAttrString(distobj, (char *)_this->_name.c_str());
        nassertv(func != nullptr);

        PyObject *result;
        {
#ifdef WITHIN_PANDA
          PStatTimer timer(_this->_field_update_pcollector);
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

/**
 * Generates a datagram containing the message necessary to send an update for
 * the indicated distributed object from the client.
 */
Datagram Extension<DCField>::
client_format_update(DOID_TYPE do_id, PyObject *args) const {
  DCPacker packer;

  packer.raw_pack_uint16(CLIENT_OBJECT_SET_FIELD);
  packer.raw_pack_uint32(do_id);
  packer.raw_pack_uint16(_this->_number);

  packer.begin_pack(_this);
  pack_args(packer, args);
  if (!packer.end_pack()) {
    return Datagram();
  }

  return Datagram(packer.get_data(), packer.get_length());
}

/**
 * Generates a datagram containing the message necessary to send an update for
 * the indicated distributed object from the AI.
 */
Datagram Extension<DCField>::
ai_format_update(DOID_TYPE do_id, CHANNEL_TYPE to_id, CHANNEL_TYPE from_id, PyObject *args) const {
  DCPacker packer;

  packer.raw_pack_uint8(1);
  packer.RAW_PACK_CHANNEL(to_id);
  packer.RAW_PACK_CHANNEL(from_id);
  packer.raw_pack_uint16(STATESERVER_OBJECT_SET_FIELD);
  packer.raw_pack_uint32(do_id);
  packer.raw_pack_uint16(_this->_number);

  packer.begin_pack(_this);
  pack_args(packer, args);
  if (!packer.end_pack()) {
    return Datagram();
  }

  return Datagram(packer.get_data(), packer.get_length());
}

/**
 * Generates a datagram containing the message necessary to send an update,
 * with the msg type, for the indicated distributed object from the AI.
 */
Datagram Extension<DCField>::
ai_format_update_msg_type(DOID_TYPE do_id, CHANNEL_TYPE to_id, CHANNEL_TYPE from_id, int msg_type, PyObject *args) const {
  DCPacker packer;

  packer.raw_pack_uint8(1);
  packer.RAW_PACK_CHANNEL(to_id);
  packer.RAW_PACK_CHANNEL(from_id);
  packer.raw_pack_uint16(msg_type);
  packer.raw_pack_uint32(do_id);
  packer.raw_pack_uint16(_this->_number);

  packer.begin_pack(_this);
  pack_args(packer, args);
  if (!packer.end_pack()) {
    return Datagram();
  }

  return Datagram(packer.get_data(), packer.get_length());
}

/**
 * Returns the string representation of the indicated Python object.
 */
std::string Extension<DCField>::
get_pystr(PyObject *value) {
  if (value == nullptr) {
    return "(null)";
  }

  PyObject *str = PyObject_Str(value);
  if (str != nullptr) {
#if PY_MAJOR_VERSION >= 3
    std::string result = PyUnicode_AsUTF8(str);
#else
    std::string result = PyString_AsString(str);
#endif
    Py_DECREF(str);
    return result;
  }

  PyObject *repr = PyObject_Repr(value);
  if (repr != nullptr) {
#if PY_MAJOR_VERSION >= 3
    std::string result = PyUnicode_AsUTF8(repr);
#else
    std::string result = PyString_AsString(repr);
#endif
    Py_DECREF(repr);
    return result;
  }

  if (value->ob_type != nullptr) {
    PyObject *typestr = PyObject_Str((PyObject *)(value->ob_type));
    if (typestr != nullptr) {
#if PY_MAJOR_VERSION >= 3
      std::string result = PyUnicode_AsUTF8(typestr);
#else
      std::string result = PyString_AsString(typestr);
#endif
      Py_DECREF(typestr);
      return result;
    }
  }

  return "(invalid object)";
}

#endif  // HAVE_PYTHON
