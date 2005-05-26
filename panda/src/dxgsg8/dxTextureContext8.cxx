// Filename: dxTextureContext8.cxx
// Created by:  georges (02Feb02)
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

#include "dxTextureContext8.h"
#include "config_dxgsg8.h"
#include "dxGraphicsStateGuardian8.h"
#include "pStatTimer.h"
#include <d3dx8tex.h>
#include <assert.h>
#include <time.h>

TypeHandle DXTextureContext8::_type_handle;

static const DWORD g_LowByteMask = 0x000000FF;

////////////////////////////////////////////////////////////////////
//     Function: DXTextureContext8::get_bits_per_pixel
//       Access: Protected
//  Description: Maps from the Texture's Format symbols
//               to bpp.  returns # of alpha bits
//               Note: Texture's format indicates REQUESTED final format,
//                     not the stored format, which is indicated by pixelbuffer type
////////////////////////////////////////////////////////////////////
unsigned int DXTextureContext8::
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
  case Texture::F_rgb5:
    return 16;
  case Texture::F_rgb8:
  case Texture::F_rgb:
    return 24;
  case Texture::F_rgba8:
  case Texture::F_rgba:
  case Texture::F_rgbm:
    if (format == Texture::F_rgbm)   // does this make any sense?
      *alphbits = 1;
    else *alphbits = 8;
    return 32;
  case Texture::F_rgb12:
    return 36;
  case Texture::F_rgba12:
    *alphbits = 12;
    return 48;
  }
  return 8;
}

