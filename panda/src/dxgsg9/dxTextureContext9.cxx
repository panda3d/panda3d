/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxTextureContext9.cxx
 * @author georges
 * @date 2002-02-02
 */

#include "config_dxgsg9.h"
#include "dxGraphicsStateGuardian9.h"
#include "pStatTimer.h"
#include "dxTextureContext9.h"
#include "bamCache.h"
#include "graphicsEngine.h"
#include <d3dx9tex.h>
#include <assert.h>
#include <time.h>

#define DEBUG_SURFACES false
#define DEBUG_TEXTURES true

using std::endl;
using std::max;
using std::min;

TypeHandle DXTextureContext9::_type_handle;

static const DWORD g_LowByteMask = 0x000000FF;

/**
 *
 */
DXTextureContext9::
DXTextureContext9(PreparedGraphicsObjects *pgo, Texture *tex, int view) :
  TextureContext(pgo, tex, view) {

  if (dxgsg9_cat.is_spam()) {
    dxgsg9_cat.spam()
      << "Creating DX texture [" << tex->get_name() << "], minfilter(" << tex->get_minfilter() << "), magfilter(" << tex->get_magfilter() << "), anisodeg(" << tex->get_anisotropic_degree() << ")\n";
  }

  _d3d_texture = nullptr;
  _d3d_2d_texture = nullptr;
  _d3d_volume_texture = nullptr;
  _d3d_cube_texture = nullptr;
  _has_mipmaps = false;
  _is_render_target = false;
  _managed = -1;
}

/**
 *
 */
DXTextureContext9::
~DXTextureContext9() {
  delete_texture();
}

/**
 * Evicts the page from the LRU.  Called internally when the LRU determines
 * that it is full.  May also be called externally when necessary to
 * explicitly evict the page.
 *
 * It is legal for this method to either evict the page as requested, do
 * nothing (in which case the eviction will be requested again at the next
 * epoch), or requeue itself on the tail of the queue (in which case the
 * eviction will be requested again much later).
 */
void DXTextureContext9::
evict_lru() {
  if (get_texture()->get_render_to_texture()) {
    // Don't evict the result of render-to-texture.
    mark_used_lru();
    return;
  }
  if (dxgsg9_cat.is_debug()) {
    dxgsg9_cat.debug()
      << "Evicting " << *get_texture() << "\n";
  }

  dequeue_lru();
  delete_texture();

  update_data_size_bytes(0);
  mark_unloaded();
}

/**
 * Use panda texture's pixelbuffer to create a texture for the specified
 * device.  This code gets the attributes of the texture from the bitmap,
 * creates the texture, and then copies the bitmap into the texture.  The
 * return value is true if the texture is successfully created, false
 * otherwise.
 */
