/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggNode_ext.h
 * @author rdb
 * @date 2021-01-01
 */

#ifndef EGGNODE_EXT_H
#define EGGNODE_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "eggData.h"
#include "eggNode.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for EggNode, which are
 * called instead of any C++ methods with the same prototype.
 */
template<>
class Extension<EggNode> : public ExtensionBase<EggNode> {
public:
  PyObject *__reduce__() const;
};

BEGIN_PUBLISH
PT(EggData) parse_egg_data(const std::string &egg_syntax);
PT(EggNode) parse_egg_node(const std::string &egg_syntax);
END_PUBLISH

#endif  // HAVE_PYTHON

#endif  // EGGNODE_EXT_H
