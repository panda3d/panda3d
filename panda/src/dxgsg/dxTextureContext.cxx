// Filename: dxTextureContext.cxx
// Created by:  drose (07Oct99)
// 
////////////////////////////////////////////////////////////////////
// Copyright (C) 2000 Walt Disney Imagineering, Inc.
// 
// These  coded  instructions,  statements,  data   structures   and
// computer  programs contain unpublished proprietary information of
// Walt Disney Imagineering and are protected by  Federal  copyright
// law.  They may  not be  disclosed to third  parties  or copied or
// duplicated in any form, in whole or in part,  without  the  prior
// written consent of Walt Disney Imagineering Inc.
////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <time.h>
#include "dxTextureContext.h"
#include "config_dxgsg.h"
#include "dxGraphicsStateGuardian.h"
#include "pnmImage.h"

//#define FORCE_16bpp_1555

typedef enum {None,Conv32to32,Conv32to32_NoAlpha,Conv32to24,Conv32to16_0555,
                  Conv32to16_1555,Conv32to16_0565,Conv32to16_4444,Conv24to32,Conv24to24,
                  Conv24to16_0555,Conv24to16_0565,ConvLum16to16_1555,ConvLum16to16_4444,
                  ConvLum16to32,ConvLum16to16,ConvLum8to8,ConvLum8to24,ConvLum8to32,ConvLum8to16_0555,ConvLum8to16_0565
} ConversionType;

#ifdef _DEBUG
char *ConvNameStrs[] = {"None","Conv32to32","Conv32to32_NoAlpha","Conv32to24","Conv32to16_0555",
                  "Conv32to16_1555","Conv32to16_0565","Conv32to16_4444","Conv24to32","Conv24to24",
                  "Conv24to16_0555","Conv24to16_0565","ConvLum16to16_1555","ConvLum16to16_4444",
                  "ConvLum16to32","ConvLum16to16","ConvLum8to8","ConvLum8to24","ConvLum8to32",
                  "ConvLum8to16_0555","ConvLum8to16_0565"
    };
#endif

TypeHandle DXTextureContext::_type_handle;

#define SWAPDWORDS(X,Y)  { DWORD temp=X;  X=Y; Y=temp; }

