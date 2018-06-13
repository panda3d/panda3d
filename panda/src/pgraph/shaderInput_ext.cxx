/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderInput_ext.cxx
 * @author rdb
 * @date 2017-10-06
 */

#include "shaderInput_ext.h"
#include "paramNodePath.h"

#ifdef HAVE_PYTHON

#ifndef CPPPARSER
extern struct Dtool_PyTypedObject Dtool_Texture;
extern struct Dtool_PyTypedObject Dtool_NodePath;
extern struct Dtool_PyTypedObject Dtool_PointerToVoid;
extern struct Dtool_PyTypedObject Dtool_PointerToArray_float;
extern struct Dtool_PyTypedObject Dtool_PointerToArray_double;
extern struct Dtool_PyTypedObject Dtool_PointerToArray_int;
extern struct Dtool_PyTypedObject Dtool_PointerToArray_UnalignedLVecBase4f;
extern struct Dtool_PyTypedObject Dtool_PointerToArray_LVecBase3f;
extern struct Dtool_PyTypedObject Dtool_PointerToArray_LVecBase2f;
extern struct Dtool_PyTypedObject Dtool_PointerToArray_UnalignedLMatrix4f;
extern struct Dtool_PyTypedObject Dtool_PointerToArray_LMatrix3f;
extern struct Dtool_PyTypedObject Dtool_PointerToArray_UnalignedLVecBase4d;
extern struct Dtool_PyTypedObject Dtool_PointerToArray_LVecBase3d;
extern struct Dtool_PyTypedObject Dtool_PointerToArray_LVecBase2d;
extern struct Dtool_PyTypedObject Dtool_PointerToArray_UnalignedLMatrix4d;
extern struct Dtool_PyTypedObject Dtool_PointerToArray_LMatrix3d;
extern struct Dtool_PyTypedObject Dtool_PointerToArray_UnalignedLVecBase4i;
extern struct Dtool_PyTypedObject Dtool_PointerToArray_LVecBase3i;
extern struct Dtool_PyTypedObject Dtool_PointerToArray_LVecBase2i;
extern struct Dtool_PyTypedObject Dtool_LMatrix4f;
extern struct Dtool_PyTypedObject Dtool_LMatrix3f;
extern struct Dtool_PyTypedObject Dtool_LMatrix4d;
extern struct Dtool_PyTypedObject Dtool_LMatrix3d;
extern struct Dtool_PyTypedObject Dtool_LVecBase4f;
extern struct Dtool_PyTypedObject Dtool_LVecBase3f;
extern struct Dtool_PyTypedObject Dtool_LVecBase2f;
extern struct Dtool_PyTypedObject Dtool_LVecBase4d;
extern struct Dtool_PyTypedObject Dtool_LVecBase3d;
extern struct Dtool_PyTypedObject Dtool_LVecBase2d;
extern struct Dtool_PyTypedObject Dtool_LVecBase4i;
extern struct Dtool_PyTypedObject Dtool_LVecBase3i;
extern struct Dtool_PyTypedObject Dtool_LVecBase2i;
extern struct Dtool_PyTypedObject Dtool_ShaderBuffer;
extern struct Dtool_PyTypedObject Dtool_ParamValueBase;
#endif  // CPPPARSER

/**
 * Sets a shader input from an arbitrary Python object.
 */
