// Filename: dxTextureContext.cxx
// Created by:  drose (07Oct99)
// 
////////////////////////////////////////////////////////////////////
// Copyright (C) 1999-2000  Walt Disney Imagineering, Inc.
// 
// These  coded  instructions,  statements,  data   structures   and
// computer  programs contain unpublished proprietary information of
// Walt Disney Imagineering and are protected by  Federal  copyright
// law.  They may  not be  disclosed to third  parties  or copied or
// duplicated in any form, in whole or in part,  without  the  prior
// written consent of Walt Disney Imagineering Inc.
////////////////////////////////////////////////////////////////////

#include "dxTextureContext.h"
#include "config_dxgsg.h"
#include "dxGraphicsStateGuardian.h"
#include <assert.h>

//#define FORCE_USE_OF_16BIT_TEXFMTS

TypeHandle DXTextureContext::_type_handle;

#ifdef _DEBUG
static void DebugPrintPixFmt(DDPIXELFORMAT* pddpf) {
  static int iddpfnum=0;
  ostream *dbgout = &dxgsg_cat.debug();
  
    *dbgout << "DDPF[" << iddpfnum << "]: RGBBitCount:" << pddpf->dwRGBBitCount
                     << " Flags:"  << (void *)pddpf->dwFlags ;

    if(pddpf->dwFlags & DDPF_RGB) {
       *dbgout << " RGBmask:" << (void *) (pddpf->dwRBitMask | pddpf->dwGBitMask | pddpf->dwBBitMask);
    }

    if(pddpf->dwFlags & DDPF_ALPHAPIXELS) {
      *dbgout << " Amask:" << (void *) pddpf->dwRGBAlphaBitMask;
    }

    if(pddpf->dwFlags & DDPF_LUMINANCE) {
      *dbgout << " Lummask:" << (void *) pddpf->dwLuminanceBitMask;
    }

    *dbgout << endl; 

    iddpfnum++;
}
#endif

/* for reference
enum Format {
    F_color_index,
    F_stencil_index,
    F_depth_component,
    F_red,
    F_green,
    F_blue,
    F_alpha,
    F_rgb,
    F_rgb5,    // specifically, 5 bits per R,G,B channel
    F_rgb8,    // 8 bits per R,G,B channel
    F_rgb12,   // 12 bits per R,G,B channel
    F_rgb332,  // 3 bits per R & G, 2 bits for B
    F_rgba,
    F_rgba4,   // 4 bits per R,G,B,A channel
    F_rgba8,   // 8 bits per R,G,B,A channel
    F_rgba12,  // 12 bits per R,G,B,A channel
    F_luminance,
    F_luminance_alpha,
  };
  enum Type {
    T_unsigned_byte,   // 1 byte per channel
    T_unsigned_short,  // 2 byte per channel
    T_unsigned_byte_332,  // RGB in 1 byte
    T_float,             // 1 channel stored as float
  };  
*/

////////////////////////////////////////////////////////////////////
//     Function: DXTextureContext::get_bits_per_pixel
//       Access: Protected
//  Description: Maps from the PixelBuffer's Format symbols
//               to bpp.  returns # of alpha bits
//               Note: PixelBuffer's format indicates REQUESTED final format,
//                     not the stored format, which is indicated by pixelbuffer type
////////////////////////////////////////////////////////////////////
unsigned int DXTextureContext::
get_bits_per_pixel(PixelBuffer::Format format, int *alphbits) {
	*alphbits = 0;		// assume no alpha bits
  switch (format) {
  case PixelBuffer::F_alpha:
    *alphbits = 8;
    return 8;
  case PixelBuffer::F_color_index:
  case PixelBuffer::F_red:
  case PixelBuffer::F_green:
  case PixelBuffer::F_blue:
  case PixelBuffer::F_rgb332:
  case PixelBuffer::F_luminance:
	return 8;
  case PixelBuffer::F_rgba4:  
	*alphbits = 4;
    return 16;
  case PixelBuffer::F_rgba5:
	*alphbits = 1;
    return 16;
  case PixelBuffer::F_depth_component:
  case PixelBuffer::F_rgb5:
    return 16;
  case PixelBuffer::F_rgb8:
  case PixelBuffer::F_rgb:
	  return 24;
  case PixelBuffer::F_luminance_alpha:		// must expand into DIB buffer
  case PixelBuffer::F_rgba8:
  case PixelBuffer::F_rgba:
    *alphbits = 8;
    return 32;
  case PixelBuffer::F_rgb12:
    return 36;
  case PixelBuffer::F_rgba12:
      *alphbits = 12;
	  return 48;
  }
return 8;
}