bool DXTextureContext9::
create_texture(DXScreenData &scrn) {

  // check if the texture has already been created
  if (_d3d_2d_texture || _d3d_cube_texture || _d3d_volume_texture) {
    // texture already created, no need to create
    return true;
  }

  HRESULT hr;
  int num_alpha_bits;     //  number of alpha bits in texture pixfmt
  D3DFORMAT target_pixel_format = D3DFMT_UNKNOWN;
  bool needs_luminance = false;
  bool needs_depth = false;
  bool needs_stencil = false;
  bool compress_texture = false;

  Texture *tex = get_texture();
  nassertr(IS_VALID_PTR(tex), false);

  // Ensure the ram image is loaded, if available.
  tex->get_ram_image();

  DWORD orig_width = (DWORD)tex->get_x_size();
  DWORD orig_height = (DWORD)tex->get_y_size();
  DWORD orig_depth = (DWORD)tex->get_z_size();

  // check for texture compression
  bool texture_wants_compressed = false;
  Texture::CompressionMode compression_mode = tex->get_ram_image_compression();
  bool texture_stored_compressed = compression_mode != Texture::CM_off;

  if (texture_stored_compressed) {
    texture_wants_compressed = true;
  } else {
    if (tex->get_compression() == Texture::CM_off) {
      // no compression
    } else {
      if (tex->get_compression() == Texture::CM_default) {
        // default = use "compressed-textures" config setting
        if (compressed_textures) {
          texture_wants_compressed = true;
        }
      } else {
        texture_wants_compressed = true;
      }
    }
  }

  switch (tex->get_texture_type()) {
    case Texture::TT_1d_texture:
    case Texture::TT_2d_texture:
    case Texture::TT_cube_map:
      // no compression for render target textures, or very small textures
      if (!tex->get_render_to_texture() &&
          orig_width >= 4 && orig_height >= 4) {
        if (texture_wants_compressed){
          compress_texture = true;
        }
      }
      break;
    case Texture::TT_3d_texture:
      // compression of 3d textures not supported by all video chips
      break;
  }

  if (texture_stored_compressed && !compress_texture) {
    // If we're going to need to reload the texture to get its uncompressed
    // image, we'd better do so now, *before* we figure out the source format.
    // We have to do this early, even though we're going to do it again in
    // fill_d3d_texture_pixels(), because sometimes reloading the original ram
    // image will change the texture's apparent pixel format (the compressed
    // form may have a different format than the uncompressed form).
    tex->get_uncompressed_ram_image();

    orig_width = (DWORD)tex->get_x_size();
    orig_height = (DWORD)tex->get_y_size();
    orig_depth = (DWORD)tex->get_z_size();
  }

  // bpp indicates requested fmt, not texture fmt
  DWORD target_bpp = get_bits_per_pixel(tex->get_format(), &num_alpha_bits);
  DWORD num_color_channels = tex->get_num_components();

  // figure out what 'D3DFMT' the Texture is in, so D3DXLoadSurfFromMem knows
  // how to perform copy
  switch (tex->get_format()) {
  case Texture::F_depth_stencil:
    _d3d_format = D3DFMT_D24S8;
    needs_depth = true;
    needs_stencil = true;
    break;

  case Texture::F_depth_component:
  case Texture::F_depth_component16:
    _d3d_format = D3DFMT_D16;
    needs_depth = true;
    break;

  case Texture::F_depth_component24:
    _d3d_format = D3DFMT_D24X8;
    needs_depth = true;
    break;

  case Texture::F_depth_component32:
    _d3d_format = D3DFMT_D32;
    needs_depth = true;
    break;

  case Texture::F_luminance:
  case Texture::F_sluminance:
    _d3d_format = D3DFMT_L8;
    needs_luminance = true;
    break;

  case Texture::F_luminance_alpha:
  case Texture::F_luminance_alphamask:
  case Texture::F_sluminance_alpha:
    _d3d_format = D3DFMT_A8L8;
    needs_luminance = true;
    break;

  default:
    if (num_alpha_bits > 0) {
      if (num_color_channels == 3) {
        dxgsg9_cat.error()
          << "texture " << tex->get_name()
          << " has no inherent alpha channel, but alpha format is requested!\n";
      }
    }

    _d3d_format = D3DFMT_UNKNOWN;

    switch (num_color_channels) {
    case 1:
      if (num_alpha_bits > 0) {
        _d3d_format = D3DFMT_A8;
      } else {
        _d3d_format = D3DFMT_L8;
      }
      break;
    case 2:
      _d3d_format = D3DFMT_A8L8;
      break;
    case 3:
      _d3d_format = D3DFMT_R8G8B8;
      break;
    case 4:
      _d3d_format = D3DFMT_A8R8G8B8;
      break;
    }

    // make sure we handled all the possible cases
    nassertr(_d3d_format != D3DFMT_UNKNOWN, false);
  }

  DWORD target_width = orig_width;
  DWORD target_height = orig_height;
  DWORD target_depth = orig_depth;

  DWORD filter_caps;

  switch (tex->get_texture_type()) {
  case Texture::TT_1d_texture:
  case Texture::TT_2d_texture:
    filter_caps = scrn._d3dcaps.TextureFilterCaps;

    if (target_width > scrn._d3dcaps.MaxTextureWidth) {
      target_width = scrn._d3dcaps.MaxTextureWidth;
    }
    if (target_height > scrn._d3dcaps.MaxTextureHeight) {
      target_height = scrn._d3dcaps.MaxTextureHeight;
    }

    if (scrn._d3dcaps.TextureCaps & D3DPTEXTURECAPS_POW2) {
      if (!ISPOW2(target_width)) {
        target_width = down_to_power_2(target_width);
      }
      if (!ISPOW2(target_height)) {
        target_height = down_to_power_2(target_height);
      }
    }
    break;

  case Texture::TT_3d_texture:
    if ((scrn._d3dcaps.TextureCaps & D3DPTEXTURECAPS_VOLUMEMAP) == 0) {
      dxgsg9_cat.warning()
        << "3-d textures are not supported by this graphics driver.\n";
      return false;
    }

    filter_caps = scrn._d3dcaps.VolumeTextureFilterCaps;

    if (target_width > scrn._d3dcaps.MaxVolumeExtent) {
      target_width = scrn._d3dcaps.MaxVolumeExtent;
    }
    if (target_height > scrn._d3dcaps.MaxVolumeExtent) {
      target_height = scrn._d3dcaps.MaxVolumeExtent;
    }
    if (target_depth > scrn._d3dcaps.MaxVolumeExtent) {
      target_depth = scrn._d3dcaps.MaxVolumeExtent;
    }

    if (scrn._d3dcaps.TextureCaps & D3DPTEXTURECAPS_VOLUMEMAP_POW2) {
      if (!ISPOW2(target_width)) {
        target_width = down_to_power_2(target_width);
      }
      if (!ISPOW2(target_height)) {
        target_height = down_to_power_2(target_height);
      }
      if (!ISPOW2(target_depth)) {
        target_depth = down_to_power_2(target_depth);
      }
    }
    break;

  case Texture::TT_cube_map:
    if ((scrn._d3dcaps.TextureCaps & D3DPTEXTURECAPS_CUBEMAP) == 0) {
      dxgsg9_cat.warning()
        << "Cube map textures are not supported by this graphics driver.\n";
      return false;
    }

    filter_caps = scrn._d3dcaps.CubeTextureFilterCaps;

    if (target_width > scrn._d3dcaps.MaxTextureWidth) {
      target_width = scrn._d3dcaps.MaxTextureWidth;
    }

    if (scrn._d3dcaps.TextureCaps & D3DPTEXTURECAPS_CUBEMAP_POW2) {
      if (!ISPOW2(target_width)) {
        target_width = down_to_power_2(target_width);
      }
    }

    target_height = target_width;
    break;
  }

  // checks for SQUARE reqmt (nvidia riva128 needs this)
  if ((target_width != target_height) &&
      (scrn._d3dcaps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY) != 0) {
    // assume pow2 textures.  sum exponents, divide by 2 rounding down to get
    // sq size
    int i, width_exp, height_exp;
    for (i = target_width, width_exp = 0; i > 1; width_exp++, i >>= 1) {
    }
    for (i = target_height, height_exp = 0; i > 1; height_exp++, i >>= 1) {
    }
    target_height = target_width = 1<<((width_exp+height_exp)>>1);
  }

  bool shrink_original = false;

  if (orig_width != target_width || orig_height != target_height ||
      orig_depth != target_depth) {
    if (tex->get_texture_type() == Texture::TT_3d_texture) {
      dxgsg9_cat.info()
        << "Reducing size of " << tex->get_name()
        << " from " << orig_width << "x" << orig_height << "x" << orig_depth
        << " to " << target_width << "x" << target_height
        << "x" << target_depth << "\n";
    } else {
      dxgsg9_cat.info()
        << "Reducing size of " << tex->get_name()
        << " from " << orig_width << "x" << orig_height
        << " to " << target_width << "x" << target_height << "\n";
    }

    shrink_original = true;
  }

  if (target_width == 0 || target_height == 0) {
    // We can't create a zero-sized texture, so change it to 1x1.
    target_width = 1;
    target_height = 1;
  }

  const char *error_message;

  error_message = "create_texture failed: couldn't find compatible device Texture Pixel Format for input texture";

  if (dxgsg9_cat.is_spam()) {
    dxgsg9_cat.spam()
      << "create_texture handling target bitdepth: " << target_bpp
      << " alphabits: " << num_alpha_bits << endl;
  }

  // I could possibly replace some of this logic with
  // D3DXCheckTextureRequirements(), but it wouldn't handle all my specialized
  // low-memory cases perfectly

#define CHECK_FOR_FMT(FMT)  \
                    if (scrn._supported_tex_formats_mask & FMT##_FLAG) {   \
                        target_pixel_format = D3DFMT_##FMT;                 \
                        goto found_matching_format; }

  if (texture_stored_compressed && compress_texture) {
    // if the texture is already compressed, we need to choose the
    // corresponding format, otherwise we might end up cross-compressing from
    // e.g.  DXT5 to DXT3
    switch (compression_mode){
    case Texture::CM_dxt1:
      CHECK_FOR_FMT(DXT1);
      break;
    case Texture::CM_dxt2:
      CHECK_FOR_FMT(DXT2);
      break;
    case Texture::CM_dxt3:
      CHECK_FOR_FMT(DXT3);
      break;
    case Texture::CM_dxt4:
      CHECK_FOR_FMT(DXT4);
      break;
    case Texture::CM_dxt5:
      CHECK_FOR_FMT(DXT5);
      break;
    case Texture::CM_rgtc:
      if (num_color_channels == 1) {
        CHECK_FOR_FMT(ATI1);
      } else {
        CHECK_FOR_FMT(ATI2);
      }
      break;
    }

    // We don't support the compressed format.  Fall through.
  }

  if (compress_texture) {
    if (num_color_channels == 1) {
      CHECK_FOR_FMT(ATI1);
    } else if (num_alpha_bits == 0 && num_color_channels == 2) {
      CHECK_FOR_FMT(ATI2);
    }
    if (num_alpha_bits <= 1) {
      CHECK_FOR_FMT(DXT1);
    } else if (num_alpha_bits <= 4) {
      CHECK_FOR_FMT(DXT3);
    } else {
      CHECK_FOR_FMT(DXT5);
    }
  }

  // We can't compress for some reason, so ensure the uncompressed image is
  // ready to load.
  if (texture_stored_compressed) {
    tex->get_uncompressed_ram_image();
    compression_mode = tex->get_ram_image_compression();
    texture_stored_compressed = compression_mode != Texture::CM_off;
    compress_texture = false;
  }

  // handle each target bitdepth separately.  might be less confusing to reorg
  // by num_color_channels (input type, rather than desired 1st target)
  switch (target_bpp) {

    // IMPORTANT NOTE: target_bpp is REQUESTED bpp, not what exists in the
    // texture array (the texture array contains num_color_channels*8bits)

  case 128:
    // check if format is supported
    if (scrn._supports_rgba32_texture_format) {
      target_pixel_format = D3DFMT_A32B32G32R32F;
    }
    else {
      target_pixel_format = scrn._render_to_texture_d3d_format;
    }
    goto found_matching_format;

  case 64:
    // check if format is supported
    if (scrn._supports_rgba16f_texture_format) {
      target_pixel_format = D3DFMT_A16B16G16R16F;
    }
    else {
      target_pixel_format = scrn._render_to_texture_d3d_format;
    }
    goto found_matching_format;

  case 32:
    if (needs_depth) {
      nassertr(num_alpha_bits == 0, false);
      nassertr(num_color_channels == 1, false);

      CHECK_FOR_FMT(D32);
      break;
    }

    if (!((num_color_channels == 3) || (num_color_channels == 4)))
      break; //bail

    CHECK_FOR_FMT(A8R8G8B8);

    if (num_alpha_bits>0) {
      nassertr(num_color_channels == 4, false);

      // no 32-bit fmt, look for 16 bit walpha  (1-15)

      // 32 bit RGBA was requested, but only 16 bit alpha fmts are avail.  By
      // default, convert to 4-4-4-4 which has 4-bit alpha for blurry edges.
      // If we know tex only needs 1 bit alpha (i.e.  for a mask), use 1555
      // instead.


      // ConversionType ConvTo1 = Conv32to16_4444, ConvTo2 = Conv32to16_1555;
      // DWORD dwAlphaMask1 = 0xF000, dwAlphaMask2 = 0x8000;

      // assume ALPHAMASK is x8000 and RGBMASK is x7fff to simplify 32->16
      // conversion.  This should be true on most cards.

      if (num_alpha_bits == 1) {
        CHECK_FOR_FMT(A1R5G5B5);
      }

      // normally prefer 4444 due to better alpha channel resolution
      CHECK_FOR_FMT(A4R4G4B4);
      CHECK_FOR_FMT(A1R5G5B5);

      // At this point, bail.  Don't worry about converting to non-alpha
      // formats yet, I think this will be a very rare case.
      error_message = "create_texture failed: couldn't find compatible Tex DDPIXELFORMAT! no available 16 or 32-bit alpha formats!";
    } else {
      // convert 3 or 4 channel to closest 16bpp color fmt

      if (num_color_channels == 3) {
        CHECK_FOR_FMT(R5G6B5);
        CHECK_FOR_FMT(X1R5G5B5);
      } else {
        CHECK_FOR_FMT(R5G6B5);
        CHECK_FOR_FMT(X1R5G5B5);
      }
    }
    break;

  case 24:
    if (needs_depth) {
      nassertr(num_alpha_bits == 0, false);
      nassertr(num_color_channels == 1, false);

      // In DirectX 9, all built-in depth formats use shadow map filtering.
      // Some drivers (GeForce 8000+, Radeon HD 4000+, Intel G45+) expose a
      // FourCC format called "INTZ" that allows access to the actual depth.
      if (tex->get_minfilter() == Texture::FT_shadow) {
        if (needs_stencil) {
          CHECK_FOR_FMT(D24S8);
        }
        CHECK_FOR_FMT(D24X8);
        CHECK_FOR_FMT(D32);
        CHECK_FOR_FMT(D16);
      } else {
        if (scrn._supported_tex_formats_mask & INTZ_FLAG) {
          target_pixel_format = D3DFMT_INTZ;
          goto found_matching_format;
        }

        // We fall back to a depth format.  Chances are that it is going to be
        // used for shadow mapping, in which case the depth comparison will
        // probably still result in a useful value.
        CHECK_FOR_FMT(D24X8);
      }
    } else {
      nassertr(num_color_channels == 3, false);

      CHECK_FOR_FMT(R8G8B8);

      // no 24-bit fmt.  look for 32 bit fmt (note: this is memory-hogging
      // choice instead I could look for memory-conserving 16-bit fmt).

      CHECK_FOR_FMT(X8R8G8B8);
      CHECK_FOR_FMT(A8R8G8B8);

      // no 24-bit or 32 fmt.  look for 16 bit fmt (higher res 565 1st)
      CHECK_FOR_FMT(R5G6B5);
      CHECK_FOR_FMT(X1R5G5B5);
      CHECK_FOR_FMT(A1R5G5B5);
    }
    break;

  case 16:
    if (needs_depth) {
      nassertr(num_alpha_bits == 0, false);
      nassertr(num_color_channels == 1, false);

      // In DirectX 9, all built-in depth formats use shadow map filtering.
      // Some drivers (GeForce 8000+, Radeon HD 4000+, Intel G45+) expose a
      // FourCC format called "INTZ" that allows access to the actual depth.
      if (tex->get_minfilter() == Texture::FT_shadow) {
        if (needs_stencil) {
          CHECK_FOR_FMT(D24S8);
        }
        CHECK_FOR_FMT(D16);
        CHECK_FOR_FMT(D24X8);
        CHECK_FOR_FMT(D32);
      } else {
        if (scrn._supported_tex_formats_mask & INTZ_FLAG) {
          target_pixel_format = D3DFMT_INTZ;
          goto found_matching_format;
        }

        // We fall back to a depth format.  Chances are that it is going to be
        // used for shadow mapping, in which case the depth comparison will
        // probably still result in a useful value.
        CHECK_FOR_FMT(D24X8);
      }

    } else if (needs_luminance) {
      nassertr(num_alpha_bits > 0, false);
      nassertr(num_color_channels == 2, false);

      CHECK_FOR_FMT(A8L8);
      CHECK_FOR_FMT(A8R8G8B8);

      if (num_alpha_bits == 1) {
        CHECK_FOR_FMT(A1R5G5B5);
      }

      // normally prefer 4444 due to better alpha channel resolution
      CHECK_FOR_FMT(A4R4G4B4);
      CHECK_FOR_FMT(A1R5G5B5);
    } else {
      nassertr((num_color_channels == 3)||(num_color_channels == 4), false);
      // look for compatible 16bit fmts, if none then give up (don't worry
      // about other bitdepths for 16 bit)
      switch(num_alpha_bits) {
      case 0:
        if (num_color_channels == 3) {
          CHECK_FOR_FMT(R5G6B5);
          CHECK_FOR_FMT(X1R5G5B5);
        } else {
          nassertr(num_color_channels == 4, false);
          // it could be 4 if user asks us to throw away the alpha channel
          CHECK_FOR_FMT(R5G6B5);
          CHECK_FOR_FMT(X1R5G5B5);
        }
        break;
      case 1:
        // app specifically requests 1-5-5-5 F_rgba5 case, where you
        // explicitly want 1-5-5-5 fmt, as opposed to F_rgbm, which could use
        // 32bpp ARGB.  fail if this particular fmt not avail.
        nassertr(num_color_channels == 4, false);
        CHECK_FOR_FMT(X1R5G5B5);
        break;
      case 4:
        // app specifically requests 4-4-4-4 F_rgba4 case, as opposed to
        // F_rgba, which could use 32bpp ARGB
        nassertr(num_color_channels == 4, false);
        CHECK_FOR_FMT(A4R4G4B4);
        break;
      default:
        nassertr(false, false);  // problem in get_bits_per_pixel()?
      }
    }
  case 8:
    if (needs_luminance) {
      // don't bother handling those other 8bit lum fmts like 4-4, since 16
      // 8-8 is usually supported too
      nassertr(num_color_channels == 1, false);

      // look for native lum fmt first
      CHECK_FOR_FMT(L8);
      CHECK_FOR_FMT(L8);

      CHECK_FOR_FMT(R8G8B8);
      CHECK_FOR_FMT(X8R8G8B8);

      CHECK_FOR_FMT(R5G6B5);
      CHECK_FOR_FMT(X1R5G5B5);

    } else if (num_alpha_bits == 8) {
      // look for 16bpp A8L8, else 32-bit ARGB, else 16-4444.

      // skip 8bit alpha only (D3DFMT_A8), because I think only voodoo
      // supports it and the voodoo support isn't the kind of blending model
      // we need somehow (is it that voodoo assumes color is white?  isnt that
      // what we do in ConvAlpha8to32 anyway?)

      CHECK_FOR_FMT(A8L8);
      CHECK_FOR_FMT(A8R8G8B8);
      CHECK_FOR_FMT(A4R4G4B4);
    }
    break;

  default:
    error_message = "create_texture failed: unhandled pixel bitdepth in DX loader";
  }

  // if we've gotten here, haven't found a match
  dxgsg9_cat.error()
    << error_message << ": " << tex->get_name() << endl
    << "NumColorChannels: " << num_color_channels << "; NumAlphaBits: "
    << num_alpha_bits << "; targetbpp: " <<target_bpp
    << "; _supported_tex_formats_mask: 0x"
    << std::hex << scrn._supported_tex_formats_mask << std::dec
    << "; NeedLuminance: " << needs_luminance << endl;
  goto error_exit;


 found_matching_format:
  // We found a suitable format that matches the texture's format.

  if (tex->get_match_framebuffer_format()) {
    // Instead of creating a texture with the found format, we will need to
    // make one that exactly matches the framebuffer's format.  Look up what
    // that format is.
    IDirect3DSurface9 *render_target;

    if (needs_depth) {
      hr = scrn._d3d_device->GetDepthStencilSurface(&render_target);
    } else {
      hr = scrn._d3d_device->GetRenderTarget(0, &render_target);
    }
    if (FAILED(hr)) {
      dxgsg9_cat.error()
        << "GetRenderTgt failed in create_texture: " << D3DERRORSTRING(hr);
    } else {
      D3DSURFACE_DESC surface_desc;
      hr = render_target->GetDesc(&surface_desc);
      if (FAILED(hr)) {
        dxgsg9_cat.error()
          << "GetDesc failed in create_texture: " << D3DERRORSTRING(hr);
      } else {
        if (target_pixel_format != surface_desc.Format &&
            target_pixel_format != D3DFMT_INTZ) {
          if (dxgsg9_cat.is_debug()) {
            dxgsg9_cat.debug()
              << "Chose format " << D3DFormatStr(surface_desc.Format)
              << " instead of " << D3DFormatStr(target_pixel_format)
              << " for texture to match framebuffer.\n";
          }
          target_pixel_format = surface_desc.Format;
        }
      }
      SAFE_RELEASE(render_target);
    }
  }

  // validate magfilter setting degrade filtering if no HW support

  SamplerState::FilterType ft;

  ft = tex->get_magfilter();
  if ((ft != SamplerState::FT_linear) && ft != SamplerState::FT_nearest) {
    // mipmap settings make no sense for magfilter
    if (ft == SamplerState::FT_nearest_mipmap_nearest) {
      ft = SamplerState::FT_nearest;
    } else {
      ft = SamplerState::FT_linear;
    }
  }

  if (ft == SamplerState::FT_linear &&
      (filter_caps & D3DPTFILTERCAPS_MAGFLINEAR) == 0) {
    ft = SamplerState::FT_nearest;
  }
  tex->set_magfilter(ft);

  // figure out if we are mipmapping this texture
  ft = tex->get_minfilter();
  _has_mipmaps = false;

  if (!dx_ignore_mipmaps) {  // set if no HW mipmap capable
    switch(ft) {
    case SamplerState::FT_nearest_mipmap_nearest:
    case SamplerState::FT_linear_mipmap_nearest:
    case SamplerState::FT_nearest_mipmap_linear:  // pick nearest in each, interpolate linearly b/w them
    case SamplerState::FT_linear_mipmap_linear:
      _has_mipmaps = true;
    }

    if (dx_mipmap_everything) {  // debug toggle, ok to leave in since its just a creation cost
      _has_mipmaps = true;
      if (dxgsg9_cat.is_spam()) {
        if (ft != SamplerState::FT_linear_mipmap_linear) {
          dxgsg9_cat.spam()
            << "Forcing trilinear mipmapping on DX texture ["
            << tex->get_name() << "]\n";
        }
      }
      ft = SamplerState::FT_linear_mipmap_linear;
      tex->set_minfilter(ft);
    }

  } else if ((ft == SamplerState::FT_nearest_mipmap_nearest) ||   // cvt to no-mipmap filter types
             (ft == SamplerState::FT_nearest_mipmap_linear)) {
    ft = SamplerState::FT_nearest;

  } else if ((ft == SamplerState::FT_linear_mipmap_nearest) ||
            (ft == SamplerState::FT_linear_mipmap_linear)) {
    ft = SamplerState::FT_linear;
  }

  nassertr((filter_caps & D3DPTFILTERCAPS_MINFPOINT) != 0, false);

#define TRILINEAR_MIPMAP_TEXFILTERCAPS (D3DPTFILTERCAPS_MIPFLINEAR | D3DPTFILTERCAPS_MINFLINEAR)

  // do any other filter type degradations necessary
  switch(ft) {
  case SamplerState::FT_linear_mipmap_linear:
    if ((filter_caps & TRILINEAR_MIPMAP_TEXFILTERCAPS) != TRILINEAR_MIPMAP_TEXFILTERCAPS) {
      if (filter_caps & D3DPTFILTERCAPS_MINFLINEAR) {
        ft = SamplerState::FT_linear_mipmap_nearest;
      } else {
        // if you cant do linear in a level, you probably cant do linear bw
        // levels, so just do nearest-all
        ft = SamplerState::FT_nearest_mipmap_nearest;
      }
    }
    break;

  case SamplerState::FT_nearest_mipmap_linear:
    // if we don't have bilinear, do nearest_nearest
    if (!((filter_caps & D3DPTFILTERCAPS_MIPFPOINT) &&
          (filter_caps & D3DPTFILTERCAPS_MINFLINEAR))) {
      ft = SamplerState::FT_nearest_mipmap_nearest;
    }
    break;

  case SamplerState::FT_linear_mipmap_nearest:
    // if we don't have mip linear, do nearest_nearest
    if (!(filter_caps & D3DPTFILTERCAPS_MIPFLINEAR)) {
      ft = SamplerState::FT_nearest_mipmap_nearest;
    }
    break;

  case SamplerState::FT_linear:
    if (!(filter_caps & D3DPTFILTERCAPS_MINFLINEAR)) {
      ft = SamplerState::FT_nearest;
    }
    break;
  }

  tex->set_minfilter(ft);

  uint aniso_degree;

  aniso_degree = 1;
  if (scrn._d3dcaps.RasterCaps & D3DPRASTERCAPS_ANISOTROPY) {
    aniso_degree = tex->get_anisotropic_degree();
    if ((aniso_degree>scrn._d3dcaps.MaxAnisotropy) ||
        dx_force_anisotropic_filtering) {
      aniso_degree = scrn._d3dcaps.MaxAnisotropy;
    }
  }
  tex->set_anisotropic_degree(aniso_degree);

#ifdef _DEBUG
  if (dxgsg9_cat.is_spam()) {
    dxgsg9_cat.spam()
      << "create_texture: setting aniso degree for " << tex->get_name()
      << " to: " << aniso_degree << endl;
  }
#endif

  UINT mip_level_count;

  if (_has_mipmaps) {
    tex->get_ram_image();
    mip_level_count = tex->get_num_loadable_ram_mipmap_images();
    if (mip_level_count < 2) {
      // tell CreateTex to alloc space for all mip levels down to 1x1
      mip_level_count = 0;

      if (dxgsg9_cat.is_debug()) {
        dxgsg9_cat.debug()
          << "create_texture: generating mipmaps for " << tex->get_name()
          << endl;
      }
    }

    // DirectX will corrupt memory if we try to load mipmaps smaller than 4x4
    // for ATI1 or ATI2 textures.
    if (target_pixel_format == D3DFMT_ATI1 ||
        target_pixel_format == D3DFMT_ATI2) {

      UINT dimension = min(target_height, target_width);
      mip_level_count = 0;
      while (dimension >= 4) {
        ++mip_level_count;
        dimension >>= 1;
      }

      if ((UINT)tex->get_num_ram_mipmap_images() < mip_level_count) {
        // We also have to generate mipmaps on the CPU for these.
        tex->generate_ram_mipmap_images();
        mip_level_count = min(mip_level_count, (UINT)tex->get_num_ram_mipmap_images());
      }
    }
  } else {
    mip_level_count = 1;
  }

  DWORD usage;
  D3DPOOL pool;

  if (tex->get_render_to_texture ( )) {
    // REQUIRED PARAMETERS
    _managed = false;
    _is_render_target = true;

    pool = D3DPOOL_DEFAULT;
    if (needs_depth) {
      usage = D3DUSAGE_DEPTHSTENCIL;
    } else {
      usage = D3DUSAGE_RENDERTARGET;
    }
    // if (target_bpp <= 32) { target_pixel_format =
    // scrn._render_to_texture_d3d_format; }

    dxgsg9_cat.debug ()
      << "*** RENDER TO TEXTURE ***: format "
      << D3DFormatStr(target_pixel_format)
      << "  "
      << target_pixel_format
      << "\n";
  }
  else {
    _managed = scrn._managed_textures;
    if (_managed) {
      pool = D3DPOOL_MANAGED;
      usage = 0;
    }
    else {
      if (scrn._supports_automatic_mipmap_generation) {
        pool = D3DPOOL_DEFAULT;
        usage = D3DUSAGE_AUTOGENMIPMAP;
      }
      else {
        if (dx_use_dynamic_textures) {
          if (scrn._supports_dynamic_textures) {
            pool = D3DPOOL_DEFAULT;
            usage = D3DUSAGE_DYNAMIC;
          }
          else {
            // can't lock textures so go back to managed for now need to use
            // UpdateTexture or UpdateSurface
            _managed = true;
            pool = D3DPOOL_MANAGED;
            usage = 0;
          }
        }
        else {
          pool = D3DPOOL_DEFAULT;
          usage = 0;
        }
      }
    }
  }

  PN_stdfloat bytes_per_texel;

  bytes_per_texel = this -> d3d_format_to_bytes_per_pixel (target_pixel_format);
  if (bytes_per_texel == 0.0f) {
    dxgsg9_cat.error()
      << "D3D create_texture ( ) unknown texture format\n";
  }

  int data_size;

  data_size = target_width * target_height * target_depth;
  if (_has_mipmaps) {
    data_size = (int) ((PN_stdfloat) data_size * 1.3333333);
  }
  data_size = (int) ((PN_stdfloat) data_size * bytes_per_texel);
  if (tex->get_texture_type() == Texture::TT_cube_map) {
    data_size *= 6;
  }
  update_data_size_bytes(data_size);

  int attempts;

  if (dxgsg9_cat.is_debug()) {
    dxgsg9_cat.debug()
      << "Creating " << *tex << ", " << data_size << " bytes, "
      << scrn._d3d_device->GetAvailableTextureMem()
      << " reported available.\n";
    dxgsg9_cat.debug()
      << " size is " << target_width << " w * " << target_height << " h * "
      << target_depth << " d";
    if (_has_mipmaps) {
      dxgsg9_cat.debug(false)
        << " * 1.3333333 mipmaps";
    }
    dxgsg9_cat.debug(false)
      << " * " << bytes_per_texel << " bpt";
    if (tex->get_texture_type() == Texture::TT_cube_map) {
      dxgsg9_cat.debug(false)
        << " * 6 faces";
    }
    dxgsg9_cat.debug(false)
      << "\n";
  }

  attempts = 0;
  do
  {
    switch (tex->get_texture_type()) {
    case Texture::TT_1d_texture:
    case Texture::TT_2d_texture:
      hr = scrn._d3d_device->CreateTexture
        (target_width, target_height, mip_level_count, usage,
         target_pixel_format, pool, &_d3d_2d_texture, nullptr);
      _d3d_texture = _d3d_2d_texture;
      break;

    case Texture::TT_3d_texture:
      hr = scrn._d3d_device->CreateVolumeTexture
        (target_width, target_height, target_depth, mip_level_count, usage,
         target_pixel_format, pool, &_d3d_volume_texture, nullptr);
      _d3d_texture = _d3d_volume_texture;
      break;

    case Texture::TT_cube_map:
      hr = scrn._d3d_device->CreateCubeTexture
        (target_width, mip_level_count, usage,
         target_pixel_format, pool, &_d3d_cube_texture, nullptr);
      _d3d_texture = _d3d_cube_texture;

      target_height = target_width;
      break;
    }

    attempts++;
  } while (scrn._dxgsg9 -> check_dx_allocation (hr, data_size, attempts));

  if (FAILED(hr)) {
    dxgsg9_cat.error()
      << "D3D create_texture failed!" << D3DERRORSTRING(hr);
    dxgsg9_cat.error()
      << "  width = " << target_width << " height = " << target_height << " target_pixel_format = " << target_pixel_format << "\n";

    goto error_exit;
  }

  if (DEBUG_TEXTURES && dxgsg9_cat.is_debug()) {
    dxgsg9_cat.debug()
      << "create_texture: " << tex->get_name()
      << " converting panda equivalent of " << D3DFormatStr(_d3d_format)
      << " => " << D3DFormatStr(target_pixel_format) << endl;
  }

  hr = fill_d3d_texture_pixels(scrn, compress_texture);
  if (FAILED(hr)) {

    dxgsg9_cat.debug ()
      << "*** fill_d3d_texture_pixels failed ***: format "
      << target_pixel_format
      << "\n";

    goto error_exit;
  }

  if (tex->get_post_load_store_cache()) {
    tex->set_post_load_store_cache(false);
    // OK, get the RAM image, and save it in a BamCache record.
    if (extract_texture_data(scrn)) {
      if (tex->has_ram_image()) {
        BamCache *cache = BamCache::get_global_ptr();
        PT(BamCacheRecord) record = cache->lookup(tex->get_fullpath(), "txo");
        if (record != nullptr) {
          record->set_data(tex, tex);
          cache->store(record);
        }
      }
    }
  }

  // must not put render to texture into LRU
  if (!_managed && !tex->get_render_to_texture()) {
    scrn._dxgsg9->get_engine()->texture_uploaded(tex);
  }
  mark_loaded();

  return true;

 error_exit:

  RELEASE(_d3d_texture, dxgsg9, "texture", RELEASE_ONCE);
  _d3d_2d_texture = nullptr;
  _d3d_volume_texture = nullptr;
  _d3d_cube_texture = nullptr;
  return false;
}

