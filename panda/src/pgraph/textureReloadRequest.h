// Filename: textureReloadRequest.h
// Created by:  drose (12Aug08)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef TEXTURERELOADREQUEST
#define TEXTURERELOADREQUEST

#include "pandabase.h"

#include "asyncTask.h"
#include "texture.h"
#include "textureContext.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : TextureReloadRequest
// Description : This loader request will call
//               Texture::get_ram_image() in a sub-thread, to force
//               the texture's image to be re-read from disk.  It is
//               used by GraphicsStateGuardian::async_reload_texture(),
//               when get_incomplete_render() is true.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPH TextureReloadRequest : public AsyncTask {
public:
  ALLOC_DELETED_CHAIN(TextureReloadRequest);

PUBLISHED:
  INLINE TextureReloadRequest(TextureContext *tc);
  
  INLINE TextureContext *get_texture_context() const;
  INLINE bool is_ready() const;
  
protected:
  virtual bool do_task();
  
private:
  TextureContext *_texture_context;
  PT(Texture) _texture;
  bool _is_ready;
  
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
