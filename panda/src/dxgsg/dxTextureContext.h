// Filename: dxTextureContext.h
// Created by:  drose (07Oct99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef DXTEXTURECONTEXT_H
#define DXTEXTURECONTEXT_H

#include "dxgsgbase.h"

#include <texture.h>
#include <textureContext.h>

////////////////////////////////////////////////////////////////////
//   Class : DXTextureContext
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX DXTextureContext : public TextureContext {
  friend class DXGraphicsStateGuardian;
  friend class wdxGraphicsWindow;

public:
  DXTextureContext(Texture *tex);
  ~DXTextureContext();

  LPDIRECTDRAWSURFACE7  _surface;
  Texture *_tex;            // ptr to parent, primarily for access to namestr

//  static is_unused_texpixelformat(DDPIXELFORMAT *)

#ifdef USE_TEXFMTVEC
  LPDIRECTDRAWSURFACE7 CreateTexture(LPDIRECT3DDEVICE7 pd3dDevice, DDPixelFormatVec &TexFmts,LPD3DDEVICEDESC7 pD3DDevDesc);
#else
  LPDIRECTDRAWSURFACE7 CreateTexture(LPDIRECT3DDEVICE7 pd3dDevice, int cNumTexPixFmts, DDPIXELFORMAT *pTexFmts,LPD3DDEVICEDESC7 pD3DDevDesc);
#endif

  bool _bHasMipMaps;
  DWORD _PixBufConversionType;  // enum ConversionType

  // must be public since called from global callback fns
  void DeleteTexture(void);
  HRESULT FillDDSurfTexturePixels(void);

protected:
    unsigned int get_bits_per_pixel(PixelBuffer::Format format, int *alphbits);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TextureContext::init_type();
    register_type(_type_handle, "DXTextureContext",
          TextureContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};


#endif

