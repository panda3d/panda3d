// Filename: textureContext.cxx
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

#include "textureContext.h"

#include "texture.h"

TypeHandle TextureContext::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TextureContext::estimate_texture_memory
//       Access: Public, Virtual
//  Description: Estimates the amount of texture memory that will be
//               consumed by loading this texture.  This is mainly
//               useful for debugging and reporting purposes.
//
//               Returns a value in bytes.
////////////////////////////////////////////////////////////////////
size_t TextureContext::
estimate_texture_memory() {
  size_t pixels = _texture->get_x_size() * _texture->get_y_size();

  size_t bpp = 4;

  /*
  size_t bpp = 1;
  switch (_texture->get_format()) {
  case Texture::F_rgb332:
  case Texture::F_alpha:
  case Texture::F_red:
  case Texture::F_green:
  case Texture::F_blue:
  case Texture::F_luminance:
  case Texture::F_luminance_alpha:
  case Texture::F_luminance_alphamask:
  case Texture::F_color_index:
  case Texture::F_stencil_index:
  case Texture::F_depth_component:
    bpp = 1;
    break;

  case Texture::F_rgba:
  case Texture::F_rgba4:
  case Texture::F_rgbm:
  case Texture::F_rgb:
  case Texture::F_rgb5:
  case Texture::F_rgba5:
    bpp = 2;
    break;

  case Texture::F_rgb8:
  case Texture::F_rgba8:
    bpp = 4;
    break;

  case Texture::F_rgba12:
  case Texture::F_rgb12:
    bpp = 6;
    break;
  }
  */

  size_t bytes = pixels * bpp;

  bool use_mipmaps;
  switch (_texture->get_minfilter()) {
  case Texture::FT_nearest_mipmap_nearest:
  case Texture::FT_linear_mipmap_nearest:
  case Texture::FT_nearest_mipmap_linear:
  case Texture::FT_linear_mipmap_linear:
    use_mipmaps = true;
    break;

  default:
    use_mipmaps = false;
    break;
  }

  if (use_mipmaps) {
    bytes += bytes / 3;
  }

  return bytes;
}