//-----------------------------------------------------------------------------
// Name: CreateTextureFromBitmap()
// Desc: Use a bitmap to create a texture for the specified device. This code
//       gets the attributes of the texture from the bitmap, creates the
//       texture, and then copies the bitmap into the texture.
//-----------------------------------------------------------------------------
LPDIRECTDRAWSURFACE7 DXTextureContext::
CreateTexture( HDC hdc, LPDIRECT3DDEVICE7 pd3dDevice, int cNumTexPixFmts, LPDDPIXELFORMAT pTexPixFmts)
{
    HRESULT hr;
  
	PixelBuffer *pbuf = _texture->_pbuffer;
    BOOL bSizeExpanded = FALSE;
    int cNumAlphaBits;     //  number of alpha bits in texture pixfmt

    DDPIXELFORMAT *pDesiredPixFmt;
    LPDIRECTDRAWSURFACE7 pddsRender;
    LPDIRECTDRAW7        pDD = NULL;

    DDPIXELFORMAT TexFmtsArr[MAX_DX_TEXPIXFMTS];  

    typedef enum {None,Conv32to32,Conv32to32_NoAlpha,Conv32to24,Conv32to16_0555,
                  Conv32to16_1555,Conv32to16_0565,Conv32to16_4444,Conv24to32,Conv24to24,
                  Conv24to16_0555,Conv24to16_0565
    } ConversionType;

    ConversionType ConvNeeded;

#ifdef _DEBUG
    char *ConvNameStrs[] = {"None","Conv32to32","Conv32to32_NoAlpha","Conv32to24","Conv32to16_0555",
                  "Conv32to16_1555","Conv32to16_0565","Conv32to16_4444","Conv24to32","Conv24to24",
                  "Conv24to16_0555","Conv24to16_0565"};
#endif

    // make local copy of array so I can muck with it during searches for this texture fmt
    memcpy(TexFmtsArr,pTexPixFmts,cNumTexPixFmts*sizeof(DDPIXELFORMAT));
    pTexPixFmts=TexFmtsArr;
	
    // bpp indicates requested fmt, not pixbuf fmt
    DWORD bpp = get_bits_per_pixel(pbuf->get_format(), &cNumAlphaBits);
    PixelBuffer::Type pixbuf_type = pbuf->get_image_type();
    DWORD cNumColorChannels = pbuf->get_num_components();

    if((cNumColorChannels != 3) && (cNumColorChannels != 4)) {
        dxgsg_cat.error() << "CreateTexture failed for "<< _tex->get_name()<<", havent handled pixbufs with < 3 channels yet! \n";
        return NULL;
    }

    if((cNumColorChannels == 3) && (cNumAlphaBits>0)) {
        dxgsg_cat.error() << "CreateTexture ignoring request for texformat w/alpha channel since original texture is RGB only\n";
        cNumAlphaBits=0;
    }

    if((pixbuf_type != PixelBuffer::T_unsigned_byte) || (pbuf->get_component_width()!=1)) {
        dxgsg_cat.error() << "CreateTexture failed, havent handled non 8-bit channel pixelbuffer types yet! \n";
        return NULL;
    }
    
    DWORD dwOrigWidth  = (DWORD)pbuf->get_xsize();
    DWORD dwOrigHeight = (DWORD)pbuf->get_ysize();

    if (pbuf->get_format() == PixelBuffer::F_luminance_alpha) {	// must expand	
		
        dxgsg_cat.error() << "havent handled luminance in CreateTexture yet!! \n";
        return NULL;

        /*
        /// old code I need to rewrite and re-test for this case
        
        #define DIBLINEWIDTH(x)  ((((x)+3)>>2)<<2)

        DWORD lineBytes =  DIBLINEWIDTH(dwWidth * (bpp >> 3));
        DWORD imageBytes = lineBytes * dwHeight;
		int bytes_per_pixel = bpp >> 3;			// should be 4
		DWORD padBytes = lineBytes - (dwWidth * bytes_per_pixel);   // should be 0;
		unsigned char *src = (unsigned char *)pbuf->_image;
		unsigned char *dst = (unsigned char *)bits;
		int x,y;
		for (y = dwHeight; --y >= 0;)
			{
			for (x = dwWidth; --x >= 0;)
				{
				*dst++ = *src;
				*dst++ = *src;
				*dst++ = *src++;
				*dst++ = *src++;
				}
			dst += padBytes;
			}
        */
    }

    // Get the device caps so we can check if the device has any constraints
    // when using textures
    D3DDEVICEDESC7 ddDesc;
    if( FAILED( pd3dDevice->GetCaps( &ddDesc ) ) ) {
        goto error_exit;
    }

    // Setup the new surface desc for the texture. Note how we are using the
    // texture manage attribute, so Direct3D does alot of dirty work for us
    DDSURFACEDESC2 ddsd;
    ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );
    ddsd.dwSize          = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags         =  DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH 
                           | DDSD_PIXELFORMAT | DDSD_TEXTURESTAGE;

	ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);

    // setup ddpf to match against avail fmts
	
    ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;  // assumes non-luminance/bump texture
    ddsd.ddpfPixelFormat.dwRGBBitCount = bpp;

	if (cNumAlphaBits) {
		if (bpp == 8 && cNumAlphaBits == 8)	// handle special case:  Alpha only buffer
			{
			ddsd.dwFlags |= DDPF_ALPHA;
			ddsd.dwAlphaBitDepth = 8;
			ddsd.ddpfPixelFormat.dwAlphaBitDepth = 8;
			ddsd.ddpfPixelFormat.dwFlags = DDPF_ALPHA;
            ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0xff;
			}
		else {
			ddsd.ddpfPixelFormat.dwFlags |= DDPF_ALPHAPIXELS;
			if (cNumAlphaBits == 8)
				ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0xff000000;
			else if (cNumAlphaBits == 4)
				ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0xf000;
			}
    }

    ddsd.dwWidth         = dwOrigWidth;
    ddsd.dwHeight        = dwOrigHeight;