/**
 *
 */
bool DXTextureContext9::
create_simple_texture(DXScreenData &scrn) {
  nassertr(IS_VALID_PTR(get_texture()), false);

  HRESULT hr;

  delete_texture();

  _d3d_format = D3DFMT_A8R8G8B8;
  D3DFORMAT target_pixel_format = D3DFMT_A8R8G8B8;
  DWORD target_bpp = 32;
  DWORD num_color_channels = 4;

  DWORD target_width = (DWORD)get_texture()->get_simple_x_size();
  DWORD target_height = (DWORD)get_texture()->get_simple_y_size();
  DWORD mip_level_count = 1;
  DWORD usage = 0;
  D3DPOOL pool = D3DPOOL_MANAGED;

  int data_size = target_width * target_height * 4;

  hr = scrn._d3d_device->CreateTexture
    (target_width, target_height, mip_level_count, usage,
     target_pixel_format, pool, &_d3d_2d_texture, nullptr);
  _d3d_texture = _d3d_2d_texture;
  if (FAILED(hr)) {
    dxgsg9_cat.error()
      << "D3D create_simple_texture failed!" << D3DERRORSTRING(hr);
    dxgsg9_cat.error()
      << "  width = " << target_width << " height = " << target_height << " target_pixel_format = " << target_pixel_format << "\n";

    goto error_exit;
  }

  if (DEBUG_TEXTURES && dxgsg9_cat.is_debug()) {
    dxgsg9_cat.debug()
      << "create_simple_texture: " << get_texture()->get_name()
      << "\n";
  }

  {
    CPTA_uchar image = get_texture()->get_simple_ram_image();

    hr = -1;
    // hr = fill_d3d_texture_pixels(scrn);

    IDirect3DSurface9 *surface = nullptr;
    _d3d_2d_texture->GetSurfaceLevel(0, &surface);

    RECT source_size;
    source_size.left = source_size.top = 0;
    source_size.right = target_width;
    source_size.bottom = target_height;

    DWORD mip_filter = D3DX_FILTER_LINEAR;

    hr = D3DXLoadSurfaceFromMemory
      (surface, nullptr, nullptr, (LPCVOID)image.p(),
       target_pixel_format, target_width * 4, nullptr,
       &source_size, mip_filter, (D3DCOLOR)0x0);

    RELEASE(surface, dxgsg9, "create_simple_texture Surface", RELEASE_ONCE);
  }

  if (FAILED(hr)) {
    dxgsg9_cat.debug ()
      << "*** fill_d3d_texture_pixels failed ***: format "
      << target_pixel_format
      << "\n";

    goto error_exit;
  }

  mark_simple_loaded();
  return true;

 error_exit:
  RELEASE(_d3d_texture, dxgsg9, "texture", RELEASE_ONCE);
  _d3d_2d_texture = nullptr;
  _d3d_volume_texture = nullptr;
  _d3d_cube_texture = nullptr;
  return false;
}