void Extension<ShaderInput>::
__init__(CPT_InternalName name, PyObject *value, int priority) {
  _this->_name = std::move(name);
  _this->_priority = priority;

  if (PyTuple_CheckExact(value) && PyTuple_GET_SIZE(value) <= 4) {
    // A tuple is interpreted as a vector.
    Py_ssize_t size = PyTuple_GET_SIZE(value);

    // If any of them is a float, we are storing it as a float vector.
    bool is_float = false;
    for (Py_ssize_t i = 0; i < size; ++i) {
      if (PyFloat_CheckExact(PyTuple_GET_ITEM(value, i))) {
        is_float = true;
        break;
      }
    }
    if (is_float) {
      LVecBase4 vec(0);
      for (Py_ssize_t i = 0; i < size; ++i) {
        vec[i] = (PN_stdfloat)PyFloat_AsDouble(PyTuple_GET_ITEM(value, i));
      }
      _this->_type = ShaderInput::M_vector;
      _this->_stored_ptr = vec;
      _this->_stored_vector = vec;
    } else {
      LVecBase4i vec(0);
      for (Py_ssize_t i = 0; i < size; ++i) {
        vec[i] = (int)PyLong_AsLong(PyTuple_GET_ITEM(value, i));
      }
      _this->_type = ShaderInput::M_numeric;
      _this->_stored_ptr = vec;
      _this->_stored_vector = LCAST(PN_stdfloat, vec);
    }

  } else if (DtoolInstance_Check(value)) {
    void *ptr;

    if ((ptr = DtoolInstance_UPCAST(value, Dtool_Texture))) {
      _this->_type = ShaderInput::M_texture;
      _this->_value = (Texture *)ptr;

    } else if ((ptr = DtoolInstance_UPCAST(value, Dtool_NodePath))) {
      _this->_type = ShaderInput::M_nodepath;
      _this->_value = new ParamNodePath(*(const NodePath *)ptr);

    } else if (DtoolInstance_UPCAST(value, Dtool_PointerToVoid)) {
      if ((ptr = DtoolInstance_UPCAST(value, Dtool_PointerToArray_float))) {
        _this->_type = ShaderInput::M_numeric;
        _this->_stored_ptr = *(const PTA_float *)ptr;

      } else if ((ptr = DtoolInstance_UPCAST(value, Dtool_PointerToArray_double))) {
        _this->_type = ShaderInput::M_numeric;
        _this->_stored_ptr = *(const PTA_double *)ptr;

      } else if ((ptr = DtoolInstance_UPCAST(value, Dtool_PointerToArray_int))) {
        _this->_type = ShaderInput::M_numeric;
        _this->_stored_ptr = *(const PTA_int *)ptr;

      } else if ((ptr = DtoolInstance_UPCAST(value, Dtool_PointerToArray_UnalignedLVecBase4f))) {
        _this->_type = ShaderInput::M_numeric;
        _this->_stored_ptr = *(const PTA_LVecBase4f *)ptr;

      } else if ((ptr = DtoolInstance_UPCAST(value, Dtool_PointerToArray_LVecBase3f))) {
        _this->_type = ShaderInput::M_numeric;
        _this->_stored_ptr = *(const PTA_LVecBase3f *)ptr;

      } else if ((ptr = DtoolInstance_UPCAST(value, Dtool_PointerToArray_LVecBase2f))) {
        _this->_type = ShaderInput::M_numeric;
        _this->_stored_ptr = *(const PTA_LVecBase2f *)ptr;

      } else if ((ptr = DtoolInstance_UPCAST(value, Dtool_PointerToArray_UnalignedLMatrix4f))) {
        _this->_type = ShaderInput::M_numeric;
        _this->_stored_ptr = *(const PTA_LMatrix4f *)ptr;

      } else if ((ptr = DtoolInstance_UPCAST(value, Dtool_PointerToArray_LMatrix3f))) {
        _this->_type = ShaderInput::M_numeric;
        _this->_stored_ptr = *(const PTA_LMatrix3f *)ptr;

      } else if ((ptr = DtoolInstance_UPCAST(value, Dtool_PointerToArray_UnalignedLVecBase4d))) {
        _this->_type = ShaderInput::M_numeric;
        _this->_stored_ptr = *(const PTA_LVecBase4d *)ptr;

      } else if ((ptr = DtoolInstance_UPCAST(value, Dtool_PointerToArray_LVecBase3d))) {
        _this->_type = ShaderInput::M_numeric;
        _this->_stored_ptr = *(const PTA_LVecBase3d *)ptr;

      } else if ((ptr = DtoolInstance_UPCAST(value, Dtool_PointerToArray_LVecBase2d))) {
        _this->_type = ShaderInput::M_numeric;
        _this->_stored_ptr = *(const PTA_LVecBase2d *)ptr;

      } else if ((ptr = DtoolInstance_UPCAST(value, Dtool_PointerToArray_UnalignedLMatrix4d))) {
        _this->_type = ShaderInput::M_numeric;
        _this->_stored_ptr = *(const PTA_LMatrix4d *)ptr;

      } else if ((ptr = DtoolInstance_UPCAST(value, Dtool_PointerToArray_LMatrix3d))) {
        _this->_type = ShaderInput::M_numeric;
        _this->_stored_ptr = *(const PTA_LMatrix3d *)ptr;

      } else if ((ptr = DtoolInstance_UPCAST(value, Dtool_PointerToArray_UnalignedLVecBase4i))) {
        _this->_type = ShaderInput::M_numeric;
        _this->_stored_ptr = *(const PTA_LVecBase4i *)ptr;

      } else if ((ptr = DtoolInstance_UPCAST(value, Dtool_PointerToArray_LVecBase3i))) {
        _this->_type = ShaderInput::M_numeric;
        _this->_stored_ptr = *(const PTA_LVecBase3i *)ptr;

      } else if ((ptr = DtoolInstance_UPCAST(value, Dtool_PointerToArray_LVecBase2i))) {
        _this->_type = ShaderInput::M_numeric;
        _this->_stored_ptr = *(const PTA_LVecBase2i *)ptr;

      } else {
        Dtool_Raise_TypeError("unknown type passed to ShaderInput");
        return;
      }

    } else if ((ptr = DtoolInstance_UPCAST(value, Dtool_LMatrix4f))) {
      _this->_type = ShaderInput::M_numeric;
      _this->_stored_ptr = *(const LMatrix4f *)ptr;

    } else if ((ptr = DtoolInstance_UPCAST(value, Dtool_LMatrix3f))) {
      _this->_type = ShaderInput::M_numeric;
      _this->_stored_ptr = *(const LMatrix3f *)ptr;

    } else if ((ptr = DtoolInstance_UPCAST(value, Dtool_LMatrix4d))) {
      _this->_type = ShaderInput::M_numeric;
      _this->_stored_ptr = *(const LMatrix4d *)ptr;

    } else if ((ptr = DtoolInstance_UPCAST(value, Dtool_LMatrix3d))) {
      _this->_type = ShaderInput::M_numeric;
      _this->_stored_ptr = *(const LMatrix3d *)ptr;

    } else if ((ptr = DtoolInstance_UPCAST(value, Dtool_LVecBase4f))) {
      const LVecBase4f &vec = *(const LVecBase4f *)ptr;
      _this->_type = ShaderInput::M_vector;
      _this->_stored_ptr = vec;
      _this->_stored_vector = LCAST(PN_stdfloat, vec);

    } else if ((ptr = DtoolInstance_UPCAST(value, Dtool_LVecBase3f))) {
      const LVecBase3f &vec = *(const LVecBase3f *)ptr;
      _this->_type = ShaderInput::M_vector;
      _this->_stored_ptr = vec;
      _this->_stored_vector.set(vec[0], vec[1], vec[2], 0);

    } else if ((ptr = DtoolInstance_UPCAST(value, Dtool_LVecBase2f))) {
      const LVecBase2f &vec = *(const LVecBase2f *)ptr;
      _this->_type = ShaderInput::M_vector;
      _this->_stored_ptr = vec;
      _this->_stored_vector.set(vec[0], vec[1], 0, 0);

    } else if ((ptr = DtoolInstance_UPCAST(value, Dtool_LVecBase4d))) {
      const LVecBase4d &vec = *(const LVecBase4d *)ptr;
      _this->_type = ShaderInput::M_vector;
      _this->_stored_ptr = vec;
      _this->_stored_vector = LCAST(PN_stdfloat, vec);

    } else if ((ptr = DtoolInstance_UPCAST(value, Dtool_LVecBase3d))) {
      const LVecBase3d &vec = *(const LVecBase3d *)ptr;
      _this->_type = ShaderInput::M_vector;
      _this->_stored_ptr = vec;
      _this->_stored_vector.set(vec[0], vec[1], vec[2], 0);

    } else if ((ptr = DtoolInstance_UPCAST(value, Dtool_LVecBase2d))) {
      const LVecBase2d &vec = *(const LVecBase2d *)ptr;
      _this->_type = ShaderInput::M_vector;
      _this->_stored_ptr = vec;
      _this->_stored_vector.set(vec[0], vec[1], 0, 0);

    } else if ((ptr = DtoolInstance_UPCAST(value, Dtool_LVecBase4i))) {
      const LVecBase4i &vec = *(const LVecBase4i *)ptr;
      _this->_type = ShaderInput::M_numeric;
      _this->_stored_ptr = vec;
      _this->_stored_vector = LCAST(PN_stdfloat, vec);

    } else if ((ptr = DtoolInstance_UPCAST(value, Dtool_LVecBase3i))) {
      const LVecBase3i &vec = *(const LVecBase3i *)ptr;
      _this->_type = ShaderInput::M_numeric;
      _this->_stored_ptr = vec;
      _this->_stored_vector.set(vec[0], vec[1], vec[2], 0);

    } else if ((ptr = DtoolInstance_UPCAST(value, Dtool_LVecBase2i))) {
      const LVecBase2i &vec = *(const LVecBase2i *)ptr;
      _this->_type = ShaderInput::M_numeric;
      _this->_stored_ptr = vec;
      _this->_stored_vector.set(vec[0], vec[1], 0, 0);

    } else if ((ptr = DtoolInstance_UPCAST(value, Dtool_ShaderBuffer))) {
      _this->_type = ShaderInput::M_buffer;
      _this->_value = (ShaderBuffer *)ptr;

    } else if ((ptr = DtoolInstance_UPCAST(value, Dtool_ParamValueBase))) {
      _this->_type = ShaderInput::M_param;
      _this->_value = (ParamValueBase *)ptr;

    } else {
      Dtool_Raise_TypeError("unknown type passed to ShaderInput");
      return;
    }

  } else if (PyFloat_Check(value)) {
    LVecBase4 vec(PyFloat_AS_DOUBLE(value), 0, 0, 0);
    _this->_type = ShaderInput::M_vector;
    _this->_stored_ptr = vec;
    _this->_stored_vector = vec;

#if PY_MAJOR_VERSION < 3
  } else if (PyInt_Check(value)) {
    LVecBase4i vec((int)PyInt_AS_LONG(value), 0, 0, 0);
    _this->_type = ShaderInput::M_numeric;
    _this->_stored_ptr = vec;
    _this->_stored_vector.set((PN_stdfloat)vec[0], 0, 0, 0);
#endif

  } else if (PyLong_Check(value)) {
    LVecBase4i vec((int)PyLong_AsLong(value), 0, 0, 0);
    _this->_type = ShaderInput::M_numeric;
    _this->_stored_ptr = vec;
    _this->_stored_vector.set((PN_stdfloat)vec[0], 0, 0, 0);

  } else if (PySequence_Check(value) && !PyUnicode_CheckExact(value)) {
    // Iterate over the sequence to make sure all have the same type.
    PyObject *fast = PySequence_Fast(value, "unknown type passed to ShaderInput");
    if (fast == nullptr) {
      return;
    }

    Py_ssize_t num_items = PySequence_Fast_GET_SIZE(value);
    if (num_items <= 0) {
      // We can't determine the type of a list of size 0.
      _this->_type = ShaderInput::M_numeric;
      Py_DECREF(fast);
      return;
    }

    bool has_float = false;
    Py_ssize_t known_itemsize = -1;

    PyObject **items = PySequence_Fast_ITEMS(fast);
    for (Py_ssize_t i = 0; i < num_items; ++i) {
      PyObject *item = items[i];

      if (PySequence_Check(item)) {
        Py_ssize_t itemsize = PySequence_Size(item);
        if (known_itemsize >= 0 && itemsize != known_itemsize) {
          Dtool_Raise_TypeError("inconsistent sequence length among elements of sequence passed to ShaderInput");
          Py_DECREF(fast);
          return;
        }
        known_itemsize = itemsize;

        // Check their types.
        for (Py_ssize_t j = 0; j < itemsize; ++j) {
          PyObject *subitem = PySequence_ITEM(item, j);
          if (PyFloat_CheckExact(subitem)) {
            Py_DECREF(subitem);
            has_float = true;
            break;
          } else if (PyLongOrInt_Check(subitem)) {
          } else {
            Dtool_Raise_TypeError("unknown element type in sequence passed as element of sequence passed to ShaderInput");
            Py_DECREF(subitem);
            Py_DECREF(fast);
            break;
          }
          Py_DECREF(subitem);
        }
      } else if (PyFloat_CheckExact(item)) {
        has_float = true;
      } else if (PyLongOrInt_Check(item)) {
      } else {
        Dtool_Raise_TypeError("unknown element type in sequence passed to ShaderInput");
        Py_DECREF(fast);
        return;
      }
    }

    // Now that we have verified the dimensions and type of the PTA, we can
    // read in the actual elements.
    switch (known_itemsize) {
    case -1:
      if (has_float) {
        PTA_float pta;
        pta.reserve(num_items);
        for (Py_ssize_t i = 0; i < num_items; ++i) {
          pta.push_back(PyFloat_AsDouble(items[i]));
        }
        _this->_stored_ptr = pta;
      } else {
        PTA_int pta;
        pta.reserve(num_items);
        for (Py_ssize_t i = 0; i < num_items; ++i) {
          pta.push_back((int)PyLongOrInt_AS_LONG(items[i]));
        }
        _this->_stored_ptr = pta;
      }
      break;

    case 1:
      if (has_float) {
        PTA_float pta;
        pta.reserve(num_items);
        for (Py_ssize_t i = 0; i < num_items; ++i) {
          PyObject *item = items[i];
          if (PySequence_Check(item)) {
            PyObject *subitem = PySequence_ITEM(item, 0);
            pta.push_back(PyFloat_AsDouble(subitem));
            Py_DECREF(subitem);
          } else {
            pta.push_back(PyFloat_AsDouble(item));
          }
        }
        _this->_stored_ptr = pta;
      } else {
        PTA_int pta;
        pta.reserve(num_items);
        for (Py_ssize_t i = 0; i < num_items; ++i) {
          PyObject *item = items[i];
          if (PySequence_Check(item)) {
            PyObject *subitem = PySequence_ITEM(item, 0);
            pta.push_back((int)PyLongOrInt_AS_LONG(subitem));
            Py_DECREF(subitem);
          } else {
            pta.push_back((int)PyLongOrInt_AS_LONG(item));
          }
        }
        _this->_stored_ptr = pta;
      }
      break;

    case 2:
      if (has_float) {
        PTA_LVecBase2f pta;
        pta.reserve(num_items);
        for (Py_ssize_t i = 0; i < num_items; ++i) {
          PyObject *item = items[i];
          if (PySequence_Check(item)) {
            PyObject *subitem0 = PySequence_ITEM(item, 0);
            PyObject *subitem1 = PySequence_ITEM(item, 1);
            pta.push_back(LVecBase2f(PyFloat_AsDouble(subitem0),
                                     PyFloat_AsDouble(subitem1)));
            Py_DECREF(subitem0);
            Py_DECREF(subitem1);
          } else {
            pta.push_back(PyFloat_AsDouble(item));
          }
        }
        _this->_stored_ptr = pta;
      } else {
        PTA_LVecBase2i pta;
        pta.reserve(num_items);
        for (Py_ssize_t i = 0; i < num_items; ++i) {
          PyObject *item = items[i];
          if (PySequence_Check(item)) {
            PyObject *subitem0 = PySequence_ITEM(item, 0);
            PyObject *subitem1 = PySequence_ITEM(item, 1);
            pta.push_back(LVecBase2i((int)PyLongOrInt_AS_LONG(subitem0),
                                     (int)PyLongOrInt_AS_LONG(subitem1)));
            Py_DECREF(subitem0);
            Py_DECREF(subitem1);
          } else {
            pta.push_back((int)PyLongOrInt_AS_LONG(item));
          }
        }
        _this->_stored_ptr = pta;
      }
      break;

    case 3:
      if (has_float) {
        PTA_LVecBase3f pta;
        pta.reserve(num_items);
        for (Py_ssize_t i = 0; i < num_items; ++i) {
          PyObject *item = items[i];
          if (PySequence_Check(item)) {
            PyObject *subitem0 = PySequence_ITEM(item, 0);
            PyObject *subitem1 = PySequence_ITEM(item, 1);
            PyObject *subitem2 = PySequence_ITEM(item, 2);
            pta.push_back(LVecBase3f(PyFloat_AsDouble(subitem0),
                                     PyFloat_AsDouble(subitem1),
                                     PyFloat_AsDouble(subitem2)));
            Py_DECREF(subitem0);
            Py_DECREF(subitem1);
            Py_DECREF(subitem2);
          } else {
            pta.push_back(PyFloat_AsDouble(item));
          }
        }
        _this->_stored_ptr = pta;
      } else {
        PTA_LVecBase3i pta;
        pta.reserve(num_items);
        for (Py_ssize_t i = 0; i < num_items; ++i) {
          PyObject *item = items[i];
          if (PySequence_Check(item)) {
            PyObject *subitem0 = PySequence_ITEM(item, 0);
            PyObject *subitem1 = PySequence_ITEM(item, 1);
            PyObject *subitem2 = PySequence_ITEM(item, 2);
            pta.push_back(LVecBase3i((int)PyLongOrInt_AS_LONG(subitem0),
                                     (int)PyLongOrInt_AS_LONG(subitem1),
                                     (int)PyLongOrInt_AS_LONG(subitem2)));
            Py_DECREF(subitem0);
            Py_DECREF(subitem1);
            Py_DECREF(subitem2);
          } else {
            pta.push_back((int)PyLongOrInt_AS_LONG(item));
          }
        }
        _this->_stored_ptr = pta;
      }
      break;

    case 4:
      if (has_float) {
        PTA_LVecBase4f pta;
        pta.reserve(num_items);
        for (Py_ssize_t i = 0; i < num_items; ++i) {
          PyObject *item = items[i];
          if (PySequence_Check(item)) {
            PyObject *subitem0 = PySequence_ITEM(item, 0);
            PyObject *subitem1 = PySequence_ITEM(item, 1);
            PyObject *subitem2 = PySequence_ITEM(item, 2);
            PyObject *subitem3 = PySequence_ITEM(item, 3);
            pta.push_back(LVecBase4f(PyFloat_AsDouble(subitem0),
                                     PyFloat_AsDouble(subitem1),
                                     PyFloat_AsDouble(subitem2),
                                     PyFloat_AsDouble(subitem3)));
            Py_DECREF(subitem0);
            Py_DECREF(subitem1);
            Py_DECREF(subitem2);
            Py_DECREF(subitem3);
          } else {
            pta.push_back(PyFloat_AsDouble(item));
          }
        }
        _this->_stored_ptr = pta;
      } else {
        PTA_LVecBase4i pta;
        pta.reserve(num_items);
        for (Py_ssize_t i = 0; i < num_items; ++i) {
          PyObject *item = items[i];
          if (PySequence_Check(item)) {
            PyObject *subitem0 = PySequence_ITEM(item, 0);
            PyObject *subitem1 = PySequence_ITEM(item, 1);
            PyObject *subitem2 = PySequence_ITEM(item, 2);
            PyObject *subitem3 = PySequence_ITEM(item, 3);
            pta.push_back(LVecBase4i((int)PyLongOrInt_AS_LONG(subitem0),
                                     (int)PyLongOrInt_AS_LONG(subitem1),
                                     (int)PyLongOrInt_AS_LONG(subitem2),
                                     (int)PyLongOrInt_AS_LONG(subitem3)));
            Py_DECREF(subitem0);
            Py_DECREF(subitem1);
            Py_DECREF(subitem2);
            Py_DECREF(subitem3);
          } else {
            pta.push_back((int)PyLongOrInt_AS_LONG(item));
          }
        }
        _this->_stored_ptr = pta;
      }
      break;

    case 0:
      Dtool_Raise_TypeError("sequence passed to ShaderInput contains an empty sequence");
      break;

    default:
      Dtool_Raise_TypeError("sequence passed to ShaderInput contains a sequence of more than 4 elements");
      break;
    }

    _this->_type = ShaderInput::M_numeric;

    Py_DECREF(fast);

  } else {
    Dtool_Raise_TypeError("unknown type passed to ShaderInput");
  }
}

#endif  // HAVE_PYTHON
