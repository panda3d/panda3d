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

#include "dxgsg8base.h"
#include <texture.h>
#include <textureContext.h>

//#define DO_CUSTOM_CONVERSIONS

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

  IDirect3DTexture8  *_pD3DTexture8;
  Texture *_tex;            // ptr to parent, primarily for access to namestr
  IDirect3DTexture8 *CreateTexture(DXScreenData &scrn);

  D3DFORMAT _PixBufD3DFmt;    // the 'D3DFORMAT' the Panda PixelBuffer fmt corresponds to

  bool _bHasMipMaps;

#ifdef DO_CUSTOM_CONVERSIONS
  DWORD _PixBufConversionType;  // enum ConversionType
#endif

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

extern HRESULT ConvertD3DSurftoPixBuf(RECT &SrcRect,IDirect3DSurface8 *pD3DSurf8,PixelBuffer *pixbuf);

#endif

