// Filename: nodePathCollection_ext.h
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

#ifndef NODEPATHCOLLECTION_EXT_H
#define NODEPATHCOLLECTION_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "nodePathCollection.h"
#include "py_panda.h"

////////////////////////////////////////////////////////////////////
//       Class : Extension<NodePathCollection>
// Description : This class defines the extension methods for
//               NodePathCollection, which are called instead of
//               any C++ methods with the same prototype.
////////////////////////////////////////////////////////////////////
template<>
class Extension<NodePathCollection> : public ExtensionBase<NodePathCollection> {
public:
  void __init__(PyObject *self, PyObject *sequence);

  PyObject *__reduce__(PyObject *self) const;

  PyObject *get_tight_bounds() const;
};

#endif  // HAVE_PYTHON

#endif  // NODEPATHCOLLECTION_EXT_H
