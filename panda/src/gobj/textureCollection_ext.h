// Filename: textureCollection_ext.h
// Created by:  rdb (11Feb15)
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

#ifndef TEXTURECOLLECTION_EXT_H
#define TEXTURECOLLECTION_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "textureCollection.h"
#include "py_panda.h"

////////////////////////////////////////////////////////////////////
//       Class : Extension<TextureCollection>
// Description : This class defines the extension methods for
//               TextureCollection, which are called instead of
//               any C++ methods with the same prototype.
////////////////////////////////////////////////////////////////////
template<>
class Extension<TextureCollection> : public ExtensionBase<TextureCollection> {
public:
  void __init__(PyObject *self, PyObject *sequence);

  PyObject *__reduce__(PyObject *self) const;
};

#endif  // HAVE_PYTHON

#endif  // TEXTURECOLLECTION_EXT_H