#ifdef _DEBUG
static void DebugPrintPixFmt(DDPIXELFORMAT* pddpf) {
  static int iddpfnum=0;
  ostream *dbgout = &dxgsg_cat.debug();
  
    *dbgout << "DDPF[" << iddpfnum << "]: RGBBitCount:" << pddpf->dwRGBBitCount
                     << " Flags:"  << (void *)pddpf->dwFlags ;

    if(pddpf->dwFlags & DDPF_RGB) {
       *dbgout << " RGBmask:" << (void *) (pddpf->dwRBitMask | pddpf->dwGBitMask | pddpf->dwBBitMask);
       *dbgout << " Rmask:" << (void *) (pddpf->dwRBitMask);
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

void PrintLastError(char *msgbuf) {
    DWORD dwFlags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;

    if(msgbuf==NULL) {
        LPVOID lpMsgBuf;
        dwFlags|=FORMAT_MESSAGE_ALLOCATE_BUFFER;
        FormatMessage( dwFlags,
                       NULL,GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                       (LPTSTR) &lpMsgBuf,0,NULL );
        MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
        LocalFree(lpMsgBuf);
    } else {
        FormatMessage( dwFlags,
                   NULL,GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                   (LPTSTR) msgbuf,500,NULL );
    }
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
  F_rgb,     // any suitable RGB mode, whatever the hardware prefers
  F_rgb5,    // specifically, 5 bits per R,G,B channel
  F_rgb8,    // 8 bits per R,G,B channel
  F_rgb12,   // 12 bits per R,G,B channel
  F_rgb332,  // 3 bits per R & G, 2 bits for B
  F_rgba,    // any suitable RGBA mode, whatever the hardware prefers
  F_rgbm,    // as above, but only requires 1 bit for alpha (i.e. mask)
  F_rgba4,   // 4 bits per R,G,B,A channel
  F_rgba5,   // 5 bits per R,G,B channel, 1 bit alpha
  F_rgba8,   // 8 bits per R,G,B,A channel
  F_rgba12,  // 12 bits per R,G,B,A channel
  F_luminance,
  F_luminance_alpha
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
    *alphbits = 0;      // assume no alpha bits
  switch (format) {
  case PixelBuffer::F_alpha:
    *alphbits = 8;
    return 8;
  case PixelBuffer::F_color_index:
  case PixelBuffer::F_red:
  case PixelBuffer::F_green:
  case PixelBuffer::F_blue:
  case PixelBuffer::F_rgb332:
  case PixelBuffer::F_luminance_alphamask:
    *alphbits = 1;
    return 16;
  case PixelBuffer::F_luminance_alpha:
    *alphbits = 8;
    return 16;
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
  case PixelBuffer::F_rgba8:
  case PixelBuffer::F_rgba:
    *alphbits = 8;
    return 32;
  case PixelBuffer::F_rgbm: 
    *alphbits = 1;
    return 32;
  case PixelBuffer::F_rgb12:
    return 36;
  case PixelBuffer::F_rgba12:
      *alphbits = 12;
      return 48;
  }
return 8;
}

HRESULT ConvertPixBuftoDDSurf(ConversionType ConvNeeded,BYTE *pbuf,LPDIRECTDRAWSURFACE7 pDDSurf) {
        HRESULT hr;
        DX_DECLARE_CLEAN(DDSURFACEDESC2, ddsd);

        if( FAILED( hr = pDDSurf->Lock( NULL, &ddsd,  DDLOCK_NOSYSLOCK | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL )))  {
            dxgsg_cat.error() << "CreateTexture failed: _surface->Lock() failed on texture! hr = " << ConvD3DErrorToString(hr) << "\n";
            return hr;
        }

        //pbuf contains raw ARGB in PixelBuffer byteorder

        DWORD lPitch = ddsd.lPitch;
        BYTE* pDDSurfBytes = (BYTE*)ddsd.lpSurface;
        BYTE r,g,b,a;
        DWORD x,y,dwPixel;
        DWORD dwOrigWidth=ddsd.dwWidth,dwOrigHeight=ddsd.dwHeight;

        switch(ConvNeeded) {
         case Conv32to32:
         case Conv32to32_NoAlpha: {

          DWORD *pSrcWord = (DWORD *) pbuf;
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
          DWORD abit,*pSrcWord = (DWORD *) pbuf;
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

        case Conv32to16_0565: {   // could merge this w/above case, but whatever
          DWORD *pSrcWord = (DWORD *) pbuf;
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

          DWORD *pSrcWord = (DWORD *) pbuf;
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
           DWORD *pSrcWord = (DWORD *) pbuf;
           WORD *pDstWord;
    
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

          BYTE *pSrcWord = (BYTE *) pbuf;
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
             BYTE *pSrcWord = (BYTE *) pbuf;
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
             BYTE *pSrcWord = (BYTE *) pbuf;
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

          BYTE *pSrcWord = (BYTE *) pbuf;
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
        
        case ConvLum16to32: {

          WORD *pSrcWord = (WORD *) pbuf;
          DWORD *pDstWord;

          for( y=0; y<dwOrigHeight; y++,pDDSurfBytes += ddsd.lPitch) {
              pDstWord = (DWORD *)pDDSurfBytes;

              for( x=0; x<dwOrigWidth; x++,pSrcWord++,pDstWord++) {
                  // pixel buffer is in ABGR format(it stores big-endian RGBA)
                  // need to change byte order to ARGB
                  dwPixel=*pSrcWord;

                  a = dwPixel >> 8;
                  r = dwPixel & 0xFF;

                  *pDstWord = (a<<24)| (r << 16)| (r << 8) | r;
              }
          }
          break;
        }

        case ConvLum16to16_4444: {
                WORD *pSrcWord = (WORD *) pbuf;
                WORD *pDstWord;

                assert(ddsd.ddpfPixelFormat.dwRGBAlphaBitMask==0xf000);  // assumes ARGB

                for( y=0; y<dwOrigHeight; y++,pDDSurfBytes+=ddsd.lPitch) {
                   pDstWord = (WORD*)pDDSurfBytes;

                   for( x=0; x<dwOrigWidth; x++,pSrcWord++,pDstWord++) {
                       dwPixel = *pSrcWord;

                       a = (BYTE)(dwPixel>>8)           >> 4; 
                       r = (BYTE)(dwPixel & 0x000000ff) >> 4;

                       *pDstWord = (a << 12) | (r << 8)| (r << 4) | r;
                     }
                 }
                 break;
        }

        case ConvLum16to16_1555: {
                WORD *pSrcWord = (WORD *) pbuf;
                WORD *pDstWord;

                assert(ddsd.ddpfPixelFormat.dwRGBAlphaBitMask==0x8000);  // assumes ARGB

                for( y=0; y<dwOrigHeight; y++,pDDSurfBytes+=ddsd.lPitch) {
                   pDstWord = (WORD*)pDDSurfBytes;

                   for( x=0; x<dwOrigWidth; x++,pSrcWord++,pDstWord++) {
                       dwPixel = *pSrcWord;

                       r = (BYTE)(dwPixel & 0x000000ff) >> 3;

                       *pDstWord = (dwPixel & 0x8000) | (r << 10)| (r << 5) | r;
                     }
                 }
                 break;
        }

        case ConvLum16to16: {
            // AL bytes are in same order
            CopyMemory(pDDSurfBytes,pbuf,dwOrigWidth*dwOrigHeight*2);
            break;
        }

       case ConvLum8to16_0565: 
       case ConvLum8to16_0555: {
          BYTE *pSrcWord = (BYTE *) pbuf;
          WORD *pDstWord;
          DWORD FarShift,OrVal,MiddleRoundShift,GrnVal;

          if(ConvNeeded==ConvLum8to16_0555) {
             FarShift=10;  OrVal=0x8000;  // turn on alpha bit, just in case
             MiddleRoundShift = 3;
             assert(ddsd.ddpfPixelFormat.dwRBitMask==0x7C00);  
          } else {
             FarShift=11;  OrVal=0x0; 
             MiddleRoundShift = 2;
             assert(ddsd.ddpfPixelFormat.dwRBitMask==0xF800);            
          }
          
          for( y=0; y<dwOrigHeight; y++,pDDSurfBytes+=ddsd.lPitch) {
              pDstWord = (WORD*)pDDSurfBytes;

              for( x=0; x<dwOrigWidth; x++,pSrcWord++,pDstWord++) {
                  dwPixel = *pSrcWord;

                  r = (BYTE) dwPixel >> 3;
                  GrnVal = (BYTE) dwPixel >> MiddleRoundShift;
                  
                  // code truncates 8 bit values to 5 bit  (set alpha=1 for opaque)

                  *pDstWord = ((r << FarShift)| (GrnVal << 5) | r) | OrVal;
              }
          }
          break;
       }

        case ConvLum8to8: {
            CopyMemory(pDDSurfBytes,pbuf,dwOrigWidth*dwOrigHeight);
            break;
        }

        case ConvLum8to32: {

          // this is kind of a waste of space, but we trade it for lum resolution
          BYTE *pSrcWord = (BYTE *) pbuf;
          DWORD *pDstWord;

          for( y=0; y<dwOrigHeight; y++,pDDSurfBytes += ddsd.lPitch) {
              pDstWord = (DWORD *)pDDSurfBytes;

              for( x=0; x<dwOrigWidth; x++,pSrcWord++,pDstWord++) {
                  dwPixel=*pSrcWord;

                  *pDstWord = 0xFF000000 | (dwPixel<<16) | (dwPixel<<8) | dwPixel;
              }
          }
          break;
        }

        case ConvLum8to24: {

          // this is kind of a waste of space, but we trade it for lum resolution
          BYTE *pSrcWord = (BYTE *) pbuf;
          BYTE *pDstWord;

          for( y=0; y<dwOrigHeight; y++,pDDSurfBytes += ddsd.lPitch) {
              pDstWord = (BYTE *)pDDSurfBytes;

              for( x=0; x<dwOrigWidth; x++,pSrcWord++,pDstWord++) {
                  dwPixel=*pSrcWord;

                  *pDstWord++ = dwPixel;
                  *pDstWord++ = dwPixel;
                  *pDstWord   = dwPixel;
              }
          }
          break;
        }

        default:  
               dxgsg_cat.error() << "CreateTexture failed! unhandled texture conversion type: "<< ConvNeeded <<" \n";
               pDDSurf->Unlock(NULL);
               return E_INVALIDARG;
        }

     pDDSurf->Unlock(NULL);

     return S_OK;
}



//-----------------------------------------------------------------------------
// Name: CreateTextureFromBitmap()
// Desc: Use a bitmap to create a texture for the specified device. This code
//       gets the attributes of the texture from the bitmap, creates the
//       texture, and then copies the bitmap into the texture.
//-----------------------------------------------------------------------------
LPDIRECTDRAWSURFACE7 DXTextureContext::
CreateTexture( HDC PrimaryDC, LPDIRECT3DDEVICE7 pd3dDevice, int cNumTexPixFmts, LPDDPIXELFORMAT pTexPixFmts) {
    HRESULT hr;
    int i;
    PixelBuffer *pbuf = _texture->_pbuffer;
    int cNumAlphaBits;     //  number of alpha bits in texture pixfmt

    DDPIXELFORMAT *pDesiredPixFmt;
    LPDIRECTDRAWSURFACE7 pddsRender;
    LPDIRECTDRAW7        pDD = NULL;

    DDPIXELFORMAT TexFmtsArr[MAX_DX_TEXPIXFMTS];  

    ConversionType ConvNeeded;

    // make local copy of array so I can muck with it during searches for this texture fmt
    memcpy(TexFmtsArr,pTexPixFmts,cNumTexPixFmts*sizeof(DDPIXELFORMAT));
    pTexPixFmts=TexFmtsArr;
    
    // bpp indicates requested fmt, not pixbuf fmt
    DWORD bpp = get_bits_per_pixel(pbuf->get_format(), &cNumAlphaBits);
    PixelBuffer::Type pixbuf_type = pbuf->get_image_type();
    DWORD cNumColorChannels = pbuf->get_num_components();

    assert(pbuf->get_component_width()==sizeof(BYTE));   // cant handle anything else now
    assert(pixbuf_type==PixelBuffer::T_unsigned_byte);   // cant handle anything else now

    if((pixbuf_type != PixelBuffer::T_unsigned_byte) || (pbuf->get_component_width()!=1)) {
        dxgsg_cat.error() << "CreateTexture failed, havent handled non 8-bit channel pixelbuffer types yet! \n";
        return NULL;
    }
    
    DWORD dwOrigWidth  = (DWORD)pbuf->get_xsize();
    DWORD dwOrigHeight = (DWORD)pbuf->get_ysize();

    // Get the device caps so we can check if the device has any constraints
    // when using textures
    D3DDEVICEDESC7 devDesc;
    if( FAILED( pd3dDevice->GetCaps( &devDesc ) ) ) {
        goto error_exit;
    }

    // Setup the new surface desc for the texture. Note how we are using the
    // texture manage attribute, so Direct3D does alot of dirty work for us
    DDSURFACEDESC2 ddsd;
    ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );
    ddsd.dwSize          = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags         =  DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH 
                           | DDSD_PIXELFORMAT ;

    ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);

    // setup ddpf to match against avail fmts

    ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;  
    
    if((pbuf->get_format() == PixelBuffer::F_luminance_alpha)||
       (pbuf->get_format() == PixelBuffer::F_luminance_alphamask) ||
       (pbuf->get_format() == PixelBuffer::F_luminance)) {
        ddsd.ddpfPixelFormat.dwFlags = DDPF_LUMINANCE; 
    }

    ddsd.ddpfPixelFormat.dwRGBBitCount = bpp;

    if (cNumAlphaBits) {
        if (bpp == 8 && cNumAlphaBits == 8) // handle special case:  Alpha only buffer
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
        dxgsg_cat.error() << "ERROR: texture dimensions are not a power of 2 for " << _tex->get_name() << "!!!!! \n";
        #ifdef _DEBUG
          exit(1);  // want to catch badtexsize errors
        #else
          goto error_exit;
        #endif
    }

    BOOL bShrinkOriginal;

    bShrinkOriginal=FALSE;
    if((dwOrigWidth>devDesc.dwMaxTextureWidth)||(dwOrigHeight>devDesc.dwMaxTextureHeight)) {
        #ifdef _DEBUG
         dxgsg_cat.error() << "WARNING: " <<_tex->get_name() << ": Image size exceeds max texture dimensions of (" << devDesc.dwMaxTextureWidth << "," << devDesc.dwMaxTextureHeight << ") !!\n" 
                           << "Scaling "<< _tex->get_name() << " ("<< dwOrigWidth<<"," <<dwOrigHeight << ") => ("<<  devDesc.dwMaxTextureWidth << "," << devDesc.dwMaxTextureHeight << ") !\n";
        #endif
        
         if(dwOrigWidth>devDesc.dwMaxTextureWidth) 
             ddsd.dwWidth=devDesc.dwMaxTextureWidth;
         if(dwOrigHeight>devDesc.dwMaxTextureHeight)
             ddsd.dwHeight=devDesc.dwMaxTextureHeight;        
         bShrinkOriginal=TRUE;
    }

#if 0   
    // checks for SQUARE reqmt
    //riva128 seems to handle non-sq fine.  is it wasting mem to do this?  do I care or should I shrink to be sure we save mem?  
    if( (ddsd.dwWidth != ddsd.dwHeight) && (devDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY )) {

        // assume pow2 textures.   sum exponents, divide by 2 rounding down to get sq size
        int i,width_exp,height_exp;
        for(i=ddsd.dwWidth,width_exp=0;i>1;width_exp++,i>>=1);
        for(i=ddsd.dwHeight,height_exp=0;i>1;height_exp++,i>>=1);
        ddsd.dwHeight = ddsd.dwWidth = 1<<((width_exp+height_exp)>>1);
        bShrinkOriginal=TRUE;

        #ifdef _DEBUG
          dxgsg_cat.debug() << "Scaling "<< _tex->get_name() << " ("<< dwOrigWidth<<"," <<dwOrigHeight << ") => ("<< ddsd.dwWidth<<"," << ddsd.dwHeight << ") to meet HW square texture reqmt\n";
        #endif

    }
#endif

    if(bShrinkOriginal) {
        // need 2 add checks for errors 
          PNMImage pnmi_src;
          PNMImage *pnmi = new PNMImage(ddsd.dwWidth, ddsd.dwHeight, cNumColorChannels);
          pbuf->store(pnmi_src);
          pnmi->quick_filter_from(pnmi_src,0,0);

          pbuf->load(*pnmi);  // violates device independence of pixbufs

          dwOrigWidth  = (DWORD)pbuf->get_xsize();
          dwOrigHeight = (DWORD)pbuf->get_ysize();
          delete pnmi;
    }

#if 0
//#ifdef _DEBUG
// use dxcapsviewer
    { static BOOL bPrinted=FALSE;
       if(!bPrinted) {
            dxgsg_cat.debug() << "Gfx card supported TexFmts:\n";
            for(i=0;i<cNumTexPixFmts;i++) { DebugPrintPixFmt(&pTexPixFmts[i]); }
            bPrinted=TRUE;
       }
    }
#endif

    // first search for an exact match
    pDesiredPixFmt = &ddsd.ddpfPixelFormat;

    LPDDPIXELFORMAT pCurPixFmt;
    char *szErrorMsg;

    szErrorMsg = "CreateTexture failed: couldn't find compatible Tex DDPIXELFORMAT!\n";

    dxgsg_cat.spam() << "CreateTexture handling bitdepth: " << bpp << " alphabits: " << cNumAlphaBits << "\n";

    // Mark formats I dont want to deal with
    for(i=0,pCurPixFmt=pTexPixFmts;i<cNumTexPixFmts;i++,pCurPixFmt++) {
        if(  ( pCurPixFmt->dwFlags & (DDPF_BUMPLUMINANCE|DDPF_BUMPDUDV) )  ||
             ( pCurPixFmt->dwFourCC != 0 ) ||
             ((cNumAlphaBits==0) && (pCurPixFmt->dwFlags & DDPF_ALPHAPIXELS))) {
            
            // Make sure to skip any FourCC formats, bump formats
            // they are not handled by this code yet

            // note: I'm screening out alpha if no alpha requested, so
            // search fails if 32-rgba avail, but not 32-bit rgb
            // I could recode for that case too, hopefully this case will not pop up

            pCurPixFmt->dwRGBBitCount+=1;  // incr so it wont be an exact match anymore
        }
    }

    assert(((cNumColorChannels==4)||(cNumColorChannels==2))==(cNumAlphaBits>0));

    // handle each bitdepth separately

    switch(bpp) {  // bpp is REQUESTED bpp, not what exists in the pixbuf array

      case 32:

#ifdef _DEBUG
        if(!dx_force_16bpptextures) 
#endif
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

        if(cNumAlphaBits>0) {
            // no 32-bit fmt, look for 16 bit w/alpha  (1-15)

            // 32 bit RGBA was requested, but only 16 bit alpha fmts are avail
            // by default, convert to 4-4-4-4 which has 4-bit alpha for blurry edges
            // if we know tex only needs 1 bit alpha (i.e. for a mask), use 1555 instead

            ConversionType ConvTo1=Conv32to16_4444,ConvTo2=Conv32to16_1555;
            DWORD dwAlphaMask1=0xF000,dwAlphaMask2=0x8000;
            // assume ALPHAMASK is x8000 and RGBMASK is x7fff to simplify 32->16 conversion
            // this should be true on most cards.  

        #ifndef FORCE_16bpp_1555
            if(cNumAlphaBits==1)
        #endif
             {
                ConvTo1=Conv32to16_1555;
                dwAlphaMask1=0x8000;
             }
        
            for(i=0,pCurPixFmt=&pTexPixFmts[cNumTexPixFmts-1];i<cNumTexPixFmts;i++,pCurPixFmt--) {
                if((pCurPixFmt->dwRGBBitCount==16) && (pCurPixFmt->dwFlags & DDPF_ALPHAPIXELS)
                   && (pCurPixFmt->dwRGBAlphaBitMask==dwAlphaMask1)) {
                      ConvNeeded=ConvTo1;
                      goto found_matching_format;
                  }
            }

        #ifdef FORCE_16bpp_1555
                break;
        #endif

            for(i=0,pCurPixFmt=&pTexPixFmts[cNumTexPixFmts-1];i<cNumTexPixFmts;i++,pCurPixFmt--) {
                if((pCurPixFmt->dwRGBBitCount==16) && (pCurPixFmt->dwFlags & DDPF_ALPHAPIXELS)
                   && (pCurPixFmt->dwRGBAlphaBitMask==dwAlphaMask2)) {
                      ConvNeeded=ConvTo2;
                      goto found_matching_format;
                  }
            }

            // at this point, bail.  dont worry about converting to non-alpha formats yet,
            // I think this will be a very rare case
            szErrorMsg = "CreateTexture failed: couldn't find compatible Tex DDPIXELFORMAT! no available 16 or 32-bit alpha formats!\n";
        }

        break;

      case 24:

          assert(cNumAlphaBits==0);  // dont know how to handle non-zero alpha for 24bit total

#ifdef _DEBUG
        if(!dx_force_16bpptextures) 
#endif
          for(i=0,pCurPixFmt=pTexPixFmts;i<cNumTexPixFmts;i++,pCurPixFmt++) {
              if((pCurPixFmt->dwFlags & DDPF_RGB)&&(pCurPixFmt->dwRGBBitCount==24)) {
                      ConvNeeded=((cNumColorChannels==3) ? Conv24to24 : Conv32to24);
                      goto found_matching_format;
              }
          }

#ifdef _DEBUG
        if(!dx_force_16bpptextures) 
#endif
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

       if(ddsd.ddpfPixelFormat.dwFlags & DDPF_LUMINANCE) {
           // look for native lum fmt
       #ifdef _DEBUG
         if(!dx_force_16bpptextures) 
       #endif
          {

           for(i=0,pCurPixFmt=&pTexPixFmts[cNumTexPixFmts-1];i<cNumTexPixFmts;i++,pCurPixFmt--) {
               if((pCurPixFmt->dwRGBBitCount==16) && (pCurPixFmt->dwFlags & DDPF_ALPHAPIXELS) &&
                  (pCurPixFmt->dwFlags & DDPF_LUMINANCE)) {
                     ConvNeeded=ConvLum16to16;
                     goto found_matching_format;
               }
           }

           // else look for 32bpp ARGB
           for(i=0,pCurPixFmt=&pTexPixFmts[cNumTexPixFmts-1];i<cNumTexPixFmts;i++,pCurPixFmt--) {
               if((pCurPixFmt->dwRGBBitCount==32) && (pCurPixFmt->dwFlags & DDPF_ALPHAPIXELS) &&
                  (pCurPixFmt->dwFlags & DDPF_RGB)) {
                    ConvNeeded=ConvLum16to32;
                    goto found_matching_format;
               }
           }
         }

           // find compatible 16bpp fmt
            ConversionType ConvTo1=ConvLum16to16_4444,ConvTo2=ConvLum16to16_1555;
            DWORD dwAlphaMask1=0xF000,dwAlphaMask2=0x8000;
            // assume ALPHAMASK is x8000 and RGBMASK is x7fff to simplify 32->16 conversion
            // this should be true on most cards.  

        #ifndef FORCE_16bpp_1555
            if(cNumAlphaBits==1)
        #endif
             {
                ConvTo1=ConvLum16to16_1555;
                dwAlphaMask1=0x8000;
             }
        
            for(i=0,pCurPixFmt=&pTexPixFmts[cNumTexPixFmts-1];i<cNumTexPixFmts;i++,pCurPixFmt--) {
                if((pCurPixFmt->dwRGBBitCount==16) && (pCurPixFmt->dwFlags & DDPF_ALPHAPIXELS)
                   && (pCurPixFmt->dwRGBAlphaBitMask==dwAlphaMask1)) {
                      ConvNeeded=ConvTo1;
                      goto found_matching_format;
                  }
            }

        #ifdef FORCE_16bpp_1555
                break;
        #endif

            for(i=0,pCurPixFmt=&pTexPixFmts[cNumTexPixFmts-1];i<cNumTexPixFmts;i++,pCurPixFmt--) {
                if((pCurPixFmt->dwRGBBitCount==16) && (pCurPixFmt->dwFlags & DDPF_ALPHAPIXELS)
                   && (pCurPixFmt->dwRGBAlphaBitMask==dwAlphaMask2)) {
                      ConvNeeded=ConvTo2;
                      goto found_matching_format;
                  }
            }

         } else

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
          if(ddsd.ddpfPixelFormat.dwFlags & DDPF_LUMINANCE) {
              // look for native lum fmt

              assert(cNumAlphaBits==0);  // dont handle those other 8bit lum fmts like 4-4, since 16 8-8 is usually supported too
            #ifdef _DEBUG
             if(!dx_force_16bpptextures) 
            #endif
             {
              for(i=0,pCurPixFmt=&pTexPixFmts[cNumTexPixFmts-1];i<cNumTexPixFmts;i++,pCurPixFmt--) {
                  if((pCurPixFmt->dwRGBBitCount==8) && (pCurPixFmt->dwFlags & DDPF_LUMINANCE) &&
                     (pCurPixFmt->dwLuminanceBitMask=0xFF)) {
                        ConvNeeded=ConvLum8to8;
                        goto found_matching_format;
                  }
              }

              // else look for 24bpp RGB
              for(i=0,pCurPixFmt=&pTexPixFmts[cNumTexPixFmts-1];i<cNumTexPixFmts;i++,pCurPixFmt--) {
                  if((pCurPixFmt->dwRGBBitCount==24) && (pCurPixFmt->dwFlags & DDPF_RGB)) {
                       ConvNeeded=ConvLum8to24;
                       goto found_matching_format;
                  }
              }

              // else look for 32bpp RGB
              for(i=0,pCurPixFmt=&pTexPixFmts[cNumTexPixFmts-1];i<cNumTexPixFmts;i++,pCurPixFmt--) {
                  if((pCurPixFmt->dwRGBBitCount==32) && (pCurPixFmt->dwFlags & DDPF_RGB)) {
                       ConvNeeded=ConvLum8to32;
                       goto found_matching_format;
                  }
              }
             }

              // find compatible 16bpp fmt, just look for any 565, then 0555
              DWORD dwMasks[2] = {0xF800, 0x7C00 };

              for (DWORD modenum=0;modenum<2;modenum++)
                for(i=0,pCurPixFmt=&pTexPixFmts[0];i<cNumTexPixFmts;i++,pCurPixFmt++) {
                       if((pCurPixFmt->dwRGBBitCount==16) && (pCurPixFmt->dwFlags & DDPF_RGB)
                          && (!(pCurPixFmt->dwFlags & DDPF_ALPHAPIXELS))
                          && (pCurPixFmt->dwRBitMask==dwMasks[modenum]) ) {  
                             ConvNeeded=ConvLum8to16_0565;
                             goto found_matching_format;
                       }
                }
          }
          break;

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

    ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE | DDSCAPS_3DDEVICE;

    ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE  // Turn on texture management 
                         | DDSCAPS2_HINTSTATIC;    // BUGBUG:  is this ok for ALL textures?

    // validate magfilter setting
    // degrade filtering if no HW support

    Texture::FilterType ft;

    ft =_tex->get_magfilter();
    if((ft!=Texture::FT_linear) && ft!=Texture::FT_nearest) {
        if(ft=Texture::FT_nearest_mipmap_nearest)
            ft=Texture::FT_nearest;
         else ft=Texture::FT_linear;
    }

    if((ft==Texture::FT_linear) && !(devDesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEAR))
         ft=Texture::FT_nearest;
    _tex->set_magfilter(ft);

    BOOL bDoMipMaps;

    // figure out if we are mipmapping this texture
    ft =_tex->get_minfilter();
    bDoMipMaps=FALSE;

    if(!dx_ignore_mipmaps) {  // set if no HW mipmap capable
     #ifdef _DEBUG
        bDoMipMaps=dx_mipmap_everything;
     #endif
        switch(ft) {
          case Texture::FT_nearest_mipmap_nearest:
          case Texture::FT_linear_mipmap_nearest:
          case Texture::FT_nearest_mipmap_linear:  // pick nearest in each, interpolate linearly b/w them
          case Texture::FT_linear_mipmap_linear:
              bDoMipMaps=TRUE;
        }
    } else if((ft==Texture::FT_nearest_mipmap_nearest) ||   // cvt to no-mipmap filter types
           (ft==Texture::FT_nearest_mipmap_linear)) {
              ft=Texture::FT_nearest;
    } else if((ft==Texture::FT_linear_mipmap_nearest) ||
                 (ft==Texture::FT_linear_mipmap_linear)) {
                   ft=Texture::FT_linear;
    }

    assert((devDesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_NEAREST)!=0);

    switch(ft) {
        case Texture::FT_nearest_mipmap_linear:
            if(!(devDesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEARMIPNEAREST))
                ft=Texture::FT_nearest_mipmap_nearest;
            break;
        case Texture::FT_linear_mipmap_nearest:
            if(!(devDesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MIPLINEAR))
                ft=Texture::FT_nearest_mipmap_nearest;
            break;
        case Texture::FT_linear_mipmap_linear:
            if(!(devDesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEARMIPLINEAR)) {
                 if(devDesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MIPLINEAR)
                     ft=Texture::FT_linear_mipmap_nearest;
                  else ft=Texture::FT_nearest_mipmap_nearest;  // if you cant do linear in a level, you probably cant do linear b/w levels, so just do nearest-all
            }
            break;
        case Texture::FT_linear:
            if(!(devDesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEAR)) 
                 ft=Texture::FT_nearest;  
            break;
    }

    _tex->set_minfilter(ft);

    int aniso_degree;

    aniso_degree=1;
    if(devDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANISOTROPY) {
        aniso_degree=_tex->get_anisotropic_degree();
        if((aniso_degree>devDesc.dwMaxAnisotropy) 
#ifdef _DEBUG
           || dx_force_anisotropic_filtering
#endif
          )
             aniso_degree=devDesc.dwMaxAnisotropy;
    } 
    _tex->set_anisotropic_degree(aniso_degree);
    #ifdef _DEBUG
      dxgsg_cat.spam() << "CreateTexture: setting aniso degree for "<< _tex->get_name() << " to: " << aniso_degree << endl;
    #endif

    if(bDoMipMaps) {
       // We dont specify mipmapcount, so CreateSurface will auto-create surfs 
       // for all mipmaps down to 1x1 (if driver supports deep-mipmaps, otherwise Nx1)
        ddsd.ddsCaps.dwCaps |= (DDSCAPS_MIPMAP | DDSCAPS_COMPLEX); 
        dxgsg_cat.debug() << "CreateTexture: generating mipmaps for "<< _tex->get_name() << endl;
    }

    if(devDesc.dwDevCaps & D3DDEVCAPS_SEPARATETEXTUREMEMORIES) {
        // must assign a texture to a specific stage
        // for now I'm just going to use stage 0 for all
        ddsd.dwTextureStage=0;
        ddsd.dwFlags |= DDSD_TEXTURESTAGE;
    }

    PRINTVIDMEM(pDD,&ddsd.ddsCaps,"texture surf (includes AGP mem)");

    // Create a new surface for the texture
    if( FAILED( hr = pDD->CreateSurface( &ddsd, &_surface, NULL ) ) )
    {
        dxgsg_cat.error() << "CreateTexture failed: pDD->CreateSurface() failed!  hr = " << ConvD3DErrorToString(hr) << "\n";
        goto error_exit;
    }

#ifdef _DEBUG
        dxgsg_cat.debug() << "CreateTexture: "<< _tex->get_name() <<" converted " << ConvNameStrs[ConvNeeded] << " \n";    
#endif

    hr = ConvertPixBuftoDDSurf(ConvNeeded,pbuf->_image.p(),_surface);
    if(FAILED(hr)) {
        goto error_exit;
    }

    _surface->GetSurfaceDesc(&ddsd);

    if(bDoMipMaps) {
        DWORD i,oldcurxsize,oldcurysize,curxsize,curysize,cMipMapCount=ddsd.dwMipMapCount;
        assert(ddsd.dwMipMapCount<20);

        curxsize=ddsd.dwWidth; curysize=ddsd.dwHeight;

        assert(pixbuf_type==PixelBuffer::T_unsigned_byte);   // cant handle anything else now
        
        // all mipmap sublevels require 1/3 of total original space. alloc 1/2 for safety
        BYTE *pMipMapPixBufSpace = new BYTE[((curxsize*curysize*cNumColorChannels)/2)+1024];

        LPDIRECTDRAWSURFACE7 pCurDDSurf=_surface;
        pCurDDSurf->AddRef();  // so final release doesnt release the surface

        BYTE *pDstWord = pMipMapPixBufSpace;
        BYTE *pLastMipLevelStart  = (BYTE *) pbuf->_image.p();
//    clock_t  start1,finish1;
//    start1=clock();

        for(i=1;i<ddsd.dwMipMapCount;i++) {
            oldcurxsize=curxsize; oldcurysize=curysize;
            curysize = max(curysize>>1,1);
            curxsize = max(curxsize>>1,1);

            assert(!((oldcurxsize==1)&&(oldcurysize==1)));

            BYTE *pSrcWord;
            BYTE *pSrcLineStart=pLastMipLevelStart;

            // inc img start to DWORD boundary
            while(((DWORD)pDstWord) & 0x11)
               pDstWord++;

            pLastMipLevelStart = pDstWord;

            DWORD x,y,cPixelSize=cNumColorChannels;
            DWORD src_row_bytelength=oldcurxsize*cPixelSize;
            DWORD two_src_row_bytelength=2*src_row_bytelength;

            DWORD DivShift=2;
            if((oldcurxsize==1)||(oldcurysize==1))  
                DivShift = 1;
            DWORD x_srcptr_inc = ((oldcurxsize==1)? cPixelSize: (2*cPixelSize));

            // box-filter shrink down, avg 4 pixels at a time
            for( y=0; y<curysize; y++,pSrcLineStart+=two_src_row_bytelength) {
              pSrcWord=pSrcLineStart;
              for( x=0; x<curxsize; x++,pSrcWord+=x_srcptr_inc,pDstWord+=cPixelSize) {
                  // fetches, stores byte at a time.
                  // inefficient, but works for all channel sizes

                  for(int c=0;c<cPixelSize;c++) {  
                      DWORD colr;
                      colr =  *(pSrcWord+c);
                      if(oldcurxsize>1)  // handle 1x[X], [X]x1 cases
                          colr += *(pSrcWord+cPixelSize+c);
                      if(oldcurysize>1) {
                          colr += *(pSrcWord+src_row_bytelength+c);
                          if(oldcurxsize>1)
                              colr += *(pSrcWord+src_row_bytelength+cPixelSize+c);
                      }

                      colr >>= DivShift;

                      *(pDstWord+c)=(BYTE)colr;
                  }
              }
            }

            // now copy pixbuf to final DD surf

            DDSCAPS2 ddsCaps;
            LPDIRECTDRAWSURFACE7 pMipLevel_DDSurf;
            ZeroMemory(&ddsCaps,sizeof(DDSCAPS2));
            ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;
            ddsCaps.dwCaps2 = DDSCAPS2_MIPMAPSUBLEVEL;


            hr = pCurDDSurf->GetAttachedSurface(&ddsCaps, &pMipLevel_DDSurf);
            if (FAILED(hr)) {
                dxgsg_cat.error() << "CreateTexture failed creating mipmaps: GetAttachedSurf hr = " << ConvD3DErrorToString(hr) << "\n";
                pCurDDSurf->Release();
                goto error_exit;
            }

            hr = ConvertPixBuftoDDSurf(ConvNeeded,pLastMipLevelStart,pMipLevel_DDSurf);
            if(FAILED(hr)) {
                pCurDDSurf->Release();
                goto error_exit;
            }

            pCurDDSurf->Release();
            pCurDDSurf=pMipLevel_DDSurf;
        }

//    finish1=clock();
//   double elapsed_time  = (double)(finish1 - start1) / CLOCKS_PER_SEC;
//   cerr <<  "mipmap gen takes " << elapsed_time << " secs for this texture\n";
      
        pCurDDSurf->Release(); 

        delete pMipMapPixBufSpace;
       
        #ifdef _DEBUG
        if(dx_debug_view_mipmaps) {
#if 0
            if(!(ddcaps.dwCaps & DDCAPS_BLTSTRETCH)) {
                   dxgsg_cat.error() << "CreateTexture failed debug-viewing mipmaps, BLT stretching not supported!  ( we need to do SW stretch) \n";
                   goto error_exit;
            }
#endif

            // display mipmaps on primary surf
            HDC hTexDC;
            LPDIRECTDRAWSURFACE7 pTextureCurrent,pTexturePrev = _surface;
            int cury,curx;
            HDC hScreenDC;
            RECT scrnrect;        
            hScreenDC=GetDC(NULL);  
    
            scrnrect.left=scrnrect.top=0;
            scrnrect.bottom=GetDeviceCaps(hScreenDC,VERTRES);
            scrnrect.right=GetDeviceCaps(hScreenDC,HORZRES);
            char msg[500];

            pTexturePrev->AddRef();
    

            for (i = 0,curx=scrnrect.left,cury=scrnrect.top; i < ddsd.dwMipMapCount; i++) {
    
               DX_DECLARE_CLEAN(DDSURFACEDESC2, ddsd_cur);
               pTexturePrev->GetSurfaceDesc(&ddsd_cur);
    
               hr = pTexturePrev->GetDC(&hTexDC);
               if (FAILED(hr)) {
                     dxgsg_cat.error() << "GetDC failed hr = " << ConvD3DErrorToString(hr) << "\n";    
                     break;
               }
    
                BOOL res;
                // res = BitBlt(PrimaryDC,0,0,ddsd.dwWidth,ddsd.dwHeight, TexDC,0,0,SRCCOPY);
                // loader inverts y, so use StretchBlt to re-invert it
                // res = StretchBlt(PrimaryDC,0,ddsd_cur.dwHeight+cury,ddsd_cur.dwWidth,-ddsd_cur.dwHeight, TexDC,0,0,ddsd_cur.dwWidth,ddsd_cur.dwHeight,SRCCOPY);
#if 0
                //        dxgsg_cat.error() << "WINVER = " << (void*)WINVER << "\n";
                if(cNumAlphaBits>0) {
                    BLENDFUNCTION  bf;
                    bf.BlendOp = AC_SRC_OVER;  bf.BlendFlags=0; 
                    bf.SourceConstantAlpha=255; bf.AlphaFormat=AC_SRC_ALPHA;
                    res = AlphaBlend(hScreenDC,curx,cury,ddsd_cur.dwWidth,ddsd_cur.dwHeight, TexDC,0,0,ddsd_cur.dwWidth,ddsd_cur.dwHeight,bf);
                    if (!res)  {
                        PrintLastError(msg);
                        dxgsg_cat.error() << "AlphaBlend BLT failed: "<<msg<<"\n";    
                    }
    
                } else 
#endif                    
                {
                   res = StretchBlt(hScreenDC,curx,ddsd_cur.dwHeight+cury,ddsd_cur.dwWidth,-ddsd_cur.dwHeight, hTexDC,0,0,ddsd_cur.dwWidth,ddsd_cur.dwHeight,SRCCOPY);
                    if (!res) {
                         PrintLastError(msg);
                         dxgsg_cat.error() << "StretchBLT failed: "<<msg<<"\n";    
    
                    }
                }
//                SetBkMode(hScreenDC, TRANSPARENT);     
                sprintf(msg,"%d",i);
                TextOut(hScreenDC,curx+(ddsd_cur.dwWidth)/2,5+cury+ddsd_cur.dwHeight,msg,strlen(msg));
    
                curx+=max(20,ddsd_cur.dwWidth+10);

                if(curx>scrnrect.right) {
                    curx=0;  cury+=(scrnrect.bottom-scrnrect.top)/2;
                }
    
                hr = pTexturePrev->ReleaseDC(hTexDC);
        
                if (FAILED(hr)) {
                     dxgsg_cat.error() << "tex ReleaseDC failed for mip "<<i<<" hr = " << ConvD3DErrorToString(hr) << "\n";    
                     break;
                }
    
                if(i==ddsd.dwMipMapCount-1) {
                    pTexturePrev->Release();
                    continue;
                }
    
                DDSCAPS2 ddsCaps;
                ZeroMemory(&ddsCaps,sizeof(DDSCAPS2));
                ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;
                ddsCaps.dwCaps2 = DDSCAPS2_MIPMAPSUBLEVEL;
                hr = pTexturePrev->GetAttachedSurface(&ddsCaps, &pTextureCurrent); 
                if (FAILED(hr)) {
                    dxgsg_cat.error() << " failed displaying mipmaps: GetAttachedSurf hr = " << ConvD3DErrorToString(hr) << "\n";
                }
                // done with the previous texture
                pTexturePrev->Release();
                pTexturePrev = pTextureCurrent;            
            }
    
            ReleaseDC(0,hScreenDC);
    
            HANDLE hArr[1];
            MSG winmsg;
            hArr[0]=GetStdHandle(STD_INPUT_HANDLE);
            GetMessage(&winmsg,NULL,0,0);
    
            int val=MsgWaitForMultipleObjects(1,hArr,TRUE,INFINITE,QS_KEY);
            if(val==-1) {
                PrintLastError(msg);
               dxgsg_cat.error() << " MsgWaitForMultipleObjects returns " << val << "  " <<msg << endl;
            } else {
                dxgsg_cat.error() << " MsgWaitForMultipleObjects returns " << val << endl;
            }
        }
        #endif
    }

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


