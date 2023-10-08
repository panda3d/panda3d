/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file tinyTextureContext.h
 * @author drose
 * @date 2008-04-30
 */

#ifndef TINYTEXTURECONTEXT_H
#define TINYTEXTURECONTEXT_H

#include "pandabase.h"
#include "textureContext.h"
#include "deletedChain.h"
#include "zgl.h"

/**
 *
 */
class EXPCL_TINYDISPLAY TinyTextureContext : public TextureContext {
public:
  INLINE TinyTextureContext(PreparedGraphicsObjects *pgo, Texture *tex);
  ALLOC_DELETED_CHAIN(TinyTextureContext);

  ~TinyTextureContext();

  virtual void evict_lru();

  GLTexture _gltex;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TextureContext::init_type();
    register_type(_type_handle, "TinyTextureContext",
                  TextureContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "tinyTextureContext.I"

#endif
