/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsPipeSelection_ext.h
 * @author rdb
 * @date 2021-12-10
 */

#ifndef GRAPHICSPIPESELECTION_EXT_H
#define GRAPHICSPIPESELECTION_EXT_H

#include "pandabase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "graphicsPipeSelection.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for GraphicsPipeSelection, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<GraphicsPipeSelection> : public ExtensionBase<GraphicsPipeSelection> {
public:
  PyObject *__reduce__() const;
};

#endif  // HAVE_PYTHON

#endif  // GRAPHICSPIPESELECTION_EXT_H
