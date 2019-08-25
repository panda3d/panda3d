/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcPacker_ext.cxx
 * @author CFSworks
 * @date 2019-07-03
 */

#include "dcPacker_ext.h"
#include "dcClass_ext.h"
#include "dcField_ext.h"

#include "dcClassParameter.h"

#ifdef HAVE_PYTHON

/**
 * Packs the Python object of whatever type into the packer.  Each numeric
 * object and string object maps to the corresponding pack_value() call; a
 * tuple or sequence maps to a push() followed by all of the tuple's contents
 * followed by a pop().
 */
void Extension<DCPacker>::
pack_object(PyObject *object) {
  nassertv(_this->_mode == DCPacker::Mode::M_pack ||
           _this->_mode == DCPacker::Mode::M_repack);
  DCPackType pack_type = _this->get_pack_type();

  // had to add this for basic 64 and unsigned data to get packed right .. Not
  // sure if we can just do the rest this way..

  switch(pack_type) {
  case PT_int64:
    if (PyLong_Check(object)) {
      _this->pack_int64(PyLong_AsLongLong(object));
      return;
    }
#if PY_MAJOR_VERSION < 3
    else if (PyInt_Check(object)) {
      _this->pack_int64(PyInt_AsLong(object));
      return;
    }
#endif
    break;

  case PT_uint64:
    if (PyLong_Check(object)) {
      _this->pack_uint64(PyLong_AsUnsignedLongLong(object));
      return;
    }
#if PY_MAJOR_VERSION < 3
    else if (PyInt_Check(object)) {
      PyObject *obj1 = PyNumber_Long(object);
      _this->pack_int(PyLong_AsUnsignedLongLong(obj1));
      Py_DECREF(obj1);
      return;
    }
#endif
    break;

  case PT_int:
    if (PyLong_Check(object)) {
      _this->pack_int(PyLong_AsLong(object));
      return;
    }
#if PY_MAJOR_VERSION < 3
    else if (PyInt_Check(object)) {
      _this->pack_int(PyInt_AsLong(object));
      return;
    }
#endif
    break;

  case PT_uint:
    if (PyLong_Check(object)) {
      _this->pack_uint(PyLong_AsUnsignedLong(object));
      return;
    }
#if PY_MAJOR_VERSION < 3
    else if (PyInt_Check(object)) {
      PyObject *obj1 = PyNumber_Long(object);
      _this->pack_uint(PyLong_AsUnsignedLong(obj1));
      Py_DECREF(obj1);
      return;
    }
#endif
    break;

  default:
    break;
  }

  if (PyLong_Check(object)) {
    _this->pack_int(PyLong_AsLong(object));
#if PY_MAJOR_VERSION < 3
  } else if (PyInt_Check(object)) {
    _this->pack_int(PyInt_AS_LONG(object));
#endif
  } else if (PyFloat_Check(object)) {
    _this->pack_double(PyFloat_AS_DOUBLE(object));
  } else if (PyLong_Check(object)) {
    _this->pack_int64(PyLong_AsLongLong(object));
#if PY_MAJOR_VERSION >= 3
  } else if (PyUnicode_Check(object)) {
    const char *buffer;
    Py_ssize_t length;
    buffer = PyUnicode_AsUTF8AndSize(object, &length);
    if (buffer) {
      _this->pack_string(std::string(buffer, length));
    }
  } else if (PyBytes_Check(object)) {
    const unsigned char *buffer;
    Py_ssize_t length;
    PyBytes_AsStringAndSize(object, (char **)&buffer, &length);
    if (buffer) {
      _this->pack_blob(vector_uchar(buffer, buffer + length));
    }
#else
  } else if (PyString_Check(object) || PyUnicode_Check(object)) {
    char *buffer;
    Py_ssize_t length;
    PyString_AsStringAndSize(object, &buffer, &length);
    if (buffer) {
      _this->pack_string(std::string(buffer, length));
    }
#endif
  } else {
    // For some reason, PySequence_Check() is incorrectly reporting that a
    // class instance is a sequence, even if it doesn't provide __len__, so we
    // double-check by testing for __len__ explicitly.
    bool is_sequence =
      (PySequence_Check(object) != 0) &&
      (PyObject_HasAttrString(object, "__len__") != 0);
    bool is_instance = false;

    const DCClass *dclass = nullptr;
    const DCPackerInterface *current_field = _this->get_current_field();
    if (current_field != nullptr) {
      const DCClassParameter *class_param = _this->get_current_field()->as_class_parameter();
      if (class_param != nullptr) {
        dclass = class_param->get_class();

        if (invoke_extension(dclass).has_class_def()) {
          PyObject *class_def = invoke_extension(dclass).get_class_def();
          is_instance = (PyObject_IsInstance(object, invoke_extension(dclass).get_class_def()) != 0);
          Py_DECREF(class_def);
        }
      }
    }

    // If dclass is not NULL, the packer is expecting a class object.  There
    // are then two cases: (1) the user has supplied a matching class object,
    // or (2) the user has supplied a sequence object.  Unfortunately, it may
    // be difficult to differentiate these two cases, since a class object may
    // also be a sequence object.

    // The rule to differentiate them is:

    // (1) If the supplied class object is an instance of the expected class
    // object, it is considered to be a class object.

    // (2) Otherwise, if the supplied class object has a __len__() method
    // (i.e.  PySequence_Check() returns true), then it is considered to be a
    // sequence.

    // (3) Otherwise, it is considered to be a class object.

    if (dclass != nullptr && (is_instance || !is_sequence)) {
      // The supplied object is either an instance of the expected class
      // object, or it is not a sequence--this is case (1) or (3).
      pack_class_object(dclass, object);
    } else if (is_sequence) {
      // The supplied object is not an instance of the expected class object,
      // but it is a sequence.  This is case (2).
      _this->push();
      int size = PySequence_Size(object);
      for (int i = 0; i < size; ++i) {
        PyObject *element = PySequence_GetItem(object, i);
        if (element != nullptr) {
          pack_object(element);
          Py_DECREF(element);
        } else {
          std::cerr << "Unable to extract item " << i << " from sequence.\n";
        }
      }
      _this->pop();
    } else {
      // The supplied object is not a sequence, and we weren't expecting a
      // class parameter.  This is none of the above, an error.
      std::ostringstream strm;
      strm << "Don't know how to pack object: "
           << Extension<DCField>::get_pystr(object);
      nassert_raise(strm.str());
      _this->_pack_error = true;
    }
  }
}

