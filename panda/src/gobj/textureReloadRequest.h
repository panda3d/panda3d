/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textureReloadRequest.h
 * @author drose
 * @date 2008-08-12
 */

#ifndef TEXTURERELOADREQUEST
#define TEXTURERELOADREQUEST

#include "pandabase.h"

#include "asyncTask.h"
#include "texture.h"
#include "preparedGraphicsObjects.h"
#include "pointerTo.h"
#include "pmutex.h"

/**
 * This loader request will call Texture::get_ram_image() in a sub-thread, to
 * force the texture's image to be re-read from disk.  It is used by
 * GraphicsStateGuardian::async_reload_texture(), when get_incomplete_render()
 * is true.
 */
class EXPCL_PANDA_GOBJ TextureReloadRequest : public AsyncTask {
public:
  ALLOC_DELETED_CHAIN(TextureReloadRequest);

PUBLISHED:
  INLINE explicit TextureReloadRequest(const std::string &name,
                                       PreparedGraphicsObjects *pgo,
                                       Texture *texture,
                                       bool allow_compressed);

  INLINE PreparedGraphicsObjects *get_prepared_graphics_objects() const;
  INLINE Texture *get_texture() const;
  INLINE bool get_allow_compressed() const;
  INLINE bool is_ready() const;

  MAKE_PROPERTY(texture, get_texture);

protected:
  virtual DoneStatus do_task();

private:
  PT(PreparedGraphicsObjects) _pgo;
  PT(Texture) _texture;
  bool _allow_compressed;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AsyncTask::init_type();
    register_type(_type_handle, "TextureReloadRequest",
                  AsyncTask::get_class_type());
    }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "textureReloadRequest.I"

#endif
