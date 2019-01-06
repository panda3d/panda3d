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
#include "shaderInput_ext.h"
#include "shaderAttrib.h"

using std::move;

#ifdef HAVE_PYTHON

#ifndef CPPPARSER
extern struct Dtool_PyTypedObject Dtool_BamWriter;
extern struct Dtool_PyTypedObject Dtool_BamReader;
#ifdef STDFLOAT_DOUBLE
extern struct Dtool_PyTypedObject Dtool_LPoint3d;
#else
extern struct Dtool_PyTypedObject Dtool_LPoint3f;
#endif
extern struct Dtool_PyTypedObject Dtool_NodePath;
extern struct Dtool_PyTypedObject Dtool_PandaNode;
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
  if (dupe != nullptr) {
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
    return nullptr;
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
  return __reduce_persist__(self, nullptr);
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

  BamWriter *writer = nullptr;
  if (pickler != nullptr) {
    PyObject *py_writer = PyObject_GetAttrString(pickler, "bamWriter");
    if (py_writer == nullptr) {
      // It's OK if there's no bamWriter.
      PyErr_Clear();
    } else {
      DtoolInstance_GetPointer(py_writer, writer, Dtool_BamWriter);
      Py_DECREF(py_writer);
    }
  }

  // We have a non-empty NodePath.

  vector_uchar bam_stream;
  if (!_this->encode_to_bam_stream(bam_stream, writer)) {
    std::ostringstream stream;
    stream << "Could not bamify " << _this;
    std::string message = stream.str();
    PyErr_SetString(PyExc_TypeError, message.c_str());
    return nullptr;
  }

  // Start by getting this class object.
  PyObject *this_class = (PyObject *)Py_TYPE(self);
  if (this_class == nullptr) {
    return nullptr;
  }

  PyObject *func;
  if (writer != nullptr) {
    // The modified pickle support: call the "persistent" version of this
    // function, which receives the unpickler itself as an additional
    // parameter.
    func = Extension<TypedWritable>::find_global_decode(this_class, "py_decode_NodePath_from_bam_stream_persist");
    if (func == nullptr) {
      PyErr_SetString(PyExc_TypeError, "Couldn't find py_decode_NodePath_from_bam_stream_persist()");
      return nullptr;
    }

  } else {
    // The traditional pickle support: call the non-persistent version of this
    // function.
    func = Extension<TypedWritable>::find_global_decode(this_class, "py_decode_NodePath_from_bam_stream");
    if (func == nullptr) {
      PyErr_SetString(PyExc_TypeError, "Couldn't find py_decode_NodePath_from_bam_stream()");
      return nullptr;
    }
  }

  // PyTuple_SET_ITEM conveniently borrows the reference it is passed.
  PyObject *args = PyTuple_New(1);
  PyTuple_SET_ITEM(args, 0, Dtool_WrapValue(bam_stream));

  PyObject *tuple = PyTuple_New(2);
  PyTuple_SET_ITEM(tuple, 0, func);
  PyTuple_SET_ITEM(tuple, 1, args);
  return tuple;
}

/**
 * Returns the associated node's tags.
 */
PyObject *Extension<NodePath>::
get_tags() const {
  // An empty NodePath returns None
  if (_this->is_empty()) {
    Py_INCREF(Py_None);
    return Py_None;
  }

  // Just call PandaNode.tags rather than defining a whole new interface.
  PT(PandaNode) node = _this->node();
  PyObject *py_node = DTool_CreatePyInstanceTyped
    ((void *)node.p(), Dtool_PandaNode, true, false, node->get_type_index());

  // DTool_CreatePyInstanceTyped() steals a C++ reference.
  node.cheat() = nullptr;

  PyObject *result = PyObject_GetAttrString(py_node, "tags");
  Py_DECREF(py_node);
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
py_decode_NodePath_from_bam_stream(vector_uchar data) {
  return py_decode_NodePath_from_bam_stream_persist(nullptr, move(data));
}

/**
 * This wrapper is defined as a global function to suit pickle's needs.
 */
NodePath
py_decode_NodePath_from_bam_stream_persist(PyObject *unpickler, vector_uchar data) {
  BamReader *reader = nullptr;
  if (unpickler != nullptr) {
    PyObject *py_reader = PyObject_GetAttrString(unpickler, "bamReader");
    if (py_reader == nullptr) {
      // It's OK if there's no bamReader.
      PyErr_Clear();
    } else {
      DtoolInstance_GetPointer(py_reader, reader, Dtool_BamReader);
      Py_DECREF(py_reader);
    }
  }

  return NodePath::decode_from_bam_stream(move(data), reader);
}

/**
 * Sets a single shader input.
 */
void Extension<NodePath>::
set_shader_input(CPT_InternalName name, PyObject *value, int priority) {
  PT(PandaNode) node = _this->node();
  CPT(RenderAttrib) prev_attrib = node->get_attrib(ShaderAttrib::get_class_slot());
  PT(ShaderAttrib) attrib;
  if (prev_attrib == nullptr) {
    attrib = new ShaderAttrib();
  } else {
    attrib = new ShaderAttrib(*(const ShaderAttrib *)prev_attrib.p());
  }

  ShaderInput &input = attrib->_inputs[name];
  invoke_extension(&input).__init__(move(name), value, priority);

  if (!_PyErr_OCCURRED()) {
    node->set_attrib(ShaderAttrib::return_new(attrib));
  }
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

    CPT_InternalName name(std::string(buffer, length));
    ShaderInput &input = attrib->_inputs[name];
    invoke_extension(&input).__init__(move(name), value);
  }

  if (!_PyErr_OCCURRED()) {
    node->set_attrib(ShaderAttrib::return_new(attrib));
  }
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