/**
 * Unpacks a Python object of the appropriate type from the stream for the
 * current field.  This may be an integer or a string for a simple field
 * object; if the current field represents a list of fields it will be a
 * tuple.
 */
PyObject *Extension<DCPacker>::
unpack_object() {
  PyObject *object = nullptr;

  DCPackType pack_type = _this->get_pack_type();

  switch (pack_type) {
  case PT_invalid:
    object = Py_None;
    Py_INCREF(object);
    _this->unpack_skip();
    break;

  case PT_double:
    {
      double value = _this->unpack_double();
      object = PyFloat_FromDouble(value);
    }
    break;

  case PT_int:
    {
      int value = _this->unpack_int();
#if PY_MAJOR_VERSION >= 3
      object = PyLong_FromLong(value);
#else
      object = PyInt_FromLong(value);
#endif
    }
    break;

  case PT_uint:
    {
      unsigned int value = _this->unpack_uint();
#if PY_MAJOR_VERSION >= 3
      object = PyLong_FromLong(value);
#else
      if (value & 0x80000000) {
        object = PyLong_FromUnsignedLong(value);
      } else {
        object = PyInt_FromLong(value);
      }
#endif
    }
    break;

  case PT_int64:
    {
      int64_t value = _this->unpack_int64();
      object = PyLong_FromLongLong(value);
    }
    break;

  case PT_uint64:
    {
      uint64_t value = _this->unpack_uint64();
      object = PyLong_FromUnsignedLongLong(value);
    }
    break;

  case PT_blob:
#if PY_MAJOR_VERSION >= 3
    {
      std::string str;
      _this->unpack_string(str);
      object = PyBytes_FromStringAndSize(str.data(), str.size());
    }
    break;
#endif
    // On Python 2, fall through to below.

  case PT_string:
    {
      std::string str;
      _this->unpack_string(str);
#if PY_MAJOR_VERSION >= 3
      object = PyUnicode_FromStringAndSize(str.data(), str.size());
#else
      object = PyString_FromStringAndSize(str.data(), str.size());
#endif
    }
    break;

  case PT_class:
    {
      const DCClassParameter *class_param = _this->get_current_field()->as_class_parameter();
      if (class_param != nullptr) {
        const DCClass *dclass = class_param->get_class();
        if (invoke_extension(dclass).has_class_def()) {
          // If we know what kind of class object this is and it has a valid
          // constructor, create the class object instead of just a tuple.
          object = unpack_class_object(dclass);
          if (object == nullptr) {
            std::cerr << "Unable to construct object of class "
                 << dclass->get_name() << "\n";
          } else {
            break;
          }
        }
      }
    }
    // Fall through (if no constructor)

    // If we don't know what kind of class object it is, or it doesn't have a
    // constructor, fall through and make a tuple.
  default:
    {
      // First, build up a list from the nested objects.
      object = PyList_New(0);

      _this->push();
      while (_this->more_nested_fields()) {
        PyObject *element = unpack_object();
        PyList_Append(object, element);
        Py_DECREF(element);
      }
      _this->pop();

      if (pack_type != PT_array) {
        // For these other kinds of objects, we'll convert the list into a
        // tuple.
        PyObject *tuple = PyList_AsTuple(object);
        Py_DECREF(object);
        object = tuple;
      }
    }
    break;
  }

  nassertr(object != nullptr, nullptr);
  return object;
}

