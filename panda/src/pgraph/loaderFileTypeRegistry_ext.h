/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file loaderFileTypeRegistry_ext.h
 * @author rdb
 * @date 2019-07-30
 */

#ifndef LOADERFILETYPEREGISTRYEXT_H
#define LOADERFILETYPEREGISTRYEXT_H

#include "pandabase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "loaderFileTypeRegistry.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for LoaderFileTypeRegistry, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<LoaderFileTypeRegistry> : public ExtensionBase<LoaderFileTypeRegistry> {
public:
  void register_type(PyObject *type);
  void register_deferred_type(PyObject *entry_point);

  void unregister_type(PyObject *type);
};

#endif  // HAVE_PYTHON

#endif
