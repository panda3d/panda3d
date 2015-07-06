// Filename: nodePath_ext.h
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

#ifndef NODEPATH_EXT_H
#define NODEPATH_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "nodePath.h"
#include "py_panda.h"

////////////////////////////////////////////////////////////////////
//       Class : Extension<NodePath>
// Description : This class defines the extension methods for
//               NodePath, which are called instead of
//               any C++ methods with the same prototype.
////////////////////////////////////////////////////////////////////
template<>
class Extension<NodePath> : public ExtensionBase<NodePath> {
public:
  NodePath __copy__() const;
  PyObject *__deepcopy__(PyObject *self, PyObject *memo) const;
  PyObject *__reduce__(PyObject *self) const;
  PyObject *__reduce_persist__(PyObject *self, PyObject *pickler) const;

  INLINE PyObject *get_tag_keys() const;
  INLINE void set_python_tag(const string &key, PyObject *value);
  INLINE PyObject *get_python_tag(const string &key) const;
  INLINE void get_python_tag_keys(vector_string &keys) const;
  INLINE PyObject *get_python_tag_keys() const;
  INLINE bool has_python_tag(const string &key) const;
  INLINE void clear_python_tag(const string &key);
  INLINE PyObject *get_net_python_tag(const string &key) const;
  INLINE bool has_net_python_tag(const string &key) const;
  NodePath find_net_python_tag(const string &key) const;

  PyObject *get_tight_bounds(const NodePath &other = NodePath()) const;
};

BEGIN_PUBLISH
NodePath py_decode_NodePath_from_bam_stream(const string &data);
NodePath py_decode_NodePath_from_bam_stream_persist(PyObject *unpickler, const string &data);
END_PUBLISH

#include "nodePath_ext.I"

#endif  // HAVE_PYTHON

#endif  // NODEPATH_EXT_H
