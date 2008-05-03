// Filename: tinyTextureContext.h
// Created by:  drose (30Apr08)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef TINYTEXTURECONTEXT_H
#define TINYTEXTURECONTEXT_H

#include "pandabase.h"
#include "textureContext.h"
#include "deletedChain.h"
#include "tinygl.h"

struct GLTexture;

////////////////////////////////////////////////////////////////////
//       Class : TinyTextureContext
// Description :
////////////////////////////////////////////////////////////////////
class TinyTextureContext : public TextureContext {
public:
  INLINE TinyTextureContext(PreparedGraphicsObjects *pgo, Texture *tex);
  ALLOC_DELETED_CHAIN(TinyTextureContext);

  INLINE ~TinyTextureContext();

  GLTexture *_gltex;

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