/**
 * Given that the current element is a ClassParameter for a Python class
 * object, try to extract the appropriate values from the class object and
 * pack in.
 */
void Extension<DCPacker>::
pack_class_object(const DCClass *dclass, PyObject *object) {
  _this->push();
  while (_this->more_nested_fields() && !_this->_pack_error) {
    const DCField *field = _this->get_current_field()->as_field();
    nassertv(field != nullptr);
    get_class_element(dclass, object, field);
  }
  _this->pop();
}

/**
 * Given that the current element is a ClassParameter for a Python class for
 * which we have a valid constructor, unpack it and fill in its values.
 */
PyObject *Extension<DCPacker>::
unpack_class_object(const DCClass *dclass) {
  PyObject *class_def = invoke_extension(dclass).get_class_def();
  nassertr(class_def != nullptr, nullptr);

  PyObject *object = nullptr;

  if (!dclass->has_constructor()) {
    // If the class uses a default constructor, go ahead and create the Python
    // object for it now.
    object = PyObject_CallObject(class_def, nullptr);
    if (object == nullptr) {
      return nullptr;
    }
  }

  _this->push();
  if (object == nullptr && _this->more_nested_fields()) {
    // The first nested field will be the constructor.
    const DCField *field = _this->get_current_field()->as_field();
    nassertr(field != nullptr, object);
    nassertr(field == dclass->get_constructor(), object);

    set_class_element(class_def, object, field);

    // By now, the object should have been constructed.
    if (object == nullptr) {
      return nullptr;
    }
  }
  while (_this->more_nested_fields()) {
    const DCField *field = _this->get_current_field()->as_field();
    nassertr(field != nullptr, object);

    set_class_element(class_def, object, field);
  }
  _this->pop();

  return object;
}

/**
 * Unpacks the current element and stuffs it on the Python class object in
 * whatever way is appropriate.
 */
void Extension<DCPacker>::
set_class_element(PyObject *class_def, PyObject *&object,
                  const DCField *field) {
  std::string field_name = field->get_name();
  DCPackType pack_type = _this->get_pack_type();

  if (field_name.empty()) {
    switch (pack_type) {
    case PT_class:
    case PT_switch:
      // If the field has no name, but it is one of these container objects,
      // we want to unpack its nested objects directly into the class.
      _this->push();
      while (_this->more_nested_fields()) {
        const DCField *field = _this->get_current_field()->as_field();
        nassertv(field != nullptr);
        nassertv(object != nullptr);
        set_class_element(class_def, object, field);
      }
      _this->pop();
      break;

    default:
      // Otherwise, we just skip over the field.
      _this->unpack_skip();
    }

  } else {
    // If the field does have a name, we will want to store it on the class,
    // either by calling a method (for a PT_field pack_type) or by setting a
    // value (for any other kind of pack_type).

    PyObject *element = unpack_object();

    if (pack_type == PT_field) {
      if (object == nullptr) {
        // If the object hasn't been constructed yet, assume this is the
        // constructor.
        object = PyObject_CallObject(class_def, element);

      } else {
        if (PyObject_HasAttrString(object, (char *)field_name.c_str())) {
          PyObject *func = PyObject_GetAttrString(object, (char *)field_name.c_str());
          if (func != nullptr) {
            PyObject *result = PyObject_CallObject(func, element);
            Py_XDECREF(result);
            Py_DECREF(func);
          }
        }
      }

    } else {
      nassertv(object != nullptr);
      PyObject_SetAttrString(object, (char *)field_name.c_str(), element);
    }

    Py_DECREF(element);
  }
}

/**
 * Gets the current element from the Python object and packs it.
 */
void Extension<DCPacker>::
get_class_element(const DCClass *dclass, PyObject *object,
                  const DCField *field) {
  std::string field_name = field->get_name();
  DCPackType pack_type = _this->get_pack_type();

  if (field_name.empty()) {
    switch (pack_type) {
    case PT_class:
    case PT_switch:
      // If the field has no name, but it is one of these container objects,
      // we want to get its nested objects directly from the class.
      _this->push();
      while (_this->more_nested_fields() && !_this->_pack_error) {
        const DCField *field = _this->get_current_field()->as_field();
        nassertv(field != nullptr);
        get_class_element(dclass, object, field);
      }
      _this->pop();
      break;

    default:
      // Otherwise, we just pack the default value.
      _this->pack_default_value();
    }

  } else {
    // If the field does have a name, we will want to get it from the class
    // and pack it.  It just so happens that there's already a method that
    // does this on DCClass.

    if (!invoke_extension(dclass).pack_required_field(*_this, object, field)) {
      _this->_pack_error = true;
    }
  }
}

#endif  // HAVE_PYTHON
