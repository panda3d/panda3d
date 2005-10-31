// Filename: dxTextureContext7.h
// Created by:  drose (07Oct99)
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

#ifndef DXTEXTURECONTEXT7_H
#define DXTEXTURECONTEXT7_H

#include "dxgsg7base.h"

#include "texture.h"
#include "textureContext.h"

////////////////////////////////////////////////////////////////////
//   Class : DXTextureContext7
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX DXTextureContext7 : public TextureContext {
  friend class DXGraphicsStateGuardian7;
  friend class wdxGraphicsWindow7;

public:
  DXTextureContext7(Texture *tex);
  ~DXTextureContext7();

  LPDIRECTDRAWSURFACE7  _surface;
  Texture *_tex;            // ptr to parent, primarily for access to namestr

//  static is_unused_texpixelformat(DDPIXELFORMAT *)

  LPDIRECTDRAWSURFACE7 create_texture(LPDIRECT3DDEVICE7 pd3dDevice, int cNumTexPixFmts, DDPIXELFORMAT *pTexFmts,LPD3DDEVICEDESC7 pD3DDevDesc);
  void delete_texture();

  INLINE bool has_mipmaps() const;

  bool _has_mipmaps;
  DWORD _PixBufConversionType;  // enum ConversionType

  HRESULT FillDDSurfTexturePixels();

protected:
    unsigned int get_bits_per_pixel(Texture::Format format, int *alphbits);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TextureContext::init_type();
    register_type(_type_handle, "DXTextureContext7",
          TextureContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "dxTextureContext7.I"

#endif

