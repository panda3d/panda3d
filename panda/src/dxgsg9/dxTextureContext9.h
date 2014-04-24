// Filename: dxTextureContext9.h
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

#ifndef DXTEXTURECONTEXT9_H
#define DXTEXTURECONTEXT9_H

#include "dxgsg9base.h"
#include "texture.h"
#include "textureContext.h"

#include "lru.h"

////////////////////////////////////////////////////////////////////
//       Class : DXTextureContext9
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX DXTextureContext9 : public TextureContext {
public:
  DXTextureContext9(PreparedGraphicsObjects *pgo, Texture *tex, int view);
  virtual ~DXTextureContext9();

  virtual void evict_lru();

  bool create_texture(DXScreenData &scrn);
  bool create_simple_texture(DXScreenData &scrn);
  void delete_texture();
  bool extract_texture_data(DXScreenData &scrn);

  INLINE bool has_mipmaps() const;
  INLINE IDirect3DBaseTexture9 *get_d3d_texture() const;
  INLINE IDirect3DTexture9 *get_d3d_2d_texture() const;
  INLINE IDirect3DVolumeTexture9 *get_d3d_volume_texture() const;
  INLINE IDirect3DCubeTexture9 *get_d3d_cube_texture() const;

  static HRESULT d3d_surface_to_texture(RECT &source_rect,
          IDirect3DSurface9 *d3d_surface,
          bool inverted, Texture *result,
          int view, int z);

private:
  HRESULT fill_d3d_texture_mipmap_pixels(int mip_level, int depth_index, D3DFORMAT source_format);
  HRESULT fill_d3d_texture_pixels(DXScreenData &scrn, bool compress_texture);
  HRESULT fill_d3d_volume_texture_pixels(DXScreenData &scrn);
  static int down_to_power_2(int value);
  unsigned int get_bits_per_pixel(Texture::Format format, int *alphbits);
  PN_stdfloat d3d_format_to_bytes_per_pixel (D3DFORMAT format);

private:
  D3DFORMAT _d3d_format;    // the 'D3DFORMAT' the Panda TextureBuffer fmt corresponds to
  IDirect3DBaseTexture9 *_d3d_texture;
  IDirect3DTexture9 *_d3d_2d_texture;
  IDirect3DVolumeTexture9 *_d3d_volume_texture;
  IDirect3DCubeTexture9 *_d3d_cube_texture;

  int _managed;

private:
  bool _has_mipmaps;
  bool _is_render_target;
  
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

  friend class wdxGraphicsBuffer9;
};

#include "dxTextureContext9.I"

#endif