#define ISPOW2(X) (((X) & ((X)-1))==0)

    if(!ISPOW2(ddsd.dwWidth) || !ISPOW2(ddsd.dwHeight)) {
        dxgsg_cat.error() << "ERROR: Image size is not a power of 2 for " << _tex->get_name() << "!!!!! \n";
#define EXPAND_TEXSIZE_TO_POW2
#ifndef EXPAND_TEXSIZE_TO_POW2
#ifdef _DEBUG
        exit(1);  // want to catch badtexsize errors
#else
       goto error_exit;
#endif
#else
        // Expand width and height, if the driver requires it
        if( ddDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_POW2 ) {
            // round up to next pow of 2
            for( ddsd.dwWidth=1;  dwOrigWidth>ddsd.dwWidth;   ddsd.dwWidth<<=1 );
            for( ddsd.dwHeight=1; dwOrigHeight>ddsd.dwHeight; ddsd.dwHeight<<=1 );
            bSizeExpanded = TRUE;
            dxgsg_cat.debug() << "Expanding texture to power of 2 size (extra space will be black)\n";
        }
#endif
    }

    if( ddDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY ) {
        // expand smaller dimension to equal the larger
        if( ddsd.dwWidth > ddsd.dwHeight ) ddsd.dwHeight = ddsd.dwWidth;
        else                               ddsd.dwWidth  = ddsd.dwHeight;
        bSizeExpanded = (ddsd.dwWidth!=ddsd.dwHeight);
        if(bSizeExpanded)
            dxgsg_cat.debug() << "Forcing texture to square size (extra space will be black)\n";
    }

    int i;

#ifdef _DEBUG
    { static BOOL bPrinted=FALSE;
       if(!bPrinted) {
            for(i=0;i<cNumTexPixFmts;i++) { DebugPrintPixFmt(&pTexPixFmts[i]); }
            bPrinted=TRUE;
       }
    }
