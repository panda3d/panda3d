// Filename: textureContext.cxx
// Created by:  drose (07Oct99)
// 
////////////////////////////////////////////////////////////////////

#include "textureContext.h"

#include <texture.h>
#include <pixelBuffer.h>

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
  PixelBuffer *pb = _texture->_pbuffer;

  size_t pixels = pb->get_xsize() * pb->get_ysize();

  size_t bpp = 1;
  switch (pb->get_format()) {
  case PixelBuffer::F_rgb332:
  case PixelBuffer::F_alpha:
  case PixelBuffer::F_red:
  case PixelBuffer::F_green:
  case PixelBuffer::F_blue:
  case PixelBuffer::F_luminance:
  case PixelBuffer::F_luminance_alpha:
  case PixelBuffer::F_luminance_alphamask:
  case PixelBuffer::F_color_index:
  case PixelBuffer::F_stencil_index:
  case PixelBuffer::F_depth_component:
    bpp = 1;
    break;

  case PixelBuffer::F_rgba:
  case PixelBuffer::F_rgba4:
  case PixelBuffer::F_rgbm:
  case PixelBuffer::F_rgb:
  case PixelBuffer::F_rgb5:
  case PixelBuffer::F_rgba5:
    bpp = 2;
    break;

  case PixelBuffer::F_rgb8:
  case PixelBuffer::F_rgba8:
    bpp = 4;
    break;

  case PixelBuffer::F_rgba12:
  case PixelBuffer::F_rgb12:
    bpp = 6;
    break;
  }

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