// still need custom conversion since d3d/d3dx has no way to convert arbitrary fmt to ARGB in-memory user buffer
HRESULT
d3d_surface_to_texture(RECT &source_rect, IDirect3DSurface8 *d3d_surface, Texture *result) {
  // copies source_rect in pD3DSurf to upper left of texture
  HRESULT hr;
  DWORD dwNumComponents = result->get_num_components();

  nassertr(result->get_component_width() == sizeof(BYTE), E_FAIL);   // cant handle anything else now
  nassertr(result->get_component_type() == Texture::T_unsigned_byte, E_FAIL);   // cant handle anything else now
  nassertr((dwNumComponents == 3) || (dwNumComponents == 4), E_FAIL);  // cant handle anything else now
  nassertr(IS_VALID_PTR(d3d_surface), E_FAIL);

  BYTE *pbuf = result->modify_ram_image().p();

  if (IsBadWritePtr(d3d_surface, sizeof(DWORD))) {
    dxgsg8_cat.error() << "d3d_surface_to_texture failed: bad pD3DSurf ptr value (" << ((void*)d3d_surface) << ")\n";
    exit(1);
  }

  DWORD dwXWindowOffset, dwYWindowOffset;
  DWORD dwCopyWidth, dwCopyHeight;

  D3DLOCKED_RECT LockedRect;
  D3DSURFACE_DESC SurfDesc;

  hr = d3d_surface->GetDesc(&SurfDesc);

  dwXWindowOffset = source_rect.left, dwYWindowOffset = source_rect.top;
  dwCopyWidth = RECT_XSIZE(source_rect);
  dwCopyHeight = RECT_YSIZE(source_rect);

  //make sure there's enough space in the texture, its size must match (especially xsize)
  // or scanlines will be too long

  if (!((dwCopyWidth == result->get_x_size()) && (dwCopyHeight <= (DWORD)result->get_y_size()))) {
    dxgsg8_cat.error() << "d3d_surface_to_texture, Texture size too small to hold display surface!\n";
    nassertr(false, E_FAIL);
    return E_FAIL;
  }

  hr = d3d_surface->LockRect(&LockedRect, (CONST RECT*)NULL, (D3DLOCK_READONLY | D3DLOCK_NO_DIRTY_UPDATE /* | D3DLOCK_NOSYSLOCK */));
  if (FAILED(hr)) {
    dxgsg8_cat.error() << "d3d_surface_to_texture LockRect() failed!" << D3DERRORSTRING(hr);
    return hr;
  }

  // ones not listed not handled yet
  nassertr((SurfDesc.Format == D3DFMT_A8R8G8B8) ||
           (SurfDesc.Format == D3DFMT_X8R8G8B8) ||
           (SurfDesc.Format == D3DFMT_R8G8B8) ||
           (SurfDesc.Format == D3DFMT_R5G6B5) ||
           (SurfDesc.Format == D3DFMT_X1R5G5B5) ||
           (SurfDesc.Format == D3DFMT_A1R5G5B5) ||
           (SurfDesc.Format == D3DFMT_A4R4G4B4), E_FAIL);

  //pbuf contains raw ARGB in Texture byteorder

  DWORD BytePitch = LockedRect.Pitch;
  BYTE* pSurfBytes = (BYTE*)LockedRect.pBits;

  // writes out last line in DDSurf first in PixelBuf, so Y line order precedes inversely

  if (dxgsg8_cat.is_debug()) {
    dxgsg8_cat.debug()
      << "d3d_surface_to_texture converting " << D3DFormatStr(SurfDesc.Format) << "bpp DDSurf to "
      <<  dwNumComponents << "-channel panda Texture\n";
  }

  DWORD *pDstWord = (DWORD *) pbuf;
  BYTE *pDstByte = (BYTE *) pbuf;

  switch(SurfDesc.Format) {
  case D3DFMT_A8R8G8B8:
  case D3DFMT_X8R8G8B8: {
    if (dwNumComponents == 4) {
      DWORD *pSrcWord;
      BYTE *pDstLine = (BYTE*)pDstWord;

      pSurfBytes += BytePitch*(dwYWindowOffset+dwCopyHeight-1);
      for(DWORD y = 0; y<dwCopyHeight; y++, pSurfBytes -= BytePitch) {
        pSrcWord = ((DWORD*)pSurfBytes)+dwXWindowOffset;
        memcpy(pDstLine, pSrcWord, BytePitch);
        pDstLine += BytePitch;
      }
    } else {
      // 24bpp texture case (numComponents == 3)
      DWORD *pSrcWord;
      pSurfBytes += BytePitch*(dwYWindowOffset+dwCopyHeight-1);
      for(DWORD y = 0; y<dwCopyHeight; y++, pSurfBytes -= BytePitch) {
        pSrcWord = ((DWORD*)pSurfBytes)+dwXWindowOffset;

        for(DWORD x = 0; x<dwCopyWidth; x++, pSrcWord++) {
          BYTE r, g, b;
          DWORD dwPixel = *pSrcWord;

          r = (BYTE)((dwPixel>>16) & g_LowByteMask);
          g = (BYTE)((dwPixel>> 8) & g_LowByteMask);
          b = (BYTE)((dwPixel    ) & g_LowByteMask);

          *pDstByte += b;
          *pDstByte += g;
          *pDstByte += r;
        }
      }
    }
    break;
  }

  case D3DFMT_R8G8B8: {
    BYTE *pSrcByte;
    pSurfBytes += BytePitch*(dwYWindowOffset+dwCopyHeight-1);

    if (dwNumComponents == 4) {
      for(DWORD y = 0; y<dwCopyHeight; y++, pSurfBytes -= BytePitch) {
        pSrcByte = pSurfBytes+dwXWindowOffset*3*sizeof(BYTE);
        for(DWORD x = 0; x<dwCopyWidth; x++, pDstWord++) {
          DWORD r, g, b;

          b = *pSrcByte++;
          g = *pSrcByte++;
          r = *pSrcByte++;

          *pDstWord = 0xFF000000 | (r << 16) | (g << 8) | b;
        }
      }
    } else {
      // 24bpp texture case (numComponents == 3)
      for(DWORD y = 0; y<dwCopyHeight; y++, pSurfBytes -= BytePitch) {
        pSrcByte = pSurfBytes+dwXWindowOffset*3*sizeof(BYTE);
        memcpy(pDstByte, pSrcByte, BytePitch);
        pDstByte += BytePitch;
      }
    }
    break;
  }

  case D3DFMT_R5G6B5:
  case D3DFMT_X1R5G5B5:
  case D3DFMT_A1R5G5B5:
  case D3DFMT_A4R4G4B4: {
    WORD  *pSrcWord;
    // handle 0555, 1555, 0565, 4444 in same loop

    BYTE redshift, greenshift, blueshift;
    DWORD redmask, greenmask, bluemask;

    if (SurfDesc.Format == D3DFMT_R5G6B5) {
      redshift = (11-3);
      redmask = 0xF800;
      greenmask = 0x07E0;
      greenshift = (5-2);
      bluemask = 0x001F;
      blueshift = 3;
    } else if (SurfDesc.Format == D3DFMT_A4R4G4B4) {
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

    pSurfBytes += BytePitch*(dwYWindowOffset+dwCopyHeight-1);
    if (dwNumComponents == 4) {
      // Note: these 16bpp loops ignore input alpha completely (alpha
      // is set to fully opaque in texture!)

      // if we need to capture alpha, probably need to make separate
      // loops for diff 16bpp fmts for best speed

      for(DWORD y = 0; y<dwCopyHeight; y++, pSurfBytes -= BytePitch) {
        pSrcWord = ((WORD*)pSurfBytes)+dwXWindowOffset;
        for(DWORD x = 0; x<dwCopyWidth; x++, pSrcWord++, pDstWord++) {
          WORD dwPixel = *pSrcWord;
          BYTE r, g, b;

          b = (dwPixel & bluemask) << blueshift;
          g = (dwPixel & greenmask) >> greenshift;
          r = (dwPixel & redmask) >> redshift;

          // alpha is just set to 0xFF

          *pDstWord = 0xFF000000 | (r << 16) | (g << 8) | b;
        }
      }
    } else {
      // 24bpp texture case (numComponents == 3)
      for(DWORD y = 0; y<dwCopyHeight; y++, pSurfBytes -= BytePitch) {
        pSrcWord = ((WORD*)pSurfBytes)+dwXWindowOffset;
        for(DWORD x = 0; x<dwCopyWidth; x++, pSrcWord++) {
          WORD dwPixel = *pSrcWord;
          BYTE r, g, b;

          b = (dwPixel & bluemask) << blueshift;
          g = (dwPixel & greenmask) >> greenshift;
          r = (dwPixel & redmask) >> redshift;

          *pDstByte += b;
          *pDstByte += g;
          *pDstByte += r;
        }
      }
    }
    break;
  }

  default:
    dxgsg8_cat.error() << "d3d_surface_to_texture: unsupported D3DFORMAT!\n";
  }

  d3d_surface->UnlockRect();
  return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CreateTexture()
// Desc: Use panda texture's pixelbuffer to create a texture for the specified device.
//       This code gets the attributes of the texture from the bitmap, creates the
//       texture, and then copies the bitmap into the texture.
//-----------------------------------------------------------------------------
IDirect3DTexture8 *DXTextureContext8::
CreateTexture(DXScreenData &scrn) {
  HRESULT hr;
  int cNumAlphaBits;     //  number of alpha bits in texture pixfmt
  D3DFORMAT TargetPixFmt = D3DFMT_UNKNOWN;
  bool bNeedLuminance = false;

  assert(IS_VALID_PTR(_texture));

  // bpp indicates requested fmt, not texture fmt
  DWORD target_bpp = get_bits_per_pixel(_texture->get_format(), &cNumAlphaBits);
  DWORD cNumColorChannels = _texture->get_num_components();

  //PRINT_REFCNT(dxgsg8, scrn.pD3D8);

  DWORD dwOrigWidth  = (DWORD)_texture->get_x_size();
  DWORD dwOrigHeight = (DWORD)_texture->get_y_size();

  if ((_texture->get_format() == Texture::F_luminance_alpha)||
     (_texture->get_format() == Texture::F_luminance_alphamask) ||
     (_texture->get_format() == Texture::F_luminance)) {
    bNeedLuminance = true;
  }

  if (cNumAlphaBits>0) {
    if (cNumColorChannels == 3) {
      dxgsg8_cat.error() << "ERROR: texture " << _tex->get_name() << " has no inherent alpha channel, but alpha format is requested (that would be wasteful)!\n";
      exit(1);
    }
  }

  _d3d_format = D3DFMT_UNKNOWN;

  // figure out what 'D3DFMT' the Texture is in, so D3DXLoadSurfFromMem knows how to perform copy

  switch(cNumColorChannels) {
  case 1:
    if (cNumAlphaBits>0)
      _d3d_format = D3DFMT_A8;
    else if (bNeedLuminance)
      _d3d_format = D3DFMT_L8;
    break;
  case 2:
    assert(bNeedLuminance && (cNumAlphaBits>0));
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
  assert(_d3d_format != D3DFMT_UNKNOWN);

  DWORD TargetWidth = dwOrigWidth;
  DWORD TargetHeight = dwOrigHeight;

  if (!ISPOW2(dwOrigWidth) || !ISPOW2(dwOrigHeight)) {
    dxgsg8_cat.error() << "ERROR: texture dimensions are not a power of 2 for " << _tex->get_name() << "! Please rescale them so it doesnt have to be done at runtime.\n";
#ifndef NDEBUG
    exit(1);  // want to catch badtexsize errors
#else
    goto error_exit;
#endif
  }

  bool bShrinkOriginal;
  bShrinkOriginal = false;

  if ((dwOrigWidth>scrn.d3dcaps.MaxTextureWidth)||(dwOrigHeight>scrn.d3dcaps.MaxTextureHeight)) {
#ifdef _DEBUG
    dxgsg8_cat.error() << "WARNING: " <<_tex->get_name() << ": Image size exceeds max texture dimensions of (" << scrn.d3dcaps.MaxTextureWidth << ", " << scrn.d3dcaps.MaxTextureHeight << ") !!\n"
                       << "Scaling " << _tex->get_name() << " (" << dwOrigWidth<< ", " <<dwOrigHeight << ") = > (" <<  scrn.d3dcaps.MaxTextureWidth << ", " << scrn.d3dcaps.MaxTextureHeight << ") !\n";
#endif

    if (dwOrigWidth>scrn.d3dcaps.MaxTextureWidth)
      TargetWidth = scrn.d3dcaps.MaxTextureWidth;
    if (dwOrigHeight>scrn.d3dcaps.MaxTextureHeight)
      TargetHeight = scrn.d3dcaps.MaxTextureHeight;
    bShrinkOriginal = true;
  }

  // checks for SQUARE reqmt (nvidia riva128 needs this)
  if ((TargetWidth != TargetHeight) && (scrn.d3dcaps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY)) {
    // assume pow2 textures.   sum exponents, divide by 2 rounding down to get sq size
    int i, width_exp, height_exp;
    for(i = TargetWidth, width_exp = 0; i>1; width_exp++, i >>= 1);
    for(i = TargetHeight, height_exp = 0; i>1; height_exp++, i >>= 1);
    TargetHeight = TargetWidth = 1<<((width_exp+height_exp)>>1);
    bShrinkOriginal = true;

#ifdef _DEBUG
    dxgsg8_cat.debug() << "Scaling " << _tex->get_name() << " (" << dwOrigWidth<< ", " <<dwOrigHeight << ") = > (" << TargetWidth<< ", " << TargetHeight << ") to meet HW square texture reqmt\n";
#endif
  }

  char *szErrorMsg;

  szErrorMsg = "CreateTexture failed: couldn't find compatible device Texture Pixel Format for input texture";

  if (dxgsg8_cat.is_spam())
    dxgsg8_cat.spam() << "CreateTexture handling target bitdepth: " << target_bpp << " alphabits: " << cNumAlphaBits << endl;

  // I could possibly replace some of this logic with D3DXCheckTextureRequirements(), but
  // it wouldnt handle all my specialized low-memory cases perfectly

#define CONVTYPE_STMT

#define CHECK_FOR_FMT(FMT, CONV)  \
                    if (scrn.SupportedTexFmtsMask & FMT##_FLAG) {   \
                        CONVTYPE_STMT;                             \
                        TargetPixFmt = D3DFMT_##FMT;                 \
                        goto found_matching_format; }

  // handle each target bitdepth separately.  might be less confusing to reorg by cNumColorChannels (input type, rather
  // than desired 1st target)
  switch(target_bpp) {

    // IMPORTANT NOTE:
    // target_bpp is REQUESTED bpp, not what exists in the texture array (the texture array contains cNumColorChannels*8bits)

  case 32:
    if (!((cNumColorChannels == 3) || (cNumColorChannels == 4)))
      break; //bail

    if (!dx_force_16bpptextures) {
      if (cNumColorChannels == 4) {
        CHECK_FOR_FMT(A8R8G8B8, Conv32to32);
      } else {
        CHECK_FOR_FMT(A8R8G8B8, Conv24to32);
      }
    }

    if (cNumAlphaBits>0) {
      assert(cNumColorChannels == 4);

      // no 32-bit fmt, look for 16 bit w/alpha  (1-15)

      // 32 bit RGBA was requested, but only 16 bit alpha fmts are avail
      // by default, convert to 4-4-4-4 which has 4-bit alpha for blurry edges
      // if we know tex only needs 1 bit alpha (i.e. for a mask), use 1555 instead

      //                ConversionType ConvTo1 = Conv32to16_4444, ConvTo2 = Conv32to16_1555;
      //                DWORD dwAlphaMask1 = 0xF000, dwAlphaMask2 = 0x8000;
      // assume ALPHAMASK is x8000 and RGBMASK is x7fff to simplify 32->16 conversion
      // this should be true on most cards.

#ifndef FORCE_16bpp_1555
      if (cNumAlphaBits == 1)
#endif
        {
          CHECK_FOR_FMT(A1R5G5B5, Conv32to16_1555);
        }

      // normally prefer 4444 due to better alpha channel resolution
      CHECK_FOR_FMT(A4R4G4B4, Conv32to16_4444);
      CHECK_FOR_FMT(A1R5G5B5, Conv32to16_1555);

      // at this point, bail.  dont worry about converting to non-alpha formats yet,
      // I think this will be a very rare case
      szErrorMsg = "CreateTexture failed: couldn't find compatible Tex DDPIXELFORMAT! no available 16 or 32-bit alpha formats!";
    } else {
      // convert 3 or 4 channel to closest 16bpp color fmt

      if (cNumColorChannels == 3) {
        CHECK_FOR_FMT(R5G6B5, Conv24to16_4444);
        CHECK_FOR_FMT(X1R5G5B5, Conv24to16_X555);
      } else {
        CHECK_FOR_FMT(R5G6B5, Conv32to16_4444);
        CHECK_FOR_FMT(X1R5G5B5, Conv32to16_X555);
      }
    }
    break;

  case 24:
    assert(cNumColorChannels == 3);

    if (!dx_force_16bpptextures) {
      CHECK_FOR_FMT(R8G8B8, Conv24to24);

      // no 24-bit fmt.  look for 32 bit fmt  (note: this is memory-hogging choice
      // instead I could look for memory-conserving 16-bit fmt).

      CHECK_FOR_FMT(X8R8G8B8, Conv24to32);
    }

    // no 24-bit or 32 fmt.  look for 16 bit fmt (higher res 565 1st)
    CHECK_FOR_FMT(R5G6B5, Conv24to16_0565);
    CHECK_FOR_FMT(X1R5G5B5, Conv24to16_X555);
    break;

  case 16:
    if (bNeedLuminance) {
      assert(cNumAlphaBits>0);
      assert(cNumColorChannels == 2);

      CHECK_FOR_FMT(A8L8, ConvLum16to16);

      if (!dx_force_16bpptextures) {
        CHECK_FOR_FMT(A8R8G8B8, ConvLum16to32);
      }

#ifndef FORCE_16bpp_1555
      if (cNumAlphaBits == 1)
#endif
        {
          CHECK_FOR_FMT(A1R5G5B5, ConvLum16to16_1555);
        }

      // normally prefer 4444 due to better alpha channel resolution
      CHECK_FOR_FMT(A4R4G4B4, ConvLum16to16_4444);
      CHECK_FOR_FMT(A1R5G5B5, ConvLum16to16_1555);
    } else {
      assert((cNumColorChannels == 3)||(cNumColorChannels == 4));
      // look for compatible 16bit fmts, if none then give up
      // (dont worry about other bitdepths for 16 bit)
      switch(cNumAlphaBits) {
      case 0:
        if (cNumColorChannels == 3) {
          CHECK_FOR_FMT(R5G6B5, Conv24to16_0565);
          CHECK_FOR_FMT(X1R5G5B5, Conv24to16_X555);
        } else {
          assert(cNumColorChannels == 4);
          // it could be 4 if user asks us to throw away the alpha channel
          CHECK_FOR_FMT(R5G6B5, Conv32to16_0565);
          CHECK_FOR_FMT(X1R5G5B5, Conv32to16_X555);
        }
        break;
      case 1:
        // app specifically requests 1-5-5-5 F_rgba5 case, where you explicitly want 1-5-5-5 fmt, as opposed
        // to F_rgbm, which could use 32bpp ARGB.  fail if this particular fmt not avail.
        assert(cNumColorChannels == 4);
        CHECK_FOR_FMT(X1R5G5B5, Conv32to16_X555);
        break;
      case 4:
        // app specifically requests 4-4-4-4 F_rgba4 case, as opposed to F_rgba, which could use 32bpp ARGB
        assert(cNumColorChannels == 4);
        CHECK_FOR_FMT(A4R4G4B4, Conv32to16_4444);
        break;
      default: assert(0);  // problem in get_bits_per_pixel()?
      }
    }
  case 8:
    if (bNeedLuminance) {
      // dont bother handling those other 8bit lum fmts like 4-4, since 16 8-8 is usually supported too
      assert(cNumColorChannels == 1);

      // look for native lum fmt first
      CHECK_FOR_FMT(L8, ConvLum8to8);
      CHECK_FOR_FMT(L8, ConvLum8to16_A8L8);

      if (!dx_force_16bpptextures) {
        CHECK_FOR_FMT(R8G8B8, ConvLum8to24);
        CHECK_FOR_FMT(X8R8G8B8, ConvLum8to32);
      }

      CHECK_FOR_FMT(R5G6B5, ConvLum8to16_0565);
      CHECK_FOR_FMT(X1R5G5B5, ConvLum8to16_X555);

    } else if (cNumAlphaBits == 8) {
      // look for 16bpp A8L8, else 32-bit ARGB, else 16-4444.

      // skip 8bit alpha only (D3DFMT_A8), because I think only voodoo supports it
      // and the voodoo support isn't the kind of blending model we need somehow
      // (is it that voodoo assumes color is white?  isnt that what we do in ConvAlpha8to32 anyway?)

      CHECK_FOR_FMT(A8L8, ConvAlpha8to16_A8L8);

      if (!dx_force_16bpptextures) {
        CHECK_FOR_FMT(A8R8G8B8, ConvAlpha8to32);
      }

      CHECK_FOR_FMT(A4R4G4B4, ConvAlpha8to16_4444);
    }
    break;

  default:
    szErrorMsg = "CreateTexture failed: unhandled pixel bitdepth in DX loader";
  }

  // if we've gotten here, haven't found a match
  dxgsg8_cat.error() << szErrorMsg << ": " << _tex->get_name() << endl
                     << "NumColorChannels: " <<cNumColorChannels << "; NumAlphaBits: " << cNumAlphaBits
                     << "; targetbpp: " <<target_bpp << "; SupportedTexFmtsMask: 0x" << (void*)scrn.SupportedTexFmtsMask
                     << "; NeedLuminance: " << bNeedLuminance << endl;
  goto error_exit;

  ///////////////////////////////////////////////////////////

 found_matching_format:
  // We found a suitable format that matches the texture's format.

  if (_texture->get_match_framebuffer_format()) {
    // Instead of creating a texture with the found format, we will
    // need to make one that exactly matches the framebuffer's
    // format.  Look up what that format is.
    IDirect3DSurface8 *pCurRenderTarget;
    hr = scrn.pD3DDevice->GetRenderTarget(&pCurRenderTarget);
    if (FAILED(hr)) {
      dxgsg8_cat.error() << "GetRenderTgt failed in CreateTexture: " << D3DERRORSTRING(hr);
    } else {
      D3DSURFACE_DESC SurfDesc;
      hr = pCurRenderTarget->GetDesc(&SurfDesc);
      if (FAILED(hr)) {
        dxgsg8_cat.error()
          << "GetDesc failed in CreateTexture: " << D3DERRORSTRING(hr);
      } else {
        if (TargetPixFmt != SurfDesc.Format) {
          if (dxgsg8_cat.is_debug()) {
            dxgsg8_cat.debug()
              << "Chose format " << D3DFormatStr(SurfDesc.Format)
              << " instead of " << D3DFormatStr(TargetPixFmt)
              << " for texture to match framebuffer.\n";
          }
          TargetPixFmt = SurfDesc.Format;
        }
      }
      SAFE_RELEASE(pCurRenderTarget);
    }
  }

  // validate magfilter setting
  // degrade filtering if no HW support

  Texture::FilterType ft;

  ft = _tex->get_magfilter();
  if ((ft != Texture::FT_linear) && ft != Texture::FT_nearest) {
    // mipmap settings make no sense for magfilter
    if (ft == Texture::FT_nearest_mipmap_nearest)
      ft = Texture::FT_nearest;
    else ft = Texture::FT_linear;
  }

  if ((ft == Texture::FT_linear) && !(scrn.d3dcaps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR))
    ft = Texture::FT_nearest;
  _tex->set_magfilter(ft);

  // figure out if we are mipmapping this texture
  ft = _tex->get_minfilter();
  _has_mipmaps = false;

  if (!dx_ignore_mipmaps) {  // set if no HW mipmap capable
    switch(ft) {
    case Texture::FT_nearest_mipmap_nearest:
    case Texture::FT_linear_mipmap_nearest:
    case Texture::FT_nearest_mipmap_linear:  // pick nearest in each, interpolate linearly b/w them
    case Texture::FT_linear_mipmap_linear:
      _has_mipmaps = true;
    }

    if (dx_mipmap_everything) {  // debug toggle, ok to leave in since its just a creation cost
      _has_mipmaps = true;
      if (dxgsg8_cat.is_spam()) {
        if (ft != Texture::FT_linear_mipmap_linear)
          dxgsg8_cat.spam() << "Forcing trilinear mipmapping on DX texture [" << _tex->get_name() << "]\n";
      }
      ft = Texture::FT_linear_mipmap_linear;
      _tex->set_minfilter(ft);
    }
  } else if ((ft == Texture::FT_nearest_mipmap_nearest) ||   // cvt to no-mipmap filter types
            (ft == Texture::FT_nearest_mipmap_linear)) {
    ft = Texture::FT_nearest;
  } else if ((ft == Texture::FT_linear_mipmap_nearest) ||
            (ft == Texture::FT_linear_mipmap_linear)) {
    ft = Texture::FT_linear;
  }

  assert((scrn.d3dcaps.TextureFilterCaps & D3DPTFILTERCAPS_MINFPOINT) != 0);

#define TRILINEAR_MIPMAP_TEXFILTERCAPS (D3DPTFILTERCAPS_MIPFLINEAR | D3DPTFILTERCAPS_MINFLINEAR)

  // do any other filter type degradations necessary
  switch(ft) {
  case Texture::FT_linear_mipmap_linear:
    if ((scrn.d3dcaps.TextureFilterCaps & TRILINEAR_MIPMAP_TEXFILTERCAPS) != TRILINEAR_MIPMAP_TEXFILTERCAPS) {
      if (scrn.d3dcaps.TextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR)
        ft = Texture::FT_linear_mipmap_nearest;
      else ft = Texture::FT_nearest_mipmap_nearest;  // if you cant do linear in a level, you probably cant do linear b/w levels, so just do nearest-all
    }
    break;
  case Texture::FT_nearest_mipmap_linear:
    // if we dont have bilinear, do nearest_nearest
    if (!((scrn.d3dcaps.TextureFilterCaps & D3DPTFILTERCAPS_MIPFPOINT) &&
         (scrn.d3dcaps.TextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR)))
      ft = Texture::FT_nearest_mipmap_nearest;
    break;
  case Texture::FT_linear_mipmap_nearest:
    // if we dont have mip linear, do nearest_nearest
    if (!(scrn.d3dcaps.TextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR))
      ft = Texture::FT_nearest_mipmap_nearest;
    break;
  case Texture::FT_linear:
    if (!(scrn.d3dcaps.TextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR))
      ft = Texture::FT_nearest;
    break;
  }

  _tex->set_minfilter(ft);

  uint aniso_degree;

  aniso_degree = 1;
  if (scrn.d3dcaps.RasterCaps & D3DPRASTERCAPS_ANISOTROPY) {
    aniso_degree = _tex->get_anisotropic_degree();
    if ((aniso_degree>scrn.d3dcaps.MaxAnisotropy) || dx_force_anisotropic_filtering)
      aniso_degree = scrn.d3dcaps.MaxAnisotropy;
  }
  _tex->set_anisotropic_degree(aniso_degree);

#ifdef _DEBUG
  dxgsg8_cat.spam() << "CreateTexture: setting aniso degree for " << _tex->get_name() << " to: " << aniso_degree << endl;
#endif

  UINT cMipLevelCount;

  if (_has_mipmaps) {
    cMipLevelCount = 0;  // tell CreateTex to alloc space for all mip levels down to 1x1

    if (dxgsg8_cat.is_debug())
      dxgsg8_cat.debug() << "CreateTexture: generating mipmaps for " << _tex->get_name() << endl;
  } else cMipLevelCount = 1;

  if (FAILED( hr = scrn.pD3DDevice->CreateTexture(TargetWidth, TargetHeight, cMipLevelCount, 0x0,
                                                 TargetPixFmt, D3DPOOL_MANAGED, &_d3d_texture) )) {
    dxgsg8_cat.error() << "D3D CreateTexture failed!" << D3DERRORSTRING(hr);
    goto error_exit;
  }

  if (dxgsg8_cat.is_debug()) {
    dxgsg8_cat.debug() << "CreateTexture: " << _tex->get_name() << " converting panda equivalent of " << D3DFormatStr(_d3d_format) << " = > " << D3DFormatStr(TargetPixFmt) << endl;
  }

  hr = FillDDSurfTexturePixels();
  if (FAILED(hr)) {
    goto error_exit;
  }

  // PRINT_REFCNT(dxgsg8, scrn.pD3D8);

  // Return the newly created texture
  return _d3d_texture;

 error_exit:

  RELEASE(_d3d_texture, dxgsg8, "texture", RELEASE_ONCE);
  return NULL;
}

HRESULT DXTextureContext8::
FillDDSurfTexturePixels() {
  HRESULT hr = E_FAIL;
  assert(IS_VALID_PTR(_texture));

  CPTA_uchar image = _texture->get_ram_image();
  if (image.is_null()) {
    // The texture doesn't have an image to load.  That's ok; it
    // might be a texture we've rendered to by frame buffer
    // operations or something.
    return S_OK;
  }

  PStatTimer timer(GraphicsStateGuardian::_load_texture_pcollector);

  assert(IS_VALID_PTR(_d3d_texture));

  DWORD OrigWidth  = (DWORD) _texture->get_x_size();
  DWORD OrigHeight = (DWORD) _texture->get_y_size();
  DWORD cNumColorChannels = _texture->get_num_components();
  D3DFORMAT SrcFormat = _d3d_format;
  BYTE *pPixels = (BYTE*)image.p();
  int component_width = _texture->get_component_width();

  assert(IS_VALID_PTR(pPixels));

  IDirect3DSurface8 *pMipLevel0;
  hr = _d3d_texture->GetSurfaceLevel(0, &pMipLevel0);
  if (FAILED(hr)) {
    dxgsg8_cat.error() << "FillDDSurfaceTexturePixels failed for " << _tex->get_name() << ", GetSurfaceLevel failed" << D3DERRORSTRING(hr);
    return E_FAIL;
  }

  RECT SrcSize;
  SrcSize.left = SrcSize.top = 0;
  SrcSize.right = OrigWidth;
  SrcSize.bottom = OrigHeight;

  UINT SrcPixBufRowByteLength = OrigWidth*cNumColorChannels;

  DWORD Lev0Filter, MipFilterFlags;
  bool bUsingTempPixBuf = false;

  // need filtering if size changes, (also if bitdepth reduced (need dithering)??)
  Lev0Filter = D3DX_FILTER_LINEAR ; //| D3DX_FILTER_DITHER;  //dithering looks ugly on i810 for 4444 textures

  // D3DXLoadSurfaceFromMemory will load black luminance and we want full white,
  // so convert to explicit luminance-alpha format
  if (_d3d_format == D3DFMT_A8) {
    // alloc buffer for explicit D3DFMT_A8L8
    USHORT *pTempPixBuf = new USHORT[OrigWidth*OrigHeight];
    if (!IS_VALID_PTR(pTempPixBuf)) {
      dxgsg8_cat.error() << "FillDDSurfaceTexturePixels couldnt alloc mem for temp pixbuf!\n";
      goto exit_FillDDSurf;
    }
    bUsingTempPixBuf = true;

    USHORT *pOutPix = pTempPixBuf;
    BYTE *pSrcPix = pPixels + component_width - 1;
    for (UINT y = 0; y < OrigHeight; y++) {
      for (UINT x = 0;
           x < OrigWidth;
           x++, pSrcPix += component_width, pOutPix++) {
        // add full white, which is our interpretation of alpha-only
        // (similar to default adding full opaque alpha 0xFF to
        // RGB-only textures)
        *pOutPix = ((*pSrcPix) << 8 ) | 0xFF;
      }
    }

    SrcFormat = D3DFMT_A8L8;
    SrcPixBufRowByteLength = OrigWidth*sizeof(USHORT);
    pPixels = (BYTE*)pTempPixBuf;

  } else if (component_width != 1) {
    // Convert from 16-bit per channel (or larger) format down to
    // 8-bit per channel.  This throws away precision in the
    // original image, but dx8 doesn't support high-precision images
    // anyway.

    int num_components = _texture->get_num_components();
    int num_pixels = OrigWidth * OrigHeight * num_components;
    BYTE *pTempPixBuf = new BYTE[num_pixels];
    if (!IS_VALID_PTR(pTempPixBuf)) {
      dxgsg8_cat.error() << "FillDDSurfaceTexturePixels couldnt alloc mem for temp pixbuf!\n";
      goto exit_FillDDSurf;
    }
    bUsingTempPixBuf = true;

    BYTE *pSrcPix = pPixels + component_width - 1;
    for (int i = 0; i < num_pixels; i++) {
      pTempPixBuf[i] = *pSrcPix;
      pSrcPix += component_width;
    }
    pPixels = (BYTE*)pTempPixBuf;
  }


  // filtering may be done here if texture if targetsize != origsize
#ifdef DO_PSTATS
  GraphicsStateGuardian::_data_transferred_pcollector.add_level(SrcPixBufRowByteLength * OrigHeight);
#endif
  hr = D3DXLoadSurfaceFromMemory(pMipLevel0, (PALETTEENTRY*)NULL, (RECT*)NULL, (LPCVOID)pPixels, SrcFormat,
                               SrcPixBufRowByteLength, (PALETTEENTRY*)NULL, &SrcSize, Lev0Filter, (D3DCOLOR)0x0);
  if (FAILED(hr)) {
    dxgsg8_cat.error() << "FillDDSurfaceTexturePixels failed for " << _tex->get_name() << ", D3DXLoadSurfFromMem failed" << D3DERRORSTRING(hr);
    goto exit_FillDDSurf;
  }

  if (_has_mipmaps) {
    if (!dx_use_triangle_mipgen_filter)
      MipFilterFlags = D3DX_FILTER_BOX;
    else MipFilterFlags = D3DX_FILTER_TRIANGLE;

    //    MipFilterFlags| = D3DX_FILTER_DITHER;

    hr = D3DXFilterTexture(_d3d_texture, (PALETTEENTRY*)NULL, 0, MipFilterFlags);
    if (FAILED(hr)) {
      dxgsg8_cat.error() << "FillDDSurfaceTexturePixels failed for " << _tex->get_name() << ", D3DXFilterTex failed" << D3DERRORSTRING(hr);
      goto exit_FillDDSurf;
    }
  }

 exit_FillDDSurf:
  if (bUsingTempPixBuf) {
    SAFE_DELETE_ARRAY(pPixels);
  }
  RELEASE(pMipLevel0, dxgsg8, "FillDDSurf MipLev0 texture ptr", RELEASE_ONCE);
  return hr;
}

//-----------------------------------------------------------------------------
// Name: DeleteTexture()
// Desc: Release the surface used to store the texture
//-----------------------------------------------------------------------------
void DXTextureContext8::
DeleteTexture( ) {
  if (_d3d_texture == NULL) {
    // dont bother printing the msg below, since we already released it.
    return;
  }

  if (dxgsg8_cat.is_spam()) {
    dxgsg8_cat.spam() << "Deleting DX texture for " << _tex->get_name() << "\n";
  }

  RELEASE(_d3d_texture, dxgsg8, "texture", RELEASE_ONCE);
}


////////////////////////////////////////////////////////////////////
//     Function: DXTextureContext8::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXTextureContext8::
DXTextureContext8(Texture *tex) :
  TextureContext(tex) {

  if (dxgsg8_cat.is_spam()) {
    dxgsg8_cat.spam() << "Creating DX texture [" << tex->get_name() << "], minfilter(" << tex->get_minfilter() << "), magfilter(" << tex->get_magfilter() << "), anisodeg(" << tex->get_anisotropic_degree() << ")\n";
  }

  _d3d_texture = NULL;
  _has_mipmaps = false;
  _tex = tex;
}

DXTextureContext8::
~DXTextureContext8() {
  if (dxgsg8_cat.is_spam()) {
    dxgsg8_cat.spam() << "Deleting DX8 TexContext for " << _tex->get_name() << "\n";
  }
  DeleteTexture();
  TextureContext::~TextureContext();
  _tex = NULL;
}