/**
 * Release the surface used to store the texture
 */
void DXTextureContext9::
delete_texture() {

  if (_d3d_texture == nullptr) {
    // don't bother printing the msg below, since we already released it.
    return;
  }

  RELEASE(_d3d_texture, dxgsg9, "texture", RELEASE_ONCE);
  _d3d_2d_texture = nullptr;
  _d3d_volume_texture = nullptr;
  _d3d_cube_texture = nullptr;
}

/**
 * This method will be called in the draw thread to download the texture
 * memory's image into its ram_image value.  It returns true on success, false
 * otherwise.
 */
bool DXTextureContext9::
extract_texture_data(DXScreenData &screen) {
  bool state;
  HRESULT hr;

  state = false;
  Texture *tex = get_texture();
  if (tex->get_texture_type() != Texture::TT_2d_texture) {
    dxgsg9_cat.error()
      << "Not supported: extract_texture_data for " << tex->get_texture_type()
      << "\n";
    return state;
  }
  nassertr(IS_VALID_PTR(_d3d_2d_texture), false);

  D3DSURFACE_DESC desc;
  hr = _d3d_2d_texture->GetLevelDesc(0, &desc);
  if (FAILED(hr)) {
    dxgsg9_cat.error()
      << "Texture::GetLevelDesc() failed!" << D3DERRORSTRING(hr);
    return state;
  }

  int div = 1;
  Texture::Format format = Texture::F_rgba;
  Texture::CompressionMode compression = Texture::CM_off;

  switch (desc.Format) {
  case D3DFMT_R8G8B8:
    format = Texture::F_rgb;
    break;

  case D3DFMT_A8R8G8B8:
  case D3DFMT_X8R8G8B8:
    break;

  case D3DFMT_L8:
    format = Texture::F_luminance;
    break;

  case D3DFMT_A8L8:
    format = Texture::F_luminance_alpha;
    break;

  case D3DFMT_D24S8:
    format = Texture::F_depth_stencil;
    break;

  case D3DFMT_D16:
    format = Texture::F_depth_component16;
    break;

  case D3DFMT_D24X8:
    format = Texture::F_depth_component24;
    break;

  case D3DFMT_D32:
    format = Texture::F_depth_component32;
    break;

  case D3DFMT_DXT1:
    compression = Texture::CM_dxt1;
    div = 4;
    break;
  case D3DFMT_DXT2:
    compression = Texture::CM_dxt2;
    div = 4;
    break;
  case D3DFMT_DXT3:
    compression = Texture::CM_dxt3;
    div = 4;
    break;
  case D3DFMT_DXT4:
    compression = Texture::CM_dxt4;
    div = 4;
    break;
  case D3DFMT_DXT5:
    compression = Texture::CM_dxt5;
    div = 4;
    break;
  case D3DFMT_ATI1:
  case D3DFMT_ATI2:
    compression = Texture::CM_rgtc;
    div = 4;
    break;

  default:
    dxgsg9_cat.error()
      << "Cannot extract texture data: unhandled surface format "
      << desc.Format << "\n";
    return state;
  }

  int num_levels = _d3d_2d_texture->GetLevelCount();

  tex->set_x_size(desc.Width);
  tex->set_y_size(desc.Height);
  tex->set_z_size(1);
  tex->set_component_type(Texture::T_unsigned_byte);
  tex->set_format(format);
  tex->clear_ram_image();

  if (_is_render_target) {
    IDirect3DSurface9* source_surface;
    IDirect3DSurface9* destination_surface;

    source_surface = 0;
    destination_surface = 0;

    hr = _d3d_2d_texture -> GetSurfaceLevel (0, &source_surface);
    if (hr == D3D_OK) {

      D3DPOOL pool;
      D3DSURFACE_DESC surface_description;

      pool = D3DPOOL_SYSTEMMEM;
      source_surface -> GetDesc (&surface_description);

      hr = screen._d3d_device->CreateOffscreenPlainSurface (
        surface_description.Width,
        surface_description.Height,
        surface_description.Format,
        pool,
        &destination_surface,
        nullptr);
      if (hr == D3D_OK) {
        if (source_surface && destination_surface) {
          hr = screen._d3d_device -> GetRenderTargetData (source_surface, destination_surface);
          if (hr == D3D_OK) {

            D3DLOCKED_RECT rect;

            hr = destination_surface -> LockRect (&rect, nullptr, D3DLOCK_READONLY);
            if (hr == D3D_OK) {

              unsigned int y;
              int size;
              int bytes_per_line;

              int surface_bytes_per_line;
              unsigned char *surface_pointer;

              bytes_per_line = surface_description.Width * this -> d3d_format_to_bytes_per_pixel (surface_description.Format);
              size = bytes_per_line * surface_description.Height;

              surface_bytes_per_line = rect.Pitch;
              surface_pointer = (unsigned char *) rect.pBits;

              PTA_uchar image = PTA_uchar::empty_array(size);

              int offset;

              offset = 0;
              for (y = 0; y < surface_description.Height; y++)
              {
                memcpy (&image [offset], surface_pointer, bytes_per_line);

                offset += bytes_per_line;
                surface_pointer += surface_bytes_per_line;
              }

              tex->set_ram_image(image, Texture::CM_off);

              state = true;

              destination_surface -> UnlockRect();
            }
          }
        }

        destination_surface -> Release ( );
      }
      else {
        dxgsg9_cat.error()
          << "CreateImageSurface failed in extract_texture_data()"
          << D3DERRORSTRING(hr);
      }
      source_surface -> Release ( );
    }
  }
  else {
    for (int n = 0; n < num_levels; ++n) {
      D3DLOCKED_RECT rect;
      hr = _d3d_2d_texture->LockRect(n, &rect, nullptr, D3DLOCK_READONLY);
      if (FAILED(hr)) {
        dxgsg9_cat.error()
          << "Texture::LockRect() failed!  level = " << n << " " << D3DERRORSTRING(hr);
        return state;
      }

      int x_size = tex->get_expected_mipmap_x_size(n);
      int y_size = tex->get_expected_mipmap_y_size(n);
      PTA_uchar image;

      if (compression == Texture::CM_off) {
        // Uncompressed, but we have to respect the pitch.
        int pitch = x_size * tex->get_num_components() * tex->get_component_width();
        pitch = min(pitch, (int)rect.Pitch);
        int size = pitch * y_size;
        image = PTA_uchar::empty_array(size);
        if (pitch == rect.Pitch) {
          // Easy copy.
          memcpy(image.p(), rect.pBits, size);
        } else {
          // Harder copy: we have to de-interleave DirectX's extra bytes on
          // the end of each row.
          unsigned char *dest = image.p();
          unsigned char *source = (unsigned char *)rect.pBits;
          for (int yi = 0; yi < y_size; ++yi) {
            memcpy(dest, source, pitch);
            dest += pitch;
            source += rect.Pitch;
          }
        }

      } else {
        // Compressed; just copy the data verbatim.
        int size = rect.Pitch * (y_size / div);
        image = PTA_uchar::empty_array(size);
        memcpy(image.p(), rect.pBits, size);
      }

      _d3d_2d_texture->UnlockRect(n);
      if (n == 0) {
        tex->set_ram_image(image, compression);
      } else {
        tex->set_ram_mipmap_image(n, image);
      }
    }

    state = true;
  }

  return state;
}

