/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nodePath_ext.cxx
 * @author rdb
 * @date 2013-12-09
 */

#include "nodePath_ext.h"
#include "typedWritable_ext.h"
#include "shaderAttrib.h"

#ifdef HAVE_PYTHON

#ifndef CPPPARSER
extern struct Dtool_PyTypedObject Dtool_BamWriter;
extern struct Dtool_PyTypedObject Dtool_BamReader;
#ifdef STDFLOAT_DOUBLE
extern struct Dtool_PyTypedObject Dtool_LPoint3d;
#else
extern struct Dtool_PyTypedObject Dtool_LPoint3f;
#endif
extern struct Dtool_PyTypedObject Dtool_Texture;
extern struct Dtool_PyTypedObject Dtool_NodePath;
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
 * A special Python method that is invoked by copy.copy(node).  Unlike the
 * NodePath copy constructor, this makes a duplicate copy of the underlying
 * PandaNode (but shares children, instead of copying them or omitting them).
 */
NodePath Extension<NodePath>::
__copy__() const {
  if (_this->is_empty()) {
    // Invoke the copy constructor if we have no node.
    return *_this;
  }

  // If we do have a node, duplicate it, and wrap it in a new NodePath.
  return NodePath(invoke_extension(_this->node()).__copy__());
}

/**
 * A special Python method that is invoked by copy.deepcopy(np).  This calls
 * copy_to() unless the NodePath is already present in the provided
 * dictionary.
 */
PyObject *Extension<NodePath>::
__deepcopy__(PyObject *self, PyObject *memo) const {
  extern struct Dtool_PyTypedObject Dtool_NodePath;

  // Borrowed reference.
  PyObject *dupe = PyDict_GetItem(memo, self);
  if (dupe != NULL) {
    // Already in the memo dictionary.
    Py_INCREF(dupe);
    return dupe;
  }

  NodePath *np_dupe;
  if (_this->is_empty()) {
    np_dupe = new NodePath(*_this);
  } else {
    np_dupe = new NodePath(_this->copy_to(NodePath()));
  }

  dupe = DTool_CreatePyInstance((void *)np_dupe, Dtool_NodePath,
                                true, false);
  if (PyDict_SetItem(memo, self, dupe) != 0) {
    Py_DECREF(dupe);
    return NULL;
  }

  return dupe;
}

/**
 * This special Python method is implement to provide support for the pickle
 * module.
 *
 * This hooks into the native pickle and cPickle modules, but it cannot
 * properly handle self-referential BAM objects.
 */
PyObject *Extension<NodePath>::
__reduce__(PyObject *self) const {
  return __reduce_persist__(self, NULL);
}

/**
 * This special Python method is implement to provide support for the pickle
 * module.
 *
 * This is similar to __reduce__, but it provides additional support for the
 * missing persistent-state object needed to properly support self-referential
 * BAM objects written to the pickle stream.  This hooks into the pickle and
 * cPickle modules implemented in direct/src/stdpy.
 */
PyObject *Extension<NodePath>::
__reduce_persist__(PyObject *self, PyObject *pickler) const {
  // We should return at least a 2-tuple, (Class, (args)): the necessary class
  // object whose constructor we should call (e.g.  this), and the arguments
  // necessary to reconstruct this object.

  BamWriter *writer = NULL;
  if (pickler != NULL) {
    PyObject *py_writer = PyObject_GetAttrString(pickler, "bamWriter");
    if (py_writer == NULL) {
      // It's OK if there's no bamWriter.
      PyErr_Clear();
    } else {
      DTOOL_Call_ExtractThisPointerForType(py_writer, &Dtool_BamWriter, (void **)&writer);
      Py_DECREF(py_writer);
    }
  }

  // We have a non-empty NodePath.

  string bam_stream;
  if (!_this->encode_to_bam_stream(bam_stream, writer)) {
    ostringstream stream;
    stream << "Could not bamify " << _this;
    string message = stream.str();
    PyErr_SetString(PyExc_TypeError, message.c_str());
    return NULL;
  }

  // Start by getting this class object.
  PyObject *this_class = PyObject_Type(self);
  if (this_class == NULL) {
    return NULL;
  }

  PyObject *func;
  if (writer != NULL) {
    // The modified pickle support: call the "persistent" version of this
    // function, which receives the unpickler itself as an additional
    // parameter.
    func = Extension<TypedWritable>::find_global_decode(this_class, "py_decode_NodePath_from_bam_stream_persist");
    if (func == NULL) {
      PyErr_SetString(PyExc_TypeError, "Couldn't find py_decode_NodePath_from_bam_stream_persist()");
      Py_DECREF(this_class);
      return NULL;
    }

  } else {
    // The traditional pickle support: call the non-persistent version of this
    // function.

    func = Extension<TypedWritable>::find_global_decode(this_class, "py_decode_NodePath_from_bam_stream");
    if (func == NULL) {
      PyErr_SetString(PyExc_TypeError, "Couldn't find py_decode_NodePath_from_bam_stream()");
      Py_DECREF(this_class);
      return NULL;
    }
  }

#if PY_MAJOR_VERSION >= 3
  PyObject *result = Py_BuildValue("(O(y#))", func, bam_stream.data(), (Py_ssize_t) bam_stream.size());
#else
  PyObject *result = Py_BuildValue("(O(s#))", func, bam_stream.data(), (Py_ssize_t) bam_stream.size());
#endif
  Py_DECREF(func);
  Py_DECREF(this_class);
  return result;
}

