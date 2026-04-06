/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pandaNode_ext.h
 * @author CFSworks
 * @date 2014-03-30
 */

#ifndef PANDANODE_EXT_H
#define PANDANODE_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "pandaNode.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for PandaNode, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<PandaNode> : public ExtensionBase<PandaNode> {
public:
  PT(PandaNode) __copy__() const;
  PyObject *__deepcopy__(PyObject *self, PyObject *memo) const;

  PyObject *get_tag_keys() const;

  PyObject *get_python_tags();
  int set_python_tag(PyObject *keys, PyObject *value);
  PyObject *get_python_tag(PyObject *keys) const;
  bool has_python_tag(PyObject *keys) const;
  void clear_python_tag(PyObject *keys);
  PyObject *get_python_tag_keys() const;

  // This is defined to implement cycle detection in Python tags.
  int __traverse__(visitproc visit, void *arg);

private:
  PyObject *do_get_python_tags();

  // This is what actually stores the Python tags.
  class PythonTagDataImpl : public PandaNode::PythonTagData {
  public:
    PythonTagDataImpl() : _dict(PyDict_New()) {};
    PythonTagDataImpl(const PythonTagDataImpl &copy) : _dict(PyDict_Copy(copy._dict)) {};
    virtual ~PythonTagDataImpl();

    PyObject *_dict;
  };
};

#endif  // HAVE_PYTHON

#endif  // PANDANODE_EXT_H