/**
 * copies source_rect in pD3DSurf to upper left of texture
 */
HRESULT DXTextureContext9::
d3d_surface_to_texture(RECT &source_rect, IDirect3DSurface9 *d3d_surface,
           bool inverted, Texture *result, int view, int z) {

  // still need custom conversion since d3dd3dx has no way to convert
  // arbitrary fmt to ARGB in-memory user buffer

  HRESULT hr;
  DWORD num_components = result->get_num_components();

  nassertr(result->get_component_width() == sizeof(BYTE), E_FAIL);   // cant handle anything else now
  nassertr(result->get_component_type() == Texture::T_unsigned_byte, E_FAIL);   // cant handle anything else now
  nassertr((num_components == 3) || (num_components == 4), E_FAIL);  // cant handle anything else now
  nassertr(IS_VALID_PTR(d3d_surface), E_FAIL);

  BYTE *buf = result->modify_ram_image();
  if (z >= 0) {
    nassertr(z < result->get_z_size(), E_FAIL);
    buf += z * result->get_expected_ram_page_size();
  }
  if (view > 0) {
    buf += (view * result->get_z_size()) * result->get_expected_ram_page_size();
  }

  if (IsBadWritePtr(d3d_surface, sizeof(DWORD))) {
    dxgsg9_cat.error()
      << "d3d_surface_to_texture failed: bad pD3DSurf ptr value ("
      << ((void*)d3d_surface) << ")\n";
    exit(1);
  }

  DWORD x_window_offset, y_window_offset;
  DWORD copy_width, copy_height;

  D3DLOCKED_RECT locked_rect;
  D3DSURFACE_DESC surface_desc;

  hr = d3d_surface->GetDesc(&surface_desc);

  x_window_offset = source_rect.left, y_window_offset = source_rect.top;
  copy_width = RECT_XSIZE(source_rect);
  copy_height = RECT_YSIZE(source_rect);

  // make sure there's enough space in the texture, its size must match
  // (especially xsize) or scanlines will be too long

  if (!((copy_width == result->get_x_size()) && (copy_height <= (DWORD)result->get_y_size()))) {
    dxgsg9_cat.error()
      << "d3d_surface_to_texture, Texture size (" << result->get_x_size()
      << ", " << result->get_y_size()
      << ") too small to hold display surface ("
      << copy_width << ", " << copy_height << ")\n";
    nassertr(false, E_FAIL);
    return E_FAIL;
  }

  hr = d3d_surface->LockRect(&locked_rect, nullptr, (D3DLOCK_READONLY | D3DLOCK_NO_DIRTY_UPDATE));
  if (FAILED(hr)) {
    dxgsg9_cat.error()
      << "d3d_surface_to_texture LockRect() failed!" << D3DERRORSTRING(hr);
    return hr;
  }

  // ones not listed not handled yet
  nassertr((surface_desc.Format == D3DFMT_A8R8G8B8) ||
           (surface_desc.Format == D3DFMT_X8R8G8B8) ||
           (surface_desc.Format == D3DFMT_R8G8B8) ||
           (surface_desc.Format == D3DFMT_R5G6B5) ||
           (surface_desc.Format == D3DFMT_X1R5G5B5) ||
           (surface_desc.Format == D3DFMT_A1R5G5B5) ||
           (surface_desc.Format == D3DFMT_A4R4G4B4), E_FAIL);

  // buf contains raw ARGB in Texture byteorder

  int byte_pitch = locked_rect.Pitch;
  BYTE *surface_bytes = (BYTE *)locked_rect.pBits;

  if (inverted) {
    surface_bytes += byte_pitch * (y_window_offset + copy_height - 1);
    byte_pitch = -byte_pitch;
  } else {
    surface_bytes += byte_pitch * y_window_offset;
  }

  // writes out last line in DDSurf first in PixelBuf, so Y line order
  // precedes inversely

  if (DEBUG_SURFACES && dxgsg9_cat.is_debug()) {
    dxgsg9_cat.debug()
      << "d3d_surface_to_texture converting "
      << D3DFormatStr(surface_desc.Format)
      << " DDSurf to " <<  num_components << "-channel panda Texture\n";
  }

  DWORD *dest_word = (DWORD *)buf;
  BYTE *dest_byte = (BYTE *)buf;

  switch(surface_desc.Format) {
  case D3DFMT_A8R8G8B8:
  case D3DFMT_X8R8G8B8: {
    if (num_components == 4) {
      DWORD *source_word;
      BYTE *dest_line = (BYTE*)dest_word;

      for (DWORD y = 0; y < copy_height; y++) {
        source_word = ((DWORD*)surface_bytes) + x_window_offset;
        memcpy(dest_line, source_word, byte_pitch);
        dest_line += byte_pitch;
        surface_bytes += byte_pitch;
      }
    } else {
      // 24bpp texture case (numComponents == 3)
      DWORD *source_word;
      for (DWORD y = 0; y < copy_height; y++) {
        source_word = ((DWORD*)surface_bytes) + x_window_offset;

        for (DWORD x = 0; x < copy_width; x++) {
          BYTE r, g, b;
          DWORD pixel = *source_word;

          r = (BYTE)((pixel>>16) & g_LowByteMask);
          g = (BYTE)((pixel>> 8) & g_LowByteMask);
          b = (BYTE)((pixel    ) & g_LowByteMask);

          *dest_byte++ = b;
          *dest_byte++ = g;
          *dest_byte++ = r;
          source_word++;
        }
        surface_bytes += byte_pitch;
      }
    }
    break;
  }

  case D3DFMT_R8G8B8: {
    BYTE *source_byte;

    if (num_components == 4) {
      for (DWORD y = 0; y < copy_height; y++) {
        source_byte = surface_bytes + x_window_offset * 3 * sizeof(BYTE);
        for (DWORD x = 0; x < copy_width; x++) {
          DWORD r, g, b;

          b = *source_byte++;
          g = *source_byte++;
          r = *source_byte++;

          *dest_word = 0xFF000000 | (r << 16) | (g << 8) | b;
          dest_word++;
        }
        surface_bytes += byte_pitch;
      }
    } else {
      // 24bpp texture case (numComponents == 3)
      for (DWORD y = 0; y < copy_height; y++) {
        source_byte = surface_bytes + x_window_offset * 3 * sizeof(BYTE);
        memcpy(dest_byte, source_byte, byte_pitch);
        dest_byte += byte_pitch;
        surface_bytes += byte_pitch;
      }
    }
    break;
  }

  case D3DFMT_R5G6B5:
  case D3DFMT_X1R5G5B5:
  case D3DFMT_A1R5G5B5:
  case D3DFMT_A4R4G4B4: {
    WORD  *source_word;
    // handle 0555, 1555, 0565, 4444 in same loop

    BYTE redshift, greenshift, blueshift;
    DWORD redmask, greenmask, bluemask;

    if (surface_desc.Format == D3DFMT_R5G6B5) {
      redshift = (11-3);
      redmask = 0xF800;
      greenmask = 0x07E0;
      greenshift = (5-2);
      bluemask = 0x001F;
      blueshift = 3;
    } else if (surface_desc.Format == D3DFMT_A4R4G4B4) {
      redmask = 0x0F00;
      redshift = 4;
      greenmask = 0x00F0;
      greenshift = 0;
      bluemask = 0x000F;
      blueshift = 4;
    } else {  // 1555 or x555
      redmask = 0x7C00;
      redshift = (10-3);
      greenmask = 0x03E0;
      greenshift = (5-3);
      bluemask = 0x001F;
      blueshift = 3;
    }

    if (num_components == 4) {
      // Note: these 16bpp loops ignore input alpha completely (alpha is set
      // to fully opaque in texture!)

      // if we need to capture alpha, probably need to make separate loops for
      // diff 16bpp fmts for best speed

      for (DWORD y = 0; y < copy_height; y++) {
        source_word = ((WORD*)surface_bytes) + x_window_offset;
        for (DWORD x = 0; x < copy_width; x++) {
          WORD pixel = *source_word;
          BYTE r, g, b;

          b = (pixel & bluemask) << blueshift;
          g = (pixel & greenmask) >> greenshift;
          r = (pixel & redmask) >> redshift;

          // alpha is just set to 0xFF

          *dest_word = 0xFF000000 | (r << 16) | (g << 8) | b;
          source_word++;
          dest_word++;
        }
        surface_bytes += byte_pitch;
      }
    } else {
      // 24bpp texture case (numComponents == 3)
      for (DWORD y = 0; y < copy_height; y++) {
        source_word = ((WORD*)surface_bytes) + x_window_offset;
        for (DWORD x = 0; x < copy_width; x++) {
          WORD pixel = *source_word;
          BYTE r, g, b;

          b = (pixel & bluemask) << blueshift;
          g = (pixel & greenmask) >> greenshift;
          r = (pixel & redmask) >> redshift;

          *dest_byte++ = b;
          *dest_byte++ = g;
          *dest_byte++ = r;

          source_word++;
        }
        surface_bytes += byte_pitch;
      }
    }
    break;
  }

  default:
    dxgsg9_cat.error()
      << "d3d_surface_to_texture: unsupported D3DFORMAT!\n";
  }

  d3d_surface->UnlockRect();
  return S_OK;
}

