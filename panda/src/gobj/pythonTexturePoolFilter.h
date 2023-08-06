/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pythonTexturePoolFilter.h
 * @author Derzsi Daniel
 * @date 2020-06-14
 */

#ifndef PYTHONTEXTUREPOOLFILTER_H
#define PYTHONTEXTUREPOOLFILTER_H

#include "filename.h"
#include "loaderOptions.h"
#include "texture.h"
#include "texturePool.h"
#include "texturePoolFilter.h"
#include "pandabase.h"

#ifdef HAVE_PYTHON

/**
 * This defines a Python-based texture pool filter plug-in. Instances of
 * this must be constructed by inheritance and explicitly registered.
 *
 * The Python texture pool filter must implement either the pre_load
 * or the post_load function of a typical texture pool filter.
 *
 * @since 1.11.0
 */
class PythonTexturePoolFilter : public TexturePoolFilter {
public:
  PythonTexturePoolFilter();
  ~PythonTexturePoolFilter();

  bool init(PyObject *tex_filter);

  virtual PT(Texture) pre_load(const Filename &orig_filename,
                               const Filename &orig_alpha_filename,
                               int primary_file_num_channels,
                               int alpha_file_channel,
                               bool read_mipmaps,
                               const LoaderOptions &options) override;
  virtual PT(Texture) post_load(Texture *tex) override;

private:
  PyObject *_pre_load_func = nullptr;
  PyObject *_post_load_func = nullptr;
  PyObject *_entry_point = nullptr;

  friend class Extension<TexturePool>;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TexturePoolFilter::init_type();
    register_type(_type_handle, "PythonTexturePoolFilter",
                  TexturePoolFilter::get_class_type());
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() override {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#endif  // HAVE_PYTHON

#endif
