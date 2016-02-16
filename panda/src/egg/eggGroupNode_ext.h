/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggGroupNode_ext.h
 * @author rdb
 * @date 2013-12-09
 */

#ifndef EGGGROUPNODE_EXT_H
#define EGGGROUPNODE_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "eggGroupNode.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for EggGroupNode, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<EggGroupNode> : public ExtensionBase<EggGroupNode> {
public:
  PyObject *get_children() const;
};

#endif  // HAVE_PYTHON

#endif  // EGGGROUPNODE_EXT_H
