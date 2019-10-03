/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glPixelPackBufferContext_src.h
 * @author rdb
 * @date 2019-10-02
 */

#include "pandabase.h"
#include "deletedChain.h"
#include "transferBufferContext.h"

#ifndef OPENGLES

/**
 * This class manages a series of texture extractions that are performed in a
 * single batch.
 */
class EXPCL_GL CLP(PixelPackBufferContext) final : public TransferBufferContext {
public:
  INLINE CLP(PixelPackBufferContext)(CLP(GraphicsStateGuardian) *glgsg);
  virtual ~CLP(PixelPackBufferContext)();

  virtual bool is_transfer_done() const override;
  virtual void finish_transfer() override;

  CLP(GraphicsStateGuardian) *const _glgsg;
  GLuint _index = 0;
  GLsync _sync = 0;

  struct ExtractTexture {
    PT(Texture) _texture;
    uintptr_t _offset = 0;
    size_t _page_size = 0;
    int _width, _height, _depth, _num_views;
    SamplerState _sampler;
    Texture::ComponentType _type = Texture::T_unsigned_byte;
    Texture::Format _format = Texture::F_rgb;
    Texture::CompressionMode _compression = Texture::CM_off;
  };
  pvector<ExtractTexture> _textures;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TransferBufferContext::init_type();
    register_type(_type_handle, CLASSPREFIX_QUOTED "PixelPackBufferContext",
                  TransferBufferContext::get_class_type());
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

#include "glPixelPackBufferContext_src.I"

#endif  // !OPENGLES
