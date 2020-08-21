/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file texture_ext.h
 * @author rdb
 * @date 2016-08-08
 */

#ifndef TEXTURE_EXT_H
#define TEXTURE_EXT_H

#ifndef CPPPARSER

#include "extension.h"
#include "py_panda.h"
#include "texture.h"

#ifdef HAVE_PYTHON

/**
 * This class defines the extension methods for Texture, which are
 * called instead of any C++ methods with the same prototype.
 */
template<>
class Extension<Texture> : public ExtensionBase<Texture> {
public:
  void set_ram_image(PyObject *image, Texture::CompressionMode compression = Texture::CM_off,
                     size_t page_size = 0);
  void set_ram_image_as(PyObject *image, const std::string &provided_format);
};

#endif  // HAVE_PYTHON

#endif  // CPPPARSER

#endif  // TEXTURE_EXT_H