/**
 * local helper function, which calculates the 'row_byte_length' or 'pitch'
 * needed for calling D3DXLoadSurfaceFromMemory.  Takes compressed formats
 * (DXTn) into account.
 */
static UINT calculate_row_byte_length (int width, int num_color_channels, D3DFORMAT tex_format)
{
    UINT source_row_byte_length = 0;

    // check for compressed textures and adjust source_row_byte_length and
    // source_format accordingly
    switch (tex_format) {
      case D3DFMT_DXT1:
      case D3DFMT_ATI1:
          // for dxt1 compressed textures, the row_byte_lenght is "the width
          // of one row of cells, in bytes" cells are 4 pixels wide, take up 8
          // bytes, and at least 1 cell has to be there.
          source_row_byte_length = max(1,width / 4)*8;
        break;
      case D3DFMT_DXT2:
      case D3DFMT_DXT3:
      case D3DFMT_DXT4:
      case D3DFMT_DXT5:
      case D3DFMT_ATI2:
          // analogue as above, but cells take up 16 bytes
          source_row_byte_length = max(1,width / 4)*16;
        break;
      default:
        // no known compression format.. usual calculation
        source_row_byte_length = width*num_color_channels;
        break;
    }
    return source_row_byte_length;
}

/**
 * Called from fill_d3d_texture_pixels, this function fills a single mipmap
 * with texture data.  Takes care of all necessary conversions and error
 * handling.
 */
HRESULT DXTextureContext9::fill_d3d_texture_mipmap_pixels(int mip_level, int depth_index, D3DFORMAT source_format)
{
  // This whole function was refactored out of fill_d3d_texture_pixels to make
  // the code more readable and to avoid code duplication.
  IDirect3DSurface9 *mip_surface = nullptr;
  bool using_temp_buffer = false;
  HRESULT hr = E_FAIL;
  CPTA_uchar image = get_texture()->get_ram_mipmap_image(mip_level);
  BYTE *pixels = (BYTE*) image.p();
  DWORD width  = (DWORD) get_texture()->get_expected_mipmap_x_size(mip_level);
  DWORD height = (DWORD) get_texture()->get_expected_mipmap_y_size(mip_level);
  int component_width = get_texture()->get_component_width();

  size_t page_size = get_texture()->get_expected_ram_mipmap_page_size(mip_level);
  size_t view_size;
  vector_uchar clear_data;
  if (page_size > 0) {
    if (image.is_null()) {
      // Make an image, filled with the texture's clear color.
      image = get_texture()->make_ram_mipmap_image(mip_level);
      nassertr(!image.is_null(), E_FAIL);
      pixels = (BYTE *)image.p();
    }
    view_size = image.size();
    pixels += view_size * get_view();
    pixels += page_size * depth_index;
  } else {
    // This is a 0x0 texture, which gets loaded as though it were 1x1.
    width = 1;
    height = 1;
    clear_data = get_texture()->get_clear_data();
    pixels = clear_data.data();
    view_size = clear_data.size();
  }

  if (get_texture()->get_texture_type() == Texture::TT_cube_map) {
    nassertr(IS_VALID_PTR(_d3d_cube_texture), E_FAIL);
    hr = _d3d_cube_texture->GetCubeMapSurface((D3DCUBEMAP_FACES)depth_index, mip_level, &mip_surface);
  } else {
    nassertr(IS_VALID_PTR(_d3d_2d_texture), E_FAIL);
    hr = _d3d_2d_texture->GetSurfaceLevel(mip_level, &mip_surface);
  }

  if (FAILED(hr)) {
    dxgsg9_cat.error()
      << "FillDDTextureMipmapPixels failed for " << get_texture()->get_name()
      << ", GetSurfaceLevel failed" << D3DERRORSTRING(hr);
    return E_FAIL;
  }

  RECT source_size;
  source_size.left = source_size.top = 0;
  source_size.right = width;
  source_size.bottom = height;

  UINT source_row_byte_length = calculate_row_byte_length(width, get_texture()->get_num_components(), source_format);

  DWORD mip_filter;
  // need filtering if size changes, (also if bitdepth reduced (need
  // dithering)??)
  mip_filter = D3DX_FILTER_LINEAR ; //| D3DX_FILTER_DITHER;  //dithering looks ugly on i810 for 4444 textures

  if (Texture::is_srgb(get_texture()->get_format())) {
    mip_filter |= D3DX_FILTER_SRGB;
  }

  // D3DXLoadSurfaceFromMemory will load black luminance and we want full
  // white, so convert to explicit luminance-alpha format
  if (_d3d_format == D3DFMT_A8) {
    // alloc buffer for explicit D3DFMT_A8L8
    USHORT *temp_buffer = new USHORT[width * height];
    if (!IS_VALID_PTR(temp_buffer)) {
      dxgsg9_cat.error()
        << "FillDDTextureMipmapPixels couldnt alloc mem for temp pixbuf!\n";
      goto exit_FillMipmapSurf;
    }
    using_temp_buffer = true;

    USHORT *out_pixels = temp_buffer;
    BYTE *source_pixels = pixels + component_width - 1;
    for (UINT y = 0; y < height; y++) {
      for (UINT x = 0; x < width; x++, source_pixels += component_width, out_pixels++) {
        // add full white, which is our interpretation of alpha-only (similar
        // to default adding full opaque alpha 0xFF to RGB-only textures)
        *out_pixels = ((*source_pixels) << 8 ) | 0xFF;
      }
    }

    source_format = D3DFMT_A8L8;
    source_row_byte_length = width * sizeof(USHORT);
    pixels = (BYTE*)temp_buffer;
  }
  else if (component_width != 1) {
    // Convert from 16-bit per channel (or larger) format down to 8-bit per
    // channel.  This throws away precision in the original image, but dx8
    // doesn't support high-precision images anyway.

    int num_components = get_texture()->get_num_components();
    int num_pixels = width * height * num_components;
    BYTE *temp_buffer = new BYTE[num_pixels];
    if (!IS_VALID_PTR(temp_buffer)) {
      dxgsg9_cat.error() << "FillDDTextureMipmapPixels couldnt alloc mem for temp pixbuf!\n";
      goto exit_FillMipmapSurf;
    }
    using_temp_buffer = true;

    BYTE *source_pixels = pixels + component_width - 1;
    for (int i = 0; i < num_pixels; i++) {
      temp_buffer[i] = *source_pixels;
      source_pixels += component_width;
    }
    pixels = (BYTE*)temp_buffer;
  }

  // filtering may be done here if texture if targetsize != origsize
#ifdef DO_PSTATS
  GraphicsStateGuardian::_data_transferred_pcollector.add_level(source_row_byte_length * height);
#endif
  if (source_format == D3DFMT_ATI1 || source_format == D3DFMT_ATI2) {
    // These formats are not supported by D3DXLoadSurfaceFromMemory.
    D3DLOCKED_RECT rect;
    _d3d_2d_texture->LockRect(mip_level, &rect, 0, D3DLOCK_DISCARD);

    unsigned char *dest = (unsigned char *)rect.pBits;
    memcpy(dest, pixels, view_size);

    _d3d_2d_texture->UnlockRect(mip_level);

  } else {
    hr = D3DXLoadSurfaceFromMemory
      (mip_surface, nullptr, nullptr, (LPCVOID)pixels,
        source_format, source_row_byte_length, nullptr,
        &source_size, mip_filter, (D3DCOLOR)0x0);
    if (FAILED(hr)) {
      dxgsg9_cat.error()
        << "FillDDTextureMipmapPixels failed for " << get_texture()->get_name()
        << ", mip_level " << mip_level
        << ", D3DXLoadSurfFromMem failed" << D3DERRORSTRING(hr);
    }
  }

exit_FillMipmapSurf:
  if (using_temp_buffer) {
    SAFE_DELETE_ARRAY(pixels);
  }

  RELEASE(mip_surface, dxgsg9, "FillDDTextureMipmapPixels MipSurface texture ptr", RELEASE_ONCE);
  return hr;
}