/**
 * Returns the lowest ancestor of this node that contains a tag definition
 * with the indicated key, if any, or an empty NodePath if no ancestor of this
 * node contains this tag definition.  See set_python_tag().
 */
NodePath Extension<NodePath>::
find_net_python_tag(PyObject *key) const {
  if (_this->is_empty()) {
    return NodePath::not_found();
  }
  if (has_python_tag(key)) {
    return *_this;
  }
  NodePath parent = _this->get_parent();
  return invoke_extension(&parent).find_net_python_tag(key);
}

/**
 * This wrapper is defined as a global function to suit pickle's needs.
 */
NodePath
py_decode_NodePath_from_bam_stream(const string &data) {
  return py_decode_NodePath_from_bam_stream_persist(NULL, data);
}

/**
 * This wrapper is defined as a global function to suit pickle's needs.
 */
NodePath
py_decode_NodePath_from_bam_stream_persist(PyObject *unpickler, const string &data) {
  BamReader *reader = NULL;
  if (unpickler != NULL) {
    PyObject *py_reader = PyObject_GetAttrString(unpickler, "bamReader");
    if (py_reader == NULL) {
      // It's OK if there's no bamReader.
      PyErr_Clear();
    } else {
      DTOOL_Call_ExtractThisPointerForType(py_reader, &Dtool_BamReader, (void **)&reader);
      Py_DECREF(py_reader);
    }
  }

  return NodePath::decode_from_bam_stream(data, reader);
}

/**
 * Sets multiple shader inputs at the same time.  This can be significantly
 * more efficient if many inputs need to be set at the same time.
 */
