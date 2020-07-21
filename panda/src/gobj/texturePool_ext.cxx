/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file texturePool_ext.cxx
 * @author Derzsi Daniel
 * @date 2020-06-24
 */

#include "texturePool_ext.h"

#ifdef HAVE_PYTHON

#include "pythonTexturePoolFilter.h"
#include "texturePoolFilter.h"

/**
 * Registers a texture pool filter that is implemented in Python.
 */
bool Extension<TexturePool>::
register_filter(PyObject *tex_filter) {
  if (find_existing_filter(tex_filter) != nullptr) {
    PyObject *repr = PyObject_Repr(tex_filter);

    gobj_cat->warning()
      << "Attempted to register Python texture filter " << PyUnicode_AsUTF8(repr)
      << " more than once.\n";
    return false;
  }

  PythonTexturePoolFilter *filter = new PythonTexturePoolFilter();

  if (filter->init(tex_filter)) {
    return _this->ns_register_filter(filter);
  }

  delete filter;
  return false;
}

/**
 * If the given Python texture pool filter is registered, unregisters it.
 */
bool Extension<TexturePool>::
unregister_filter(PyObject *tex_filter) {
  // Keep looping until we've removed all instances of it.
  bool unregistered = false;

  while (true) {
    TexturePoolFilter *filter = find_existing_filter(tex_filter);

    if (filter == nullptr) {
      // Last filter has been removed.
      break;
    }

    unregistered = _this->ns_unregister_filter(filter);
  }

  if (!unregistered) {
    PyObject *repr = PyObject_Repr(tex_filter);

    gobj_cat->warning()
      << "Attempted to unregister Python texture filter " << PyUnicode_AsUTF8(repr)
      << " which was not registered.\n";
  }

  return unregistered;
}

/**
 * If the given Python texture pool filter is registered, returns true.
 */
bool Extension<TexturePool>::
is_filter_registered(PyObject *tex_filter) {
  return find_existing_filter(tex_filter) != nullptr;
}

/**
 * Looks for a texture pool filter that is using the
 * given Python implementation of the texture filter.
 *
 * Returns nullptr if none has been found.
 */
TexturePoolFilter *Extension<TexturePool>::
find_existing_filter(PyObject *tex_filter) {
  size_t num_filters = _this->get_num_filters();

  for (size_t i = 0; i < num_filters; ++i) {
    TexturePoolFilter *filter = _this->get_filter(i);

    if (filter != nullptr && filter->is_of_type(PythonTexturePoolFilter::get_class_type())) {
      PythonTexturePoolFilter *py_filter = (PythonTexturePoolFilter *)filter;

      if (py_filter->_entry_point == tex_filter) {
        return filter;
      }
    }
  }

  // No filter found.
  return nullptr;
}

#endif