/**
 *
 */
HRESULT DXTextureContext9::
fill_d3d_texture_pixels(DXScreenData &scrn, bool compress_texture) {
  IDirect3DDevice9 *device = scrn._d3d_device;
  Texture *tex = get_texture();
  nassertr(IS_VALID_PTR(tex), E_FAIL);
  if (tex->get_texture_type() == Texture::TT_3d_texture) {
    return fill_d3d_volume_texture_pixels(scrn);
  }

  HRESULT hr = E_FAIL;

  CPTA_uchar image;
  Texture::CompressionMode image_compression = Texture::CM_off;
  if (compress_texture) {
    // If we are to be compressing this texture, accept a pre-compressed ram
    // image if it is already so.
    image = tex->get_ram_image();
    if (!image.is_null()) {
      image_compression = tex->get_ram_image_compression();
    }
  } else {
    // If we are not to be compressing this texture, we can only accept an
    // uncompressed ram image.  Ask the texture to give us one, so that
    // there's no danger of accidentally getting a pre-compressed image.
    image = tex->get_uncompressed_ram_image();
  }

  if (image.is_null()) {
    // The texture doesn't have an image to load.  That's ok; it might be a
    // texture we've rendered to by frame buffer operations or something.
    if (tex->get_render_to_texture()) {
      HRESULT result;

      if (_d3d_2d_texture) {
        // clear render to texture
        IDirect3DSurface9 *surface;

        result = _d3d_2d_texture -> GetSurfaceLevel (0, &surface);
        if (result == D3D_OK) {
          D3DSURFACE_DESC surface_description;

          if (surface -> GetDesc (&surface_description) == D3D_OK) {
            IDirect3DSurface9 *current_render_target;

            if (device -> GetRenderTarget (0, &current_render_target) == D3D_OK) {
              IDirect3DSurface9 *depth_stencil_surface;

              // turn off depth stencil when clearing texture if it exists
              depth_stencil_surface = 0;
              if (device -> GetDepthStencilSurface (&depth_stencil_surface) == D3D_OK) {
                if (device -> SetDepthStencilSurface (nullptr) == D3D_OK) {

                }
              }

              if (device -> SetRenderTarget (0, surface)  == D3D_OK) {
                DWORD flags;
                D3DCOLOR color;

                LColor scaled = tex->get_clear_color().fmin(LColor(1)).fmax(LColor::zero());
                scaled *= 255;
                color = D3DCOLOR_RGBA((int)scaled[0], (int)scaled[1], (int)scaled[2], (int)scaled[3]);
                flags = D3DCLEAR_TARGET;
                if (device -> Clear (0, nullptr, flags, color, 0.0f, 0) == D3D_OK) {
                }
              }

              // restore depth stencil
              if (depth_stencil_surface) {
                device -> SetDepthStencilSurface (depth_stencil_surface);
                depth_stencil_surface -> Release();
              }

              // restore render target
              device -> SetRenderTarget (0, current_render_target);
              current_render_target -> Release();
            }
          }

          surface -> Release();
        }
      }

      return S_OK;
    }
  }
  //nassertr(IS_VALID_PTR((BYTE*)image.p()), E_FAIL);
  nassertr(IS_VALID_PTR(_d3d_texture), E_FAIL);

  PStatTimer timer(GraphicsStateGuardian::_load_texture_pcollector);

  DWORD orig_depth = (DWORD) tex->get_z_size();
  D3DFORMAT source_format = _d3d_format;

  // check for compressed textures and adjust source_format accordingly
  switch (image_compression) {
  case Texture::CM_dxt1:
    source_format = D3DFMT_DXT1;
    break;
  case Texture::CM_dxt2:
    source_format = D3DFMT_DXT2;
    break;
  case Texture::CM_dxt3:
    source_format = D3DFMT_DXT3;
    break;
  case Texture::CM_dxt4:
    source_format = D3DFMT_DXT4;
    break;
  case Texture::CM_dxt5:
    source_format = D3DFMT_DXT5;
    break;
  case Texture::CM_rgtc:
    if (tex->get_num_components() == 1) {
      source_format = D3DFMT_ATI1;
    } else {
      source_format = D3DFMT_ATI2;
    }
    break;
  default:
    // no known compression format.. no adjustment
    break;
  }

  for (unsigned int di = 0; di < orig_depth; di++) {

    // fill top level mipmap
    hr = fill_d3d_texture_mipmap_pixels(0, di, source_format);
    if (FAILED(hr)) {
      return hr; // error message was already output in fill_d3d_texture_mipmap_pixels
    }

    if (_has_mipmaps) {
      // if we have pre-calculated mipmap levels, use them, otherwise generate
      // on the fly
      int miplevel_count = _d3d_texture->GetLevelCount();
      if (miplevel_count <= tex->get_num_loadable_ram_mipmap_images()) {
        dxgsg9_cat.debug()
          << "Using pre-calculated mipmap levels for texture  " << tex->get_name() << "\n";

        for (int mip_level = 1; mip_level < miplevel_count; ++mip_level) {
          hr = fill_d3d_texture_mipmap_pixels(mip_level, di, source_format);
          if (FAILED(hr)) {
            return hr; // error message was already output in fill_d3d_texture_mipmap_pixels
          }
        }
      }
      else {
        // mipmaps need to be generated, either use autogen or d3dx functions

        if (_managed == false && scrn._supports_automatic_mipmap_generation) {
          if (false)
          {
            // hr = _d3d_texture -> SetAutoGenFilterType
            // (D3DTEXF_PYRAMIDALQUAD); hr = _d3d_texture ->
            // SetAutoGenFilterType (D3DTEXF_GAUSSIANQUAD); hr = _d3d_texture
            // -> SetAutoGenFilterType (D3DTEXF_ANISOTROPIC);
            hr = _d3d_texture -> SetAutoGenFilterType (D3DTEXF_LINEAR);
            if (FAILED(hr)) {
              dxgsg9_cat.error() << "SetAutoGenFilterType failed " << D3DERRORSTRING(hr);
            }

            _d3d_texture -> GenerateMipSubLevels ( );
          }
        }
        else {
          DWORD mip_filter_flags;
          if (!dx_use_triangle_mipgen_filter) {
            mip_filter_flags = D3DX_FILTER_BOX;
          } else {
            mip_filter_flags = D3DX_FILTER_TRIANGLE;
          }

          if (Texture::is_srgb(tex->get_format())) {
            mip_filter_flags |= D3DX_FILTER_SRGB;
          }

          // mip_filter_flags |= D3DX_FILTER_DITHER;
          hr = D3DXFilterTexture(_d3d_texture, nullptr, 0,
                                mip_filter_flags);

          if (FAILED(hr)) {
            dxgsg9_cat.error()
              << "FillDDSurfaceTexturePixels failed for " << tex->get_name()
              << ", D3DXFilterTex failed" << D3DERRORSTRING(hr);
          }
        }
      }
    }
  }

  return hr;
}


/**
 *
 */
