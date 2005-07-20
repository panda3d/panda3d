// Filename: dxTextureContext8.h
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

#ifndef DXTEXTURECONTEXT8_H
#define DXTEXTURECONTEXT8_H

#include "dxgsg8base.h"
#include "texture.h"
#include "textureContext.h"

////////////////////////////////////////////////////////////////////
//       Class : DXTextureContext8
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX DXTextureContext8 : public TextureContext {
public:
  DXTextureContext8(Texture *tex);
  virtual ~DXTextureContext8();

  bool create_texture(DXScreenData &scrn);
  void delete_texture();

  INLINE bool has_mipmaps() const;
  INLINE IDirect3DBaseTexture8 *get_d3d_texture() const;
  INLINE IDirect3DTexture8 *get_d3d_2d_texture() const;
  INLINE IDirect3DVolumeTexture8 *get_d3d_volume_texture() const;
  INLINE IDirect3DCubeTexture8 *get_d3d_cube_texture() const;

  static HRESULT d3d_surface_to_texture(RECT &source_rect, IDirect3DSurface8 *d3d_surface, Texture *result);

private:
  HRESULT fill_d3d_texture_pixels();
  HRESULT fill_d3d_volume_texture_pixels();
  static int down_to_power_2(int value);
  unsigned int get_bits_per_pixel(Texture::Format format, int *alphbits);

private:
  D3DFORMAT _d3d_format;    // the 'D3DFORMAT' the Panda TextureBuffer fmt corresponds to
  IDirect3DBaseTexture8 *_d3d_texture;
  IDirect3DTexture8 *_d3d_2d_texture;
  IDirect3DVolumeTexture8 *_d3d_volume_texture;
  IDirect3DCubeTexture8 *_d3d_cube_texture;

  bool _has_mipmaps;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TextureContext::init_type();
    register_type(_type_handle, "DXTextureContext8",
          TextureContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "dxTextureContext8.I"

#endif

