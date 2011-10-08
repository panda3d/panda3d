// Filename: glTextureContext_src.h
// Created by:  drose (07Oct99)
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

#include "pandabase.h"
#include "textureContext.h"
#include "deletedChain.h"

////////////////////////////////////////////////////////////////////
//       Class : GLTextureContext
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_GL CLP(TextureContext) : public TextureContext {
public:
  INLINE CLP(TextureContext)(PreparedGraphicsObjects *pgo, Texture *tex, int view);
  ALLOC_DELETED_CHAIN(CLP(TextureContext));

  virtual void evict_lru();
  void reset_data();

  // This is the GL "name" of the texture object.
  GLuint _index;

  // These are the parameters that we specified with the last
  // glTexImage2D() call.  If none of these have changed, we can
  // reload the texture image with a glTexSubImage2D().
  bool _already_applied;
  bool _uses_mipmaps;
  GLint _internal_format;
  GLsizei _width;
  GLsizei _height;
  GLsizei _depth;
  GLenum _target;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TextureContext::init_type();
    register_type(_type_handle, CLASSPREFIX_QUOTED "TextureContext",
                  TextureContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "glTextureContext_src.I"