HRESULT DXTextureContext9::
fill_d3d_volume_texture_pixels(DXScreenData &scrn) {
  Texture *tex = get_texture();
  HRESULT hr = E_FAIL;
  nassertr(IS_VALID_PTR(tex), E_FAIL);

  CPTA_uchar image;
  if (scrn._dxgsg9->get_supports_compressed_texture()) {
    image = tex->get_ram_image();
  } else {
    image = tex->get_uncompressed_ram_image();
  }

  Texture::CompressionMode image_compression;
  if (image.is_null()) {
    image_compression = Texture::CM_off;
  } else {
    image_compression = tex->get_ram_image_compression();
  }

  if (!scrn._dxgsg9->get_supports_compressed_texture_format(image_compression)) {
    image = tex->get_uncompressed_ram_image();
    image_compression = Texture::CM_off;
  }

  if (image.is_null()) {
    // The texture doesn't have an image to load.  That's ok; it might be a
    // texture we've rendered to by frame buffer operations or something.
    return S_OK;
  }

  PStatTimer timer(GraphicsStateGuardian::_load_texture_pcollector);

  nassertr(IS_VALID_PTR(_d3d_texture), E_FAIL);
  nassertr(tex->get_texture_type() == Texture::TT_3d_texture, E_FAIL);

  DWORD orig_width  = (DWORD) tex->get_x_size();
  DWORD orig_height = (DWORD) tex->get_y_size();
  DWORD orig_depth = (DWORD) tex->get_z_size();
  DWORD num_color_channels = tex->get_num_components();
  D3DFORMAT source_format = _d3d_format;
  BYTE *image_pixels = (BYTE*)image.p();
  int component_width = tex->get_component_width();

  nassertr(IS_VALID_PTR(image_pixels), E_FAIL);

  size_t view_size = tex->get_ram_mipmap_view_size(0);
  image_pixels += view_size * get_view();

  IDirect3DVolume9 *mip_level_0 = nullptr;
  bool using_temp_buffer = false;
  BYTE *pixels = image_pixels;

  nassertr(IS_VALID_PTR(_d3d_volume_texture), E_FAIL);
  hr = _d3d_volume_texture->GetVolumeLevel(0, &mip_level_0);

  if (FAILED(hr)) {
    dxgsg9_cat.error()
      << "FillDDSurfaceTexturePixels failed for " << tex->get_name()
      << ", GetSurfaceLevel failed" << D3DERRORSTRING(hr);
    return E_FAIL;
  }

  D3DBOX source_size;
  source_size.Left = source_size.Top = source_size.Front = 0;
  source_size.Right = orig_width;
  source_size.Bottom = orig_height;
  source_size.Back = orig_depth;

  UINT source_row_byte_length = orig_width * num_color_channels;
  UINT source_page_byte_length = orig_height * source_row_byte_length;

  DWORD level_0_filter, mip_filter_flags;
  using_temp_buffer = false;

  // need filtering if size changes, (also if bitdepth reduced (need
  // dithering)??)
  level_0_filter = D3DX_FILTER_LINEAR ; //| D3DX_FILTER_DITHER;  //dithering looks ugly on i810 for 4444 textures

  if (Texture::is_srgb(tex->get_format())) {
    level_0_filter |= D3DX_FILTER_SRGB;
  }

  // D3DXLoadSurfaceFromMemory will load black luminance and we want full
  // white, so convert to explicit luminance-alpha format
  if (_d3d_format == D3DFMT_A8) {
    // alloc buffer for explicit D3DFMT_A8L8
    USHORT *temp_buffer = new USHORT[orig_width * orig_height * orig_depth];
    if (!IS_VALID_PTR(temp_buffer)) {
      dxgsg9_cat.error()
        << "FillDDSurfaceTexturePixels couldnt alloc mem for temp pixbuf!\n";
      goto exit_FillDDSurf;
    }
    using_temp_buffer = true;

    USHORT *out_pixels = temp_buffer;
    BYTE *source_pixels = pixels + component_width - 1;
    for (UINT z = 0; z < orig_depth; z++) {
      for (UINT y = 0; y < orig_height; y++) {
        for (UINT x = 0;
             x < orig_width;
             x++, source_pixels += component_width, out_pixels++) {
          // add full white, which is our interpretation of alpha-only
          // (similar to default adding full opaque alpha 0xFF to RGB-only
          // textures)
          *out_pixels = ((*source_pixels) << 8 ) | 0xFF;
        }
      }
    }

    source_format = D3DFMT_A8L8;
    source_row_byte_length = orig_width * sizeof(USHORT);
    source_page_byte_length = orig_height * source_row_byte_length;
    pixels = (BYTE*)temp_buffer;

  } else if (component_width != 1) {
    // Convert from 16-bit per channel (or larger) format down to 8-bit per
    // channel.  This throws away precision in the original image, but dx8
    // doesn't support high-precision images anyway.

    int num_components = tex->get_num_components();
    int num_pixels = orig_width * orig_height * orig_depth * num_components;
    BYTE *temp_buffer = new BYTE[num_pixels];
    if (!IS_VALID_PTR(temp_buffer)) {
      dxgsg9_cat.error() << "FillDDSurfaceTexturePixels couldnt alloc mem for temp pixbuf!\n";
      goto exit_FillDDSurf;
    }
    using_temp_buffer = true;

    BYTE *source_pixels = pixels + component_width - 1;
    for (int i = 0; i < num_pixels; i++) {
      temp_buffer[i] = *source_pixels;
      source_pixels += component_width;
    }
    pixels = (BYTE*)temp_buffer;
  }


  // filtering may be done here if texture if targetsize != origsize
#ifdef DO_PSTATS
  GraphicsStateGuardian::_data_transferred_pcollector.add_level(source_page_byte_length * orig_depth);
#endif
  hr = D3DXLoadVolumeFromMemory
    (mip_level_0, nullptr, nullptr, (LPCVOID)pixels,
     source_format, source_row_byte_length, source_page_byte_length,
     nullptr,
     &source_size, level_0_filter, (D3DCOLOR)0x0);
  if (FAILED(hr)) {
    dxgsg9_cat.error()
      << "FillDDSurfaceTexturePixels failed for " << tex->get_name()
      << ", D3DXLoadVolumeFromMem failed" << D3DERRORSTRING(hr);
    goto exit_FillDDSurf;
  }

  if (_has_mipmaps) {
    if (!dx_use_triangle_mipgen_filter) {
      mip_filter_flags = D3DX_FILTER_BOX;
    } else {
      mip_filter_flags = D3DX_FILTER_TRIANGLE;
    }

    if (Texture::is_srgb(tex->get_format())) {
      mip_filter_flags |= D3DX_FILTER_SRGB;
    }

    // mip_filter_flags| = D3DX_FILTER_DITHER;

    hr = D3DXFilterTexture(_d3d_texture, nullptr, 0,
                           mip_filter_flags);
    if (FAILED(hr)) {
      dxgsg9_cat.error()
        << "FillDDSurfaceTexturePixels failed for " << tex->get_name()
        << ", D3DXFilterTex failed" << D3DERRORSTRING(hr);
      goto exit_FillDDSurf;
    }
  }

 exit_FillDDSurf:
  if (using_temp_buffer) {
    SAFE_DELETE_ARRAY(pixels);
  }
  RELEASE(mip_level_0, dxgsg9, "FillDDSurf MipLev0 texture ptr", RELEASE_ONCE);
  return hr;
}


/**
 * Returns the largest power of 2 less than or equal to value.
 */
int DXTextureContext9::
down_to_power_2(int value) {
  int x = 1;
  while ((x << 1) <= value) {
    x = (x << 1);
  }
  return x;
}

/**
 * Maps from the Texture's Format symbols to bpp.  Returns # of alpha bits.
 * Note: Texture's format indicates REQUESTED final format, not the stored
 * format, which is indicated by pixelbuffer type
 */
unsigned int DXTextureContext9::
get_bits_per_pixel(Texture::Format format, int *alphbits) {
  *alphbits = 0;      // assume no alpha bits
  switch(format) {
  case Texture::F_alpha:
    *alphbits = 8;
  case Texture::F_color_index:
  case Texture::F_red:
  case Texture::F_green:
  case Texture::F_blue:
  case Texture::F_rgb332:
    return 8;
  case Texture::F_luminance_alphamask:
    *alphbits = 1;
    return 16;
  case Texture::F_luminance_alpha:
    *alphbits = 8;
    return 16;
  case Texture::F_luminance:
    return 8;
  case Texture::F_rgba4:
    *alphbits = 4;
    return 16;
  case Texture::F_rgba5:
    *alphbits = 1;
    return 16;
  case Texture::F_depth_component:
  case Texture::F_depth_component16:
    return 16;
  case Texture::F_depth_stencil:
  case Texture::F_depth_component24:
    return 24;
  case Texture::F_depth_component32:
    return 32;
  case Texture::F_rgb5:
    return 16;
  case Texture::F_rgb8:
  case Texture::F_rgb:
    return 24;
  case Texture::F_rgba8:
  case Texture::F_rgba:
    *alphbits = 8;
    return 32;
  case Texture::F_rgbm:
    *alphbits = 1;
    return 32;
  case Texture::F_rgb12:
    return 36;
  case Texture::F_rgba12:
    *alphbits = 12;
    return 48;
  case Texture::F_rgba16:
    *alphbits = 16;
    return 64;
  case Texture::F_rgba32:
    *alphbits = 32;
    return 128;

  case Texture::F_srgb:
    return 24;
  case Texture::F_srgb_alpha:
    *alphbits = 8;
    return 32;
  case Texture::F_sluminance:
    return 8;
  case Texture::F_sluminance_alpha:
    *alphbits = 8;
    return 16;
  }
  return 8;
}

/**
 * Determines bytes per pixel from D3DFORMAT.
 */
PN_stdfloat DXTextureContext9::
d3d_format_to_bytes_per_pixel (D3DFORMAT format)
{
  PN_stdfloat bytes_per_pixel;

  bytes_per_pixel = 0.0f;
  switch (format)
  {
    case D3DFMT_R3G3B2:
    case D3DFMT_A8:
    case D3DFMT_A8P8:
    case D3DFMT_P8:
    case D3DFMT_L8:
    case D3DFMT_A4L4:
      bytes_per_pixel = 1.0f;
      break;

    case D3DFMT_R16F:
    case D3DFMT_CxV8U8:
    case D3DFMT_V8U8:
    case D3DFMT_R5G6B5:
    case D3DFMT_X1R5G5B5:
    case D3DFMT_A1R5G5B5:
    case D3DFMT_A4R4G4B4:
    case D3DFMT_L16:
    case D3DFMT_A8L8:
    case D3DFMT_A8R3G3B2:
    case D3DFMT_X4R4G4B4:
    case D3DFMT_D16:
    case (D3DFORMAT)MAKEFOURCC('D', 'F', '1', '6'):
      bytes_per_pixel = 2.0f;
      break;

    case D3DFMT_R8G8B8:
      bytes_per_pixel = 3.0f;
      break;

    case D3DFMT_G16R16F:
    case D3DFMT_Q8W8V8U8:
    case D3DFMT_V16U16:
    case D3DFMT_R32F:
    case D3DFMT_A8R8G8B8:
    case D3DFMT_X8R8G8B8:
    case D3DFMT_A2B10G10R10:
    case D3DFMT_A8B8G8R8:
    case D3DFMT_X8B8G8R8:
    case D3DFMT_G16R16:
    case D3DFMT_A2R10G10B10:
    case D3DFMT_D24X8:
    case D3DFMT_D24S8:
    case D3DFMT_D32:
    case (D3DFORMAT)MAKEFOURCC('D', 'F', '2', '4'):
    case D3DFMT_INTZ:
      bytes_per_pixel = 4.0f;
      break;

    case D3DFMT_G32R32F:
    case D3DFMT_A16B16G16R16F:
    case D3DFMT_Q16W16V16U16:
    case D3DFMT_A16B16G16R16:
      bytes_per_pixel = 8.0f;
      break;

    case D3DFMT_A32B32G32R32F:
      bytes_per_pixel = 16.0f;
      break;

    case D3DFMT_DXT1:
    case D3DFMT_ATI1:
      bytes_per_pixel = 0.5f;
      break;
    case D3DFMT_DXT2:
    case D3DFMT_DXT3:
    case D3DFMT_DXT4:
    case D3DFMT_DXT5:
    case D3DFMT_ATI2:
      bytes_per_pixel = 1.0f;
      break;
  }

  return bytes_per_pixel;
}
