// Filename: pandaNode_ext.h
// Created by:  CFSworks (30Mar14)
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

#ifndef PANDANODE_EXT_H
#define PANDANODE_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "pandaNode.h"
#include "py_panda.h"

////////////////////////////////////////////////////////////////////
//       Class : Extension<PandaNode>
// Description : This class defines the extension methods for
//               PandaNode, which are called instead of
//               any C++ methods with the same prototype.
////////////////////////////////////////////////////////////////////
template<>
class Extension<PandaNode> : public ExtensionBase<PandaNode> {
public:
  PT(PandaNode) __copy__() const;
  PyObject *__deepcopy__(PyObject *self, PyObject *memo) const;

  PyObject *get_tag_keys() const;

  void set_python_tag(const string &key, PyObject *value);
  PyObject *get_python_tag(const string &key) const;
  bool has_python_tag(const string &key) const;
  void clear_python_tag(const string &key);
  void get_python_tag_keys(vector_string &keys) const;
  PyObject *get_python_tag_keys() const;
};

#endif  // HAVE_PYTHON

#endif  // PANDANODE_EXT_H