#endif

    // first search for an exact match
    pDesiredPixFmt = &ddsd.ddpfPixelFormat;

    LPDDPIXELFORMAT pCurPixFmt;
    char *szErrorMsg;

    szErrorMsg = "CreateTexture failed: couldn't find compatible Tex DDPIXELFORMAT!  \n";

    dxgsg_cat.spam() << "CreateTexture handling bitdepth: " << bpp << " alphabits: " << cNumAlphaBits << "\n";

    // Mark formats I dont want to deal with
    for(i=0,pCurPixFmt=pTexPixFmts;i<cNumTexPixFmts;i++,pCurPixFmt++) {
        if(  ( pCurPixFmt->dwFlags & (DDPF_LUMINANCE|DDPF_BUMPLUMINANCE|DDPF_BUMPDUDV) )  ||
             ( pCurPixFmt->dwFourCC != 0 ) ||
             ((cNumAlphaBits==0) && (pCurPixFmt->dwFlags & DDPF_ALPHAPIXELS))) {
            
            // Make sure to skip any FourCC formats, bump/luminance formats
            // they are not handled by this code yet

            // note: I'm screening out alpha if no alpha requested, so
            // search will fails if 32-rgba avail, but not 32-bit rgb
            // I could recode for that case too, hopefully this case will not pop up

            pCurPixFmt->dwRGBBitCount+=1;  // incr so it wont be an exact match anymore
        }
    }

    assert((cNumColorChannels==4)==(cNumAlphaBits>0));

    // handle each bitdepth separately

    switch(bpp) {  // bpp is REQUESTED bpp

      case 32:

#ifndef FORCE_USE_OF_16BIT_TEXFMTS
        for(i=0,pCurPixFmt=pTexPixFmts;i<cNumTexPixFmts;i++,pCurPixFmt++) {
            if((pCurPixFmt->dwRGBBitCount==32) && 
              (((pCurPixFmt->dwFlags & DDPF_ALPHAPIXELS)!=0)==(cNumAlphaBits!=0))) {
                  // we should have a match
                  assert(pCurPixFmt->dwRGBAlphaBitMask==0xFF000000);
                  ConvNeeded=((cNumColorChannels==3) ? Conv24to32 : Conv32to32);
                  goto found_matching_format;
                  break;
              }
        }
#endif

        if(cNumAlphaBits>0) {
            // no 32-bit fmt, look for 16 bit w/alpha  (1-15)

            for(i=0,pCurPixFmt=&pTexPixFmts[cNumTexPixFmts-1];i<cNumTexPixFmts;i++,pCurPixFmt--) {
                // assume ALPHAMASK is x8000 and RGBMASK is x7fff to simplify 32->16 conversion
                // this should be true on most cards.  
                if((pCurPixFmt->dwRGBBitCount==16) && (pCurPixFmt->dwFlags & DDPF_ALPHAPIXELS)
                   && (pCurPixFmt->dwRGBAlphaBitMask==0x8000)) {
                      ConvNeeded=Conv32to16_1555;
                      goto found_matching_format;
                  }
#if 0
            // 32 bit RGBA was requested, but only 16 bit alpha fmts are avail
            // by default, convert to 15-1, which is the most common fmt

            // 4-4-4-4 would be useful if we know pixelbuf contains non-binary alpha,
            // but hard to infer that from RGBA request, and 15-1 is the better general choice

                if((pCurPixFmt->dwRGBBitCount==16) && (pCurPixFmt->dwFlags & DDPF_ALPHAPIXELS)
                   && (pCurPixFmt->dwRGBAlphaBitMask==0xf000)) {
                      ConvNeeded=Conv32to16_4444;
                      goto found_matching_format;
                  }
#endif
            }

            // at this point, bail.  dont worry about converting to non-alpha formats yet,
            // I think this will be a very rare case
            szErrorMsg = "CreateTexture failed: couldn't find compatible Tex DDPIXELFORMAT! no available 16 or 32-bit alpha formats!\n";
        }

        break;

      case 24:

          assert(cNumAlphaBits==0);  // dont know how to handle non-zero alpha for 24bit total

#ifndef FORCE_USE_OF_16BIT_TEXFMTS
          for(i=0,pCurPixFmt=pTexPixFmts;i<cNumTexPixFmts;i++,pCurPixFmt++) {
              if((pCurPixFmt->dwFlags & DDPF_RGB)&&(pCurPixFmt->dwRGBBitCount==24)) {
                      ConvNeeded=((cNumColorChannels==3) ? Conv24to24 : Conv32to24);
                      goto found_matching_format;
              }
          }
          
          // no 24-bit fmt.  look for 32 bit fmt  (note: this is memory-hogging choice
          // instead I could look for memory-conserving 16-bit fmt).  
          // check mask to ensure ARGB, not RGBA (which I am not handling here)

          for(i=0,pCurPixFmt=pTexPixFmts;i<cNumTexPixFmts;i++,pCurPixFmt++) {
              if((pCurPixFmt->dwRGBBitCount==32) && (pCurPixFmt->dwFlags & DDPF_RGB)
                 && ((pCurPixFmt->dwRBitMask|pCurPixFmt->dwGBitMask|pCurPixFmt->dwBBitMask)==0xFFFFFF)
                 ) {
                    // I'm allowing alpha formats here.  will set alpha to opaque 
                    ConvNeeded=((cNumColorChannels==3) ? Conv24to32 : Conv32to32_NoAlpha);
                    goto found_matching_format;
                }
          }
#endif

          // no 24-bit or 32 fmt.  look for 16 bit fmt 
          for(i=0,pCurPixFmt=&pTexPixFmts[cNumTexPixFmts-1];i<cNumTexPixFmts;i++,pCurPixFmt--) {
              // assume RGBMASK is x7fff to simplify 32->16 conversion, should be true on most cards.  
              if((pCurPixFmt->dwFlags & DDPF_RGB) && (pCurPixFmt->dwRGBBitCount==16) 
                 && !(pCurPixFmt->dwFlags & DDPF_ALPHAPIXELS))  {  // no alpha fmts
                        // if numchan==4,this means user has requested we throw away the input alpha channel
                        if(pCurPixFmt->dwGBitMask==0x7E0) {
                            // assumes GBitMask is the biggest one, if we have 16 bit 3-channel
                            ConvNeeded=((cNumColorChannels==3) ? Conv24to16_0565 : Conv32to16_0565);
                        } else {
                            assert((pCurPixFmt->dwRBitMask|pCurPixFmt->dwGBitMask|pCurPixFmt->dwBBitMask)==0x7FFF);
                            ConvNeeded=((cNumColorChannels==3) ? Conv24to16_0555 : Conv32to16_0555);
                        }
                        goto found_matching_format;
                }
          }

          // at this point, bail.  
          break;

      case 16:
          // look for compatible 16bit fmts, if none then give up 
          // (dont worry about other bitdepths for 16 bit)

          for(i=0,pCurPixFmt=pTexPixFmts;i<cNumTexPixFmts;i++,pCurPixFmt++) {

              if((pCurPixFmt->dwRGBBitCount==16)&&(pCurPixFmt->dwFlags & DDPF_RGB)) {
                  switch(cNumAlphaBits) {
                      case 0:  
                          if(!(pCurPixFmt->dwFlags & DDPF_ALPHAPIXELS)) {
                            // if numchan==4,this means user has requested we throw away the input alpha channel
                            if(pCurPixFmt->dwGBitMask==0x7E0) {
                                // assumes GBitMask is the biggest one, if we have 16 bit 3-channel
                                ConvNeeded=((cNumColorChannels==3) ? Conv24to16_0565 : Conv32to16_0565);
                            } else {
                                assert((pCurPixFmt->dwRBitMask|pCurPixFmt->dwGBitMask|pCurPixFmt->dwBBitMask)==0x7FFF);
                                ConvNeeded=((cNumColorChannels==3) ? Conv24to16_0555 : Conv32to16_0555);
                            }
                            goto found_matching_format;
                          }
                          break;
                      case 1:  
                          if((pCurPixFmt->dwFlags & DDPF_ALPHAPIXELS)&&
                             (pCurPixFmt->dwRGBAlphaBitMask==0x8000)) {
                              ConvNeeded=Conv32to16_1555;
                              goto found_matching_format;
                          }                       
                          break;
                      case 4: 
                          if((pCurPixFmt->dwFlags & DDPF_ALPHAPIXELS)&&
                             (pCurPixFmt->dwRGBAlphaBitMask==0xF000)) {
                              ConvNeeded=Conv32to16_4444;
                              goto found_matching_format;
                          }                       
                          break;
                  }
              }
          }

          break;

      case 8:

      default:
        szErrorMsg = "CreateTexture failed: unhandled pixel bitdepth in DX loader";
    }

    // if we've gotten here, haven't found a match

    dxgsg_cat.error() << szErrorMsg << ";  requested tex bitdepth: " << bpp << "\n";
    goto error_exit;

    ///////////////////////////////////////////////////////////

    found_matching_format:

    ddsd.ddpfPixelFormat = *pCurPixFmt;
    
	// Get the device's render target, so we can then use the render target to
    // get a ptr to a DDraw object. We need the DirectDraw interface for
    // creating surfaces.

    pd3dDevice->GetRenderTarget( &pddsRender );
    pddsRender->GetDDInterface( (VOID**)&pDD );
    pddsRender->Release();

    ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE;

    ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE  // Turn on texture management 
                         | DDSCAPS2_HINTSTATIC;    // BUGBUG:  is this ok for ALL textures?

    PRINTVIDMEM(pDD,&ddsd.ddsCaps,"texture surf (includes AGP mem)");

    // Create a new surface for the texture
    if( FAILED( hr = pDD->CreateSurface( &ddsd, &_surface, NULL ) ) )
    {
        dxgsg_cat.error() << "CreateTexture failed: pDD->CreateSurface() failed!  hr = " << ConvD3DErrorToString(hr) << "\n";
        goto error_exit;
    }

    if(bSizeExpanded) {
       // init to black
       DDBLTFX bltfx;
       ZeroMemory(&bltfx,sizeof(bltfx));
       bltfx.dwSize = sizeof(bltfx);
       bltfx.dwFillColor=0;
       if( FAILED( hr = _surface->Blt(NULL,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT,&bltfx))) {
           dxgsg_cat.error() << "CreateTexture failed: _surface->Blt() failed! hr = " << ConvD3DErrorToString(hr) << "\n";
       }
    }

    // Create a new surface for the texture
    if( FAILED( hr = _surface->Lock( NULL, &ddsd, DDLOCK_WAIT, NULL ))) {
        dxgsg_cat.error() << "CreateTexture failed: _surface->Lock() failed! hr = " << ConvD3DErrorToString(hr) << "\n";
        goto error_exit;
    }

    {
        DWORD lPitch = ddsd.lPitch;
        BYTE* pDDSurfBytes = (BYTE*)ddsd.lpSurface;
    
        BYTE r,g,b,a;
        DWORD x,y,dwPixel;

#ifdef _DEBUG
        dxgsg_cat.spam() << "CreateTexture executing conversion " << ConvNameStrs[ConvNeeded] << "\n";    
#endif

        switch(ConvNeeded) {
         case Conv32to32:
         case Conv32to32_NoAlpha: {

          DWORD *pSrcWord = (DWORD *) pbuf->_image.p();
          DWORD *pDstWord;
          DWORD dwAlphaMaskOff = (ConvNeeded==Conv32to32_NoAlpha) ? 0x00FFFFFF : 0xFFFFFFFF;

          for( y=0; y<dwOrigHeight; y++,pDDSurfBytes+=ddsd.lPitch) {
              pDstWord = (DWORD*)pDDSurfBytes;

              for( x=0; x<dwOrigWidth; x++,pSrcWord++,pDstWord++) {
                  dwPixel = *pSrcWord;

                  // pixel buffer is in ABGR format(it stores big-endian RGBA)
                  // need to change byte order to ARGB
#if 0
                  a = (BYTE)((dwPixel>>24));   // unsigned >>, no need to &
                  b = (BYTE)((dwPixel>>16)&0x000000ff);
                  g = (BYTE)((dwPixel>> 8)&0x000000ff);
                  r = (BYTE)((dwPixel    )&0x000000ff);

                  *pDstWord = (a << 24) | (r << 16)| (g << 8) | b;
#else 
                  // simpler: just swap r & b                   
                  b = (BYTE)((dwPixel>>16)&0x000000ff);
                  r = (BYTE)((dwPixel    )&0x000000ff);
                  *pDstWord = ((dwPixel & 0xff00ff00) | (r<<16) | b) & dwAlphaMaskOff;
#endif
                }
            }
            break;
          }

        case Conv32to16_1555: 
        case Conv32to16_0555: {
          DWORD abit,*pSrcWord = (DWORD *) pbuf->_image.p();
          WORD *pDstWord;
          DWORD dwAlphaMaskOff = (ConvNeeded==Conv32to16_0555) ? 0x7FFF : 0xFFFF;

          assert(ddsd.ddpfPixelFormat.dwRBitMask==0x7C00);  
          // for some reason, bits are 'in-order' when converting to 16bit

          // just handle 1/15 alpha/rgb
          for( y=0; y<dwOrigHeight; y++,pDDSurfBytes+=ddsd.lPitch) {
              pDstWord = (WORD*)pDDSurfBytes;

              for( x=0; x<dwOrigWidth; x++,pSrcWord++,pDstWord++) {
                  dwPixel = *pSrcWord;

                  // pixel buffer is in ABGR format (orig fmt designed for big-endian RGBA)
                  // need to change byte order to ARGB

                  abit = ((dwPixel>>16) & 0x00008000);  // just copy high bit

                  // just look at most-signf-bit for alpha.  (alternately, could
                  // convert any non-zero alpha to full transparent)

                  b = (BYTE)((dwPixel>>16) & 0x000000ff)    >> 3;
                  g = (BYTE)((dwPixel>> 8) & 0x000000ff)    >> 3;
                  r = (BYTE)((dwPixel    ) & 0x000000ff)    >> 3;

                  // code truncates 8 bit values to 5 bit (or 1 for alpha)

                  *pDstWord = (abit | (r << 10)| (g << 5) | b) & dwAlphaMaskOff;
              }
          }
          break;
        }

        case Conv32to16_0565: {
          DWORD *pSrcWord = (DWORD *) pbuf->_image.p();
          WORD *pDstWord;

          assert(ddsd.ddpfPixelFormat.dwRBitMask==0xF800);  
          // for some reason, bits are 'in-order' when converting to 16bit

          // just handle 1/15 alpha/rgb
          for( y=0; y<dwOrigHeight; y++,pDDSurfBytes+=ddsd.lPitch) {
              pDstWord = (WORD*)pDDSurfBytes;

              for( x=0; x<dwOrigWidth; x++,pSrcWord++,pDstWord++) {
                  dwPixel = *pSrcWord;

                  // pixel buffer is in ABGR format (orig fmt designed for big-endian RGBA)
                  // need to change byte order to ARGB

                  // just look at most-signf-bit for alpha.  (alternately, could
                  // convert any non-zero alpha to full transparent)

                  b = (BYTE)((dwPixel>>16) & 0x000000ff)    >> 3;
                  g = (BYTE)((dwPixel>> 8) & 0x000000ff)    >> 2;
                  r = (BYTE)((dwPixel    ) & 0x000000ff)    >> 3;

                  // code truncates 8 bit values to 5 bit (or 1 for alpha)

                  *pDstWord = ((r << 11)| (g << 5) | b);
              }
          }
          break;
        }


        case Conv32to24: {

          DWORD *pSrcWord = (DWORD *) pbuf->_image.p();
          BYTE *pDstWord;

          for( y=0; y<dwOrigHeight; y++,pDDSurfBytes+=ddsd.lPitch) {
              pDstWord = (BYTE*)pDDSurfBytes;

              for( x=0; x<dwOrigWidth; x++,pSrcWord++,pDstWord+=3) {
                  // pixel buffer is in ABGR format(it stores big-endian RGBA)
                  // need to change byte order to ARGB
                  dwPixel = *pSrcWord;

                  b = (BYTE)((dwPixel>>16) & 0x000000ff);
                  g = (BYTE)((dwPixel>> 8) & 0x000000ff);
                  r = (BYTE)((dwPixel    ) & 0x000000ff);

                  *pDstWord     = r;
                  *(pDstWord+1) = g;
                  *(pDstWord+2) = b;
              }
          }
          break;
        }


        case Conv32to16_4444: {
           DWORD *pSrcWord = (DWORD *) pbuf->_image.p();
           WORD *pDstWord;
    
           assert(cNumAlphaBits==4);  
           assert(ddsd.ddpfPixelFormat.dwRGBAlphaBitMask==0xf000);  // assumes ARGB
           assert(ddsd.ddpfPixelFormat.dwRBitMask==0x0f00);  // assumes ARGB
    
           for( y=0; y<dwOrigHeight; y++,pDDSurfBytes+=ddsd.lPitch) {
              pDstWord = (WORD*)pDDSurfBytes;

              for( x=0; x<dwOrigWidth; x++,pSrcWord++,pDstWord++) {
                  dwPixel = *pSrcWord;

                  // pixel buffer is in ABGR format (orig fmt designed for big-endian RGBA)
                  // need to change byte order to ARGB

                  a = (BYTE)((dwPixel>>24))                   >> 4; 
                  b = (BYTE)((dwPixel>>16) & 0x000000ff)      >> 4;
                  g = (BYTE)((dwPixel>> 8) & 0x000000ff)      >> 4;
                  r = (BYTE)((dwPixel    ) & 0x000000ff)      >> 4;

                  *pDstWord = (a << 12) | (r << 8)| (g << 4) | b;
                }
            }
            break;
          }


        case Conv24to24: {

          BYTE *pSrcWord = (BYTE *) pbuf->_image.p();
          BYTE *pDstWord;

          for( y=0; y<dwOrigHeight; y++,pDDSurfBytes += ddsd.lPitch) {
              pDstWord = (BYTE*)pDDSurfBytes;

              for( x=0; x<dwOrigWidth; x++,pSrcWord+=3,pDstWord+=3) {
                  // pixel buffer is in ABGR format(it stores big-endian RGBA)
                  // need to change byte order to ARGB

                  b = *pSrcWord;
                  g = *(pSrcWord+1);
                  r = *(pSrcWord+2);

                  *pDstWord     = r;
                  *(pDstWord+1) = g;
                  *(pDstWord+2) = b;
              }
          }
          break;
        }

        case Conv24to16_0555: {
             BYTE *pSrcWord = (BYTE *) pbuf->_image.p();
             WORD *pDstWord;

             assert(ddsd.ddpfPixelFormat.dwRBitMask==0x7C00);  
             // for some reason, bits are 'in-order' when converting to 16bit

             for( y=0; y<dwOrigHeight; y++,pDDSurfBytes += ddsd.lPitch) {
                 pDstWord = (WORD*)pDDSurfBytes;

                 for( x=0; x<dwOrigWidth; x++,pSrcWord+=3,pDstWord++) {

                     // pixel buffer is in ABGR format (orig fmt designed for big-endian RGBA)
                     // need to change byte order to ARGB
                     r = *pSrcWord       >> 3;
                     g = *(pSrcWord+1)   >> 3;
                     b = *(pSrcWord+2)   >> 3;

                     // code truncates 8 bit values to 5 bit, leaves high bit as 0

                     *pDstWord = (r << 10)| (g << 5) | b;
                 }
             }
             break;
           }

        case Conv24to16_0565: {
             BYTE *pSrcWord = (BYTE *) pbuf->_image.p();
             WORD *pDstWord;

             assert(ddsd.ddpfPixelFormat.dwRBitMask==0xF800);  
             // for some reason, bits are 'in-order' when converting to 16bit

             for( y=0; y<dwOrigHeight; y++,pDDSurfBytes += ddsd.lPitch) {
                 pDstWord = (WORD*)pDDSurfBytes;

                 for( x=0; x<dwOrigWidth; x++,pSrcWord+=3,pDstWord++) {

                     // pixel buffer is in ABGR format (orig fmt designed for big-endian RGBA)
                     // need to change byte order to ARGB
                     r = *pSrcWord       >> 3;
                     g = *(pSrcWord+1)   >> 2;
                     b = *(pSrcWord+2)   >> 3;

                     // code truncates 8 bit values to 5 bit, leaves high bit as 0

                     *pDstWord = (r << 11)| (g << 5) | b;
                 }
             }
             break;
           }

        case Conv24to32: {

          BYTE *pSrcWord = (BYTE *) pbuf->_image.p();
          DWORD *pDstWord;

          for( y=0; y<dwOrigHeight; y++,pDDSurfBytes += ddsd.lPitch) {
              pDstWord = (DWORD *)pDDSurfBytes;

              for( x=0; x<dwOrigWidth; x++,pSrcWord+=3,pDstWord++) {
                  // pixel buffer is in ABGR format(it stores big-endian RGBA)
                  // need to change byte order to ARGB

                  // bugbug?  swapping doesnt work  I dont understand why must
                  // swap bytes for 32-bit RGBA (herc) but not here in 24bit, 
                  // where they seem to be in correct order in pixelbuffer

                  r = *pSrcWord;
                  g = *(pSrcWord+1);
                  b = *(pSrcWord+2);

                  *pDstWord = 0xFF000000 | (r << 16)| (g << 8) | b;
              }
          }
          break;
        }

        default:  
               dxgsg_cat.error() << "CreateTexture failed! unhandled texture conversion type: "<< ConvNeeded <<" \n";
               _surface->Unlock(NULL);
               goto error_exit;
        }
    }

    _surface->Unlock(NULL);

    // Done with DDraw
    pDD->Release();

    // Return the newly created texture
    return _surface;

 error_exit:

    if(pDD!=NULL)
     pDD->Release();
    if(_surface!=NULL) {
      _surface->Release();
      _surface = NULL;
    }

    return NULL;
}


//-----------------------------------------------------------------------------
// Name: DeleteTexture()
// Desc: Release the surface used to store the texture
//-----------------------------------------------------------------------------
void DXTextureContext::
DeleteTexture( )
{
	if (_surface) _surface->Release();
	_surface = NULL;
}


////////////////////////////////////////////////////////////////////
//     Function: DXTextureContext::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
DXTextureContext::
DXTextureContext(Texture *tex) :
  TextureContext(tex)
{
#ifdef _DEBUG
  dxgsg_cat.spam() << "Making DX texture for " << tex->get_name() << "\n";
#endif
  _surface = NULL;
  _tex = tex;
}

DXTextureContext::
~DXTextureContext()
{
#ifdef _DEBUG
    dxgsg_cat.spam() << "Deleting DX texture for " << _tex->get_name() << "\n";    
#endif
	DeleteTexture();
	TextureContext::~TextureContext();
    _tex = NULL;
}


