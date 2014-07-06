// Filename: eggGroupNode_ext.h
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

#ifndef EGGGROUPNODE_EXT_H
#define EGGGROUPNODE_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "eggGroupNode.h"
#include "py_panda.h"

////////////////////////////////////////////////////////////////////
//       Class : Extension<EggGroupNode>
// Description : This class defines the extension methods for
//               EggGroupNode, which are called instead of
//               any C++ methods with the same prototype.
////////////////////////////////////////////////////////////////////
template<>
class Extension<EggGroupNode> : public ExtensionBase<EggGroupNode> {
public:
  PyObject *get_children() const;
};

#endif  // HAVE_PYTHON

#endif  // EGGGROUPNODE_EXT_H
