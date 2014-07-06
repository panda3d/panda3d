// Filename: nodePath_ext.cxx
// Created by:  rdb (09Dec13)
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

#include "nodePath_ext.h"
#include "typedWritable_ext.h"

#ifdef HAVE_PYTHON

#ifndef CPPPARSER
extern EXPCL_PANDA_PUTIL Dtool_PyTypedObject Dtool_BamWriter;
extern EXPCL_PANDA_PUTIL Dtool_PyTypedObject Dtool_BamReader;
#endif  // CPPPARSER

////////////////////////////////////////////////////////////////////
//     Function: Extension<NodePath>::__copy__
//       Access: Published
//  Description: A special Python method that is invoked by
//               copy.copy(node).  Unlike the NodePath copy
//               constructor, this makes a duplicate copy of the
//               underlying PandaNode (but shares children, instead of
//               copying them or omitting them).
////////////////////////////////////////////////////////////////////
NodePath Extension<NodePath>::
__copy__() const {
  if (_this->is_empty()) {
    // Invoke the copy constructor if we have no node.
    return *_this;
  }

  // If we do have a node, duplicate it, and wrap it in a new
  // NodePath.
  return NodePath(invoke_extension(_this->node()).__copy__());
}

////////////////////////////////////////////////////////////////////
//     Function: Extension<NodePath>::__deepcopy__
//       Access: Published
//  Description: A special Python method that is invoked by
//               copy.deepcopy(np).  This calls copy_to() unless the
//               NodePath is already present in the provided
//               dictionary.
////////////////////////////////////////////////////////////////////
PyObject *Extension<NodePath>::
__deepcopy__(PyObject *self, PyObject *memo) const {
  IMPORT_THIS struct Dtool_PyTypedObject Dtool_NodePath;

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

////////////////////////////////////////////////////////////////////
//     Function: Extension<NodePath>::__reduce__
//       Access: Published
//  Description: This special Python method is implement to provide
//               support for the pickle module.
//
//               This hooks into the native pickle and cPickle
//               modules, but it cannot properly handle
//               self-referential BAM objects.
////////////////////////////////////////////////////////////////////
PyObject *Extension<NodePath>::
__reduce__(PyObject *self) const {
  return __reduce_persist__(self, NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: Extension<NodePath>::__reduce_persist__
//       Access: Published
//  Description: This special Python method is implement to provide
//               support for the pickle module.
//
//               This is similar to __reduce__, but it provides
//               additional support for the missing persistent-state
//               object needed to properly support self-referential
//               BAM objects written to the pickle stream.  This hooks
//               into the pickle and cPickle modules implemented in
//               direct/src/stdpy.
////////////////////////////////////////////////////////////////////
PyObject *Extension<NodePath>::
__reduce_persist__(PyObject *self, PyObject *pickler) const {
  // We should return at least a 2-tuple, (Class, (args)): the
  // necessary class object whose constructor we should call
  // (e.g. this), and the arguments necessary to reconstruct this
  // object.

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
    // The modified pickle support: call the "persistent" version of
    // this function, which receives the unpickler itself as an
    // additional parameter.
    func = Extension<TypedWritable>::find_global_decode(this_class, "py_decode_NodePath_from_bam_stream_persist");
    if (func == NULL) {
      PyErr_SetString(PyExc_TypeError, "Couldn't find py_decode_NodePath_from_bam_stream_persist()");
      Py_DECREF(this_class);
      return NULL;
    }

  } else {
    // The traditional pickle support: call the non-persistent version
    // of this function.

    func = Extension<TypedWritable>::find_global_decode(this_class, "py_decode_NodePath_from_bam_stream");
    if (func == NULL) {
      PyErr_SetString(PyExc_TypeError, "Couldn't find py_decode_NodePath_from_bam_stream()");
      Py_DECREF(this_class);
      return NULL;
    }
  }

  PyObject *result = Py_BuildValue("(O(s#))", func, bam_stream.data(), bam_stream.size());
  Py_DECREF(func);
  Py_DECREF(this_class);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: Extension<NodePath>::find_net_python_tag
//       Access: Published
//  Description: Returns the lowest ancestor of this node that
//               contains a tag definition with the indicated key, if
//               any, or an empty NodePath if no ancestor of this node
//               contains this tag definition.  See set_python_tag().
////////////////////////////////////////////////////////////////////
NodePath Extension<NodePath>::
find_net_python_tag(const string &key) const {
  if (_this->is_empty()) {
    return NodePath::not_found();
  }
  if (has_python_tag(key)) {
    return *_this;
  }
  NodePath parent = _this->get_parent();
  return invoke_extension(&parent).find_net_python_tag(key);
}

////////////////////////////////////////////////////////////////////
//     Function: py_decode_NodePath_from_bam_stream
//       Access: Published
//  Description: This wrapper is defined as a global function to suit
//               pickle's needs.
////////////////////////////////////////////////////////////////////
NodePath
py_decode_NodePath_from_bam_stream(const string &data) {
  return py_decode_NodePath_from_bam_stream_persist(NULL, data);
}

////////////////////////////////////////////////////////////////////
//     Function: py_decode_NodePath_from_bam_stream_persist
//       Access: Published
//  Description: This wrapper is defined as a global function to suit
//               pickle's needs.
////////////////////////////////////////////////////////////////////
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

#endif  // HAVE_PYTHON

