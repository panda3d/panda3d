// Filename: dxTextureContext9.h
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

#ifndef DXTEXTURECONTEXT9_H
#define DXTEXTURECONTEXT9_H

#include "dxgsg9base.h"
#include "texture.h"
#include "textureContext.h"

////////////////////////////////////////////////////////////////////
//       Class : DXTextureContext9
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX DXTextureContext9 : public TextureContext {
public:
  DXTextureContext9(Texture *tex);
  virtual ~DXTextureContext9();

  bool create_texture(DXScreenData &scrn);
  void delete_texture();

  INLINE bool has_mipmaps() const;
  INLINE IDirect3DBaseTexture9 *get_d3d_texture() const;
  INLINE IDirect3DTexture9 *get_d3d_2d_texture() const;
  INLINE IDirect3DVolumeTexture9 *get_d3d_volume_texture() const;
  INLINE IDirect3DCubeTexture9 *get_d3d_cube_texture() const;

  static HRESULT d3d_surface_to_texture(RECT &source_rect,
          IDirect3DSurface9 *d3d_surface,
          bool inverted, Texture *result,
          int z);

private:
  HRESULT fill_d3d_texture_pixels();
  HRESULT fill_d3d_volume_texture_pixels();
  static int down_to_power_2(int value);
  unsigned int get_bits_per_pixel(Texture::Format format, int *alphbits);

private:
  D3DFORMAT _d3d_format;    // the 'D3DFORMAT' the Panda TextureBuffer fmt corresponds to
  IDirect3DBaseTexture9 *_d3d_texture;
  IDirect3DTexture9 *_d3d_2d_texture;
  IDirect3DVolumeTexture9 *_d3d_volume_texture;
  IDirect3DCubeTexture9 *_d3d_cube_texture;

  bool _has_mipmaps;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TextureContext::init_type();
    register_type(_type_handle, "DXTextureContext9",
          TextureContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "dxTextureContext9.I"

#endif