void Extension<NodePath>::
set_shader_inputs(PyObject *args, PyObject *kwargs) {
  if (PyObject_Size(args) > 0) {
    Dtool_Raise_TypeError("NodePath.set_shader_inputs takes only keyword arguments");
    return;
  }

  PT(PandaNode) node = _this->node();
  CPT(RenderAttrib) prev_attrib = node->get_attrib(ShaderAttrib::get_class_slot());
  PT(ShaderAttrib) attrib;
  if (prev_attrib == nullptr) {
    attrib = new ShaderAttrib();
  } else {
    attrib = new ShaderAttrib(*(const ShaderAttrib *)prev_attrib.p());
  }

  PyObject *key, *value;
  Py_ssize_t pos = 0;

  while (PyDict_Next(kwargs, &pos, &key, &value)) {
    char *buffer;
    Py_ssize_t length;
#if PY_MAJOR_VERSION >= 3
    buffer = (char *)PyUnicode_AsUTF8AndSize(key, &length);
    if (buffer == nullptr) {
#else
    if (PyString_AsStringAndSize(key, &buffer, &length) == -1) {
#endif
      Dtool_Raise_TypeError("NodePath.set_shader_inputs accepts only string keywords");
      return;
    }

    CPT_InternalName name(string(buffer, length));
    ShaderInput input(nullptr, 0);

    if (PyTuple_CheckExact(value)) {
      // A tuple is interpreted as a vector.
      Py_ssize_t size = PyTuple_GET_SIZE(value);
      if (size > 4) {
        Dtool_Raise_TypeError("NodePath.set_shader_inputs tuple input should not have more than 4 scalars");
        return;
      }
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
        input = ShaderInput(move(name), vec);
      } else {
        LVecBase4i vec(0);
        for (Py_ssize_t i = 0; i < size; ++i) {
          vec[i] = (int)PyLong_AsLong(PyTuple_GET_ITEM(value, i));
        }
        input = ShaderInput(move(name), vec);
      }

    } else if (DtoolCanThisBeAPandaInstance(value)) {
      Dtool_PyInstDef *inst = (Dtool_PyInstDef *)value;
      void *ptr;

      if ((ptr = inst->_My_Type->_Dtool_UpcastInterface(value, &Dtool_Texture))) {
        input = ShaderInput(move(name), (Texture *)ptr);

      } else if ((ptr = inst->_My_Type->_Dtool_UpcastInterface(value, &Dtool_NodePath))) {
        input = ShaderInput(move(name), *(const NodePath *)ptr);

      } else if ((ptr = inst->_My_Type->_Dtool_UpcastInterface(value, &Dtool_PointerToArray_float))) {
        input = ShaderInput(move(name), *(const PTA_float *)ptr);

      } else if ((ptr = inst->_My_Type->_Dtool_UpcastInterface(value, &Dtool_PointerToArray_double))) {
        input = ShaderInput(move(name), *(const PTA_double *)ptr);

      } else if ((ptr = inst->_My_Type->_Dtool_UpcastInterface(value, &Dtool_PointerToArray_int))) {
        input = ShaderInput(move(name), *(const PTA_int *)ptr);

      } else if ((ptr = inst->_My_Type->_Dtool_UpcastInterface(value, &Dtool_PointerToArray_UnalignedLVecBase4f))) {
        input = ShaderInput(move(name), *(const PTA_LVecBase4f *)ptr);

      } else if ((ptr = inst->_My_Type->_Dtool_UpcastInterface(value, &Dtool_PointerToArray_LVecBase3f))) {
        input = ShaderInput(move(name), *(const PTA_LVecBase3f *)ptr);

      } else if ((ptr = inst->_My_Type->_Dtool_UpcastInterface(value, &Dtool_PointerToArray_LVecBase2f))) {
        input = ShaderInput(move(name), *(const PTA_LVecBase2f *)ptr);

      } else if ((ptr = inst->_My_Type->_Dtool_UpcastInterface(value, &Dtool_PointerToArray_UnalignedLMatrix4f))) {
        input = ShaderInput(move(name), *(const PTA_LMatrix4f *)ptr);

      } else if ((ptr = inst->_My_Type->_Dtool_UpcastInterface(value, &Dtool_PointerToArray_LMatrix3f))) {
        input = ShaderInput(move(name), *(const PTA_LMatrix3f *)ptr);

      } else if ((ptr = inst->_My_Type->_Dtool_UpcastInterface(value, &Dtool_PointerToArray_UnalignedLVecBase4d))) {
        input = ShaderInput(move(name), *(const PTA_LVecBase4d *)ptr);

      } else if ((ptr = inst->_My_Type->_Dtool_UpcastInterface(value, &Dtool_PointerToArray_LVecBase3d))) {
        input = ShaderInput(move(name), *(const PTA_LVecBase3d *)ptr);

      } else if ((ptr = inst->_My_Type->_Dtool_UpcastInterface(value, &Dtool_PointerToArray_LVecBase2d))) {
        input = ShaderInput(move(name), *(const PTA_LVecBase2d *)ptr);

      } else if ((ptr = inst->_My_Type->_Dtool_UpcastInterface(value, &Dtool_PointerToArray_UnalignedLMatrix4d))) {
        input = ShaderInput(move(name), *(const PTA_LMatrix4d *)ptr);

      } else if ((ptr = inst->_My_Type->_Dtool_UpcastInterface(value, &Dtool_PointerToArray_LMatrix3d))) {
        input = ShaderInput(move(name), *(const PTA_LMatrix3d *)ptr);

      } else if ((ptr = inst->_My_Type->_Dtool_UpcastInterface(value, &Dtool_PointerToArray_UnalignedLVecBase4i))) {
        input = ShaderInput(move(name), *(const PTA_LVecBase4i *)ptr);

      } else if ((ptr = inst->_My_Type->_Dtool_UpcastInterface(value, &Dtool_PointerToArray_LVecBase3i))) {
        input = ShaderInput(move(name), *(const PTA_LVecBase3i *)ptr);

      } else if ((ptr = inst->_My_Type->_Dtool_UpcastInterface(value, &Dtool_PointerToArray_LVecBase2i))) {
        input = ShaderInput(move(name), *(const PTA_LVecBase2i *)ptr);

      } else if ((ptr = inst->_My_Type->_Dtool_UpcastInterface(value, &Dtool_LVecBase4f))) {
        input = ShaderInput(move(name), *(const LVecBase4f *)ptr);

      } else if ((ptr = inst->_My_Type->_Dtool_UpcastInterface(value, &Dtool_LVecBase3f))) {
        input = ShaderInput(move(name), *(const LVecBase3f *)ptr);

      } else if ((ptr = inst->_My_Type->_Dtool_UpcastInterface(value, &Dtool_LVecBase2f))) {
        input = ShaderInput(move(name), *(const LVecBase2f *)ptr);

      } else if ((ptr = inst->_My_Type->_Dtool_UpcastInterface(value, &Dtool_LVecBase4d))) {
        input = ShaderInput(move(name), *(const LVecBase4d *)ptr);

      } else if ((ptr = inst->_My_Type->_Dtool_UpcastInterface(value, &Dtool_LVecBase3d))) {
        input = ShaderInput(move(name), *(const LVecBase3d *)ptr);

      } else if ((ptr = inst->_My_Type->_Dtool_UpcastInterface(value, &Dtool_LVecBase2d))) {
        input = ShaderInput(move(name), *(const LVecBase2d *)ptr);

      } else if ((ptr = inst->_My_Type->_Dtool_UpcastInterface(value, &Dtool_LVecBase4i))) {
        input = ShaderInput(move(name), *(const LVecBase4i *)ptr);

      } else if ((ptr = inst->_My_Type->_Dtool_UpcastInterface(value, &Dtool_LVecBase3i))) {
        input = ShaderInput(move(name), *(const LVecBase3i *)ptr);

      } else if ((ptr = inst->_My_Type->_Dtool_UpcastInterface(value, &Dtool_LVecBase2i))) {
        input = ShaderInput(move(name), *(const LVecBase2i *)ptr);

      } else if ((ptr = inst->_My_Type->_Dtool_UpcastInterface(value, &Dtool_ShaderBuffer))) {
        input = ShaderInput(move(name), (ShaderBuffer *)ptr);

      } else if ((ptr = inst->_My_Type->_Dtool_UpcastInterface(value, &Dtool_ParamValueBase))) {
        input = ShaderInput(move(name), (ParamValueBase *)ptr);

      } else {
        Dtool_Raise_TypeError("unknown type passed to NodePath.set_shader_inputs");
        return;
      }

    } else if (PyFloat_Check(value)) {
      input = ShaderInput(move(name), LVecBase4(PyFloat_AS_DOUBLE(value), 0, 0, 0));

#if PY_MAJOR_VERSION < 3
    } else if (PyInt_Check(value)) {
      input = ShaderInput(move(name), LVecBase4i((int)PyInt_AS_LONG(value), 0, 0, 0));
#endif

    } else if (PyLong_Check(value)) {
      input = ShaderInput(move(name), LVecBase4i((int)PyLong_AsLong(value), 0, 0, 0));

    } else {
      Dtool_Raise_TypeError("unknown type passed to NodePath.set_shader_inputs");
      return;
    }

    attrib->_inputs[input.get_name()] = move(input);
  }

  node->set_attrib(ShaderAttrib::return_new(attrib));
}

/**
 * Returns the tight bounds as a 2-tuple of LPoint3 objects.  This is a
 * convenience function for Python users, among which the use of
 * calc_tight_bounds may be confusing.
 *
 * Returns None if calc_tight_bounds returned false.
 */
PyObject *Extension<NodePath>::
get_tight_bounds(const NodePath &other) const {
  LPoint3 *min_point = new LPoint3;
  LPoint3 *max_point = new LPoint3;

  if (_this->calc_tight_bounds(*min_point, *max_point, other)) {
#ifdef STDFLOAT_DOUBLE
    PyObject *min_inst = DTool_CreatePyInstance((void*) min_point, Dtool_LPoint3d, true, false);
    PyObject *max_inst = DTool_CreatePyInstance((void*) max_point, Dtool_LPoint3d, true, false);
#else
    PyObject *min_inst = DTool_CreatePyInstance((void*) min_point, Dtool_LPoint3f, true, false);
    PyObject *max_inst = DTool_CreatePyInstance((void*) max_point, Dtool_LPoint3f, true, false);
#endif
    return Py_BuildValue("NN", min_inst, max_inst);

  } else {
    Py_INCREF(Py_None);
    return Py_None;
  }
}

#endif  // HAVE_PYTHON
