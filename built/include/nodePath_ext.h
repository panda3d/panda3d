/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nodePath_ext.h
 * @author rdb
 * @date 2013-12-09
 */

#ifndef NODEPATH_EXT_H
#define NODEPATH_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "nodePath.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for NodePath, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<NodePath> : public ExtensionBase<NodePath> {
public:
  NodePath __copy__() const;
  PyObject *__deepcopy__(PyObject *self, PyObject *memo) const;
  PyObject *__reduce__(PyObject *self) const;
  PyObject *__reduce_persist__(PyObject *self, PyObject *pickler) const;

  PyObject *get_tags() const;
  INLINE PyObject *get_tag_keys() const;

  INLINE PyObject *get_python_tags();
  INLINE void set_python_tag(PyObject *key, PyObject *value);
  INLINE PyObject *get_python_tag(PyObject *key) const;
  INLINE PyObject *get_python_tag_keys() const;
  INLINE bool has_python_tag(PyObject *key) const;
  INLINE void clear_python_tag(PyObject *key);
  INLINE PyObject *get_net_python_tag(PyObject *key) const;
  INLINE bool has_net_python_tag(PyObject *key) const;
  NodePath find_net_python_tag(PyObject *key) const;

  // This is defined to implement cycle detection in Python tags.
  //INLINE int __traverse__(visitproc visit, void *arg);

  void set_shader_input(CPT_InternalName id, PyObject *value, int priority=0);
  void set_shader_inputs(PyObject *args, PyObject *kwargs);

  PyObject *get_tight_bounds(const NodePath &other = NodePath()) const;

  void set_collide_owner(PyObject *owner);

private:
  static void r_set_collide_owner(PandaNode *node, PyObject *weakref);
  static void r_clear_collide_owner(PandaNode *node);
};

BEGIN_PUBLISH
NodePath py_decode_NodePath_from_bam_stream(vector_uchar data);
NodePath py_decode_NodePath_from_bam_stream_persist(PyObject *unpickler, vector_uchar data);
END_PUBLISH

#include "nodePath_ext.I"

#endif  // HAVE_PYTHON

#endif  // NODEPATH_EXT_H
