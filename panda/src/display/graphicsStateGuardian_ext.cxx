/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsStateGuardian_ext.cxx
 * @author rdb
 * @date 2013-12-10
 */

#include "graphicsStateGuardian_ext.h"
#include "textureContext.h"

#ifdef HAVE_PYTHON

#ifndef CPPPARSER
extern struct Dtool_PyTypedObject Dtool_Texture;
#endif

static bool traverse_callback(TextureContext *tc, void *data) {
  PT(Texture) tex = tc->get_texture();
  PyObject *element =
    DTool_CreatePyInstanceTyped(tex, Dtool_Texture,
                                true, false, tex->get_type_index());
  tex->ref();

  PyObject *list = (PyObject *) data;
  PyList_Append(list, element);

  return true;
}

/**
 * Returns a Python list of all of the currently-prepared textures within the
 * GSG.
 */
PyObject *Extension<GraphicsStateGuardian>::
get_prepared_textures() const {
  PyObject *list = PyList_New(0);

  if (list == nullptr) {
    return nullptr;
  }

  _this->traverse_prepared_textures(&traverse_callback, (void *)list);
  return list;
}

#endif
