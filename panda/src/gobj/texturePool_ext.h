/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file texturePool_ext.h
 * @author Derzsi Daniel
 * @date 2020-06-24
 */

#ifndef TEXTUREPOOL_EXT_H
#define TEXTUREPOOL_EXT_H

#include "pandabase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "texturePoolFilter.h"
#include "texturePool.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for TexturePool, which are called
 * instead of any C++ methods with the same prototype.
 *
 * @since 1.11.0
 */
template<>
class Extension<TexturePool> : public ExtensionBase<TexturePool> {
public:
  bool register_filter(PyObject *tex_filter);
  bool unregister_filter(PyObject *tex_filter);
  bool is_filter_registered(PyObject *tex_filter);

private:
  TexturePoolFilter *find_existing_filter(PyObject *tex_filter);
};

#endif  // HAVE_PYTHON

#endif
