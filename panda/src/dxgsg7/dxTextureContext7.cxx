// Filename: dxTextureContext7.cxx
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

#include <assert.h>
#include <time.h>
#include "dxTextureContext7.h"
#include "config_dxgsg7.h"
#include "dxGraphicsStateGuardian7.h"
#include "pnmImage.h"

static const DWORD g_LowByteMask = 0x000000FF;

//#define FORCE_16bpp_1555

#define PANDA_BGRA_ORDER

#ifdef PANDA_BGRA_ORDER
// assume Panda uses byte-order BGRA/LA to store pixels, which when read into little-endian word is ARGB/AL
// these macros GET from PixelBuffer, (wont work from DDSurface)
#define GET_RED_BYTE(PIXEL_DWORD)  ((BYTE)((PIXEL_DWORD >> 16) & g_LowByteMask))
#define GET_BLUE_BYTE(PIXEL_DWORD) ((BYTE)((PIXEL_DWORD)       & g_LowByteMask))
#else 
// otherwise Panda uses int ABGR (big-endian RGBA order), (byte-order RGBA or RGB)
#define GET_RED_BYTE(PIXEL_DWORD)  ((BYTE)(PIXEL_DWORD         & g_LowByteMask))
#define GET_BLUE_BYTE(PIXEL_DWORD) ((BYTE)((PIXEL_DWORD >> 16) & g_LowByteMask))
#endif

#define GET_GREEN_BYTE(PIXEL_DWORD) ((BYTE)((PIXEL_DWORD >> 8) & g_LowByteMask))
#define GET_ALPHA_BYTE(PIXEL_DWORD) ((BYTE)(((DWORD)PIXEL_DWORD) >> 24))  // unsigned >> shifts in 0's, so dont need to mask off upper bits

typedef enum {
    None,Conv32to32,Conv32to32_NoAlpha,Conv32to24,Conv32to16_X555,
    Conv32to16_1555,Conv32to16_0565,Conv32to16_4444,Conv24to32,Conv24to24,
    Conv24to16_X555,Conv24to16_0565,ConvLum16to16_1555,ConvLum16to16_4444,
    ConvLum16to32,ConvLum16to16,ConvLum8to8,ConvLum8to24,ConvLum8to32,ConvLum8to16_X555,ConvLum8to16_0565,
    ConvAlpha8to16_4444,ConvAlpha8to32,ConvAlpha8to8
} ConversionType;

#ifndef NDEBUG
char *ConvNameStrs[] = {"None","Conv32to32","Conv32to32_NoAlpha","Conv32to24","Conv32to16_X555",
    "Conv32to16_1555","Conv32to16_0565","Conv32to16_4444","Conv24to32","Conv24to24",
    "Conv24to16_X555","Conv24to16_0565","ConvLum16to16_1555","ConvLum16to16_4444",
    "ConvLum16to32","ConvLum16to16","ConvLum8to8","ConvLum8to24","ConvLum8to32",
    "ConvLum8to16_X555","ConvLum8to16_0565","ConvAlpha8to16_4444","ConvAlpha8to32","ConvAlpha8to8"
};
#endif

char *PandaFilterNameStrs[] = {"FT_nearest","FT_linear","FT_nearest_mipmap_nearest","FT_linear_mipmap_nearest",
    "FT_nearest_mipmap_linear", "FT_linear_mipmap_linear"
};

TypeHandle DXTextureContext7::_type_handle;

#define SWAPDWORDS(X,Y)  { DWORD temp=X;  X=Y; Y=temp; }

#ifdef _DEBUG
static void DebugPrintPixFmt(DDPIXELFORMAT* pddpf) {
    static int iddpfnum=0;
    ostream *dbgout = &dxgsg7_cat.debug();

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
//     Function: DXTextureContext7::get_bits_per_pixel
//       Access: Protected
//  Description: Maps from the PixelBuffer's Format symbols
//               to bpp.  returns # of alpha bits
//               Note: PixelBuffer's format indicates REQUESTED final format,
//                     not the stored format, which is indicated by pixelbuffer type
////////////////////////////////////////////////////////////////////

unsigned int DXTextureContext7::
get_bits_per_pixel(PixelBuffer::Format format, int *alphbits) {
    *alphbits = 0;      // assume no alpha bits
    switch(format) {
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

    if(IsBadWritePtr(pDDSurf,sizeof(DWORD))) {
        dxgsg7_cat.error() << "ConvertPixBuftoDDSurf failed: bad pDDSurf ptr value (" << ((void*)pDDSurf) << ")\n";
        exit(1);
    }

    if(FAILED( hr = pDDSurf->Lock( NULL, &ddsd,  DDLOCK_NOSYSLOCK | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL ))) {
        dxgsg7_cat.error() << "CreateTexture failed: _surface->Lock() failed on texture! hr = " << ConvD3DErrorToString(hr) << "\n";
        return hr;
    }

    //pbuf contains raw ARGB in PixelBuffer byteorder

    DWORD lPitch = ddsd.lPitch;
    BYTE* pDDSurfBytes = (BYTE*)ddsd.lpSurface;
    DWORD dwOrigWidth=ddsd.dwWidth,dwOrigHeight=ddsd.dwHeight;

    switch(ConvNeeded) {
        case Conv32to32:
        case Conv32to32_NoAlpha: {

#ifdef PANDA_BGRA_ORDER
                if(ConvNeeded==Conv32to32) {
                    memcpy(pDDSurfBytes,(BYTE*) pbuf,dwOrigWidth*dwOrigHeight*sizeof(DWORD));
                } else {
                    DWORD *pSrcWord = (DWORD *) pbuf;
                    DWORD *pDstWord;

                    // need to set all pixels alpha to 0xFF
                    for(DWORD y=0; y<dwOrigHeight; y++,pDDSurfBytes+=ddsd.lPitch) {
                        pDstWord = (DWORD*)pDDSurfBytes;
                        for(DWORD x=0; x<dwOrigWidth; x++,pSrcWord++,pDstWord++) {
                            *pDstWord = *pSrcWord | 0xFF000000;
                        }
                    }
                }
#else
                DWORD *pDstWord,*pSrcWord = (DWORD *) pbuf;
                DWORD dwAlphaMaskOn = (ConvNeeded==Conv32to32_NoAlpha) ? 0xFF000000 : 0x0;

                for(DWORD y=0; y<dwOrigHeight; y++,pDDSurfBytes+=ddsd.lPitch) {
                    pDstWord = (DWORD*)pDDSurfBytes;
                    for(DWORD x=0; x<dwOrigWidth; x++,pSrcWord++,pDstWord++) {
                        DWORD dwPixel = *pSrcWord;

                        // pixel buffer is in ABGR format(it stores big-endian RGBA)
                        // need to change byte order to ARGB

                        BYTE r,b;
                        // just swap r & b
                        b = GET_BLUE_BYTE(dwPixel);
                        r = GET_RED_BYTE(dwPixel);
                        *pDstWord = ((dwPixel & 0xff00ff00) | (r<<16) | b) | dwAlphaMaskOn;
                    }
                }
#endif
                break;
            }

        case Conv32to16_1555:
        case Conv32to16_X555: {
                DWORD *pSrcWord = (DWORD *) pbuf;
                WORD *pDstWord;

                unsigned short dwAlphaMaskOn = (ConvNeeded==Conv32to16_X555) ? 0x8000 : 0x0;

                assert(ddsd.ddpfPixelFormat.dwRBitMask==0x7C00);

                for(DWORD y=0; y<dwOrigHeight; y++,pDDSurfBytes+=ddsd.lPitch) {
                    pDstWord = (WORD*)pDDSurfBytes;

                    for(DWORD x=0; x<dwOrigWidth; x++,pSrcWord++,pDstWord++) {
                        BYTE r,g,b;
                        DWORD dwPixel = *pSrcWord;
                        unsigned short abit;

                        // just look at most-signf-bit for alpha.  (alternately, could
                        // convert any non-zero alpha to full transparent)

                        abit = ((dwPixel>>16) & 0x00008000) | dwAlphaMaskOn;  // just copy high bit
                        g = GET_GREEN_BYTE(dwPixel) >> 3;
                        b = GET_BLUE_BYTE(dwPixel) >> 3;
                        r = GET_RED_BYTE(dwPixel) >> 3;

                        // truncates 8 bit values to 5 bit (or 1 for alpha)

                        *pDstWord = (abit | (r << 10)| (g << 5) | b);
                    }
                }
                break;
            }

        case Conv32to16_0565: {   // could merge this w/above case, but whatever
                DWORD *pSrcWord = (DWORD *) pbuf;
                WORD *pDstWord;

                assert(ddsd.ddpfPixelFormat.dwRBitMask==0xF800);
                // for some reason, bits are 'in-order' when converting to 16bit

                for(DWORD y=0; y<dwOrigHeight; y++,pDDSurfBytes+=ddsd.lPitch) {
                    pDstWord = (WORD*)pDDSurfBytes;

                    for(DWORD x=0; x<dwOrigWidth; x++,pSrcWord++,pDstWord++) {
                        BYTE r,g,b;
                        DWORD dwPixel = *pSrcWord;

                        g = GET_GREEN_BYTE(dwPixel) >> 2;
                        b = GET_BLUE_BYTE(dwPixel) >> 3;
                        r = GET_RED_BYTE(dwPixel) >> 3;
                        *pDstWord = ((r << 11)| (g << 5) | b);
                    }
                }
                break;
            }

        case Conv32to16_4444: {
                DWORD *pSrcWord = (DWORD *) pbuf;
                WORD *pDstWord;

                assert(ddsd.ddpfPixelFormat.dwRGBAlphaBitMask==0xf000);  // assumes ARGB
                assert(ddsd.ddpfPixelFormat.dwRBitMask==0x0f00);  // assumes ARGB

                for(DWORD y=0; y<dwOrigHeight; y++,pDDSurfBytes+=ddsd.lPitch) {
                    pDstWord = (WORD*)pDDSurfBytes;

                    for(DWORD x=0; x<dwOrigWidth; x++,pSrcWord++,pDstWord++) {
                        BYTE r,g,b,a;
                        DWORD dwPixel = *pSrcWord;

                        a = GET_ALPHA_BYTE(dwPixel)  >> 4;
                        g = GET_GREEN_BYTE(dwPixel)  >> 4;
                        b = GET_BLUE_BYTE(dwPixel)   >> 4;
                        r = GET_RED_BYTE(dwPixel)    >> 4;

                        *pDstWord = (a << 12) | (r << 8)| (g << 4) | b;
                    }
                }
                break;
            }

        case Conv32to24: {

                DWORD *pSrcWord = (DWORD *) pbuf;
                BYTE *pDstWord;

                for(DWORD y=0; y<dwOrigHeight; y++,pDDSurfBytes+=ddsd.lPitch) {
                    pDstWord = (BYTE*)pDDSurfBytes;

                    for(DWORD x=0; x<dwOrigWidth; x++,pSrcWord++,pDstWord+=3) {
                        BYTE r,g,b;
                        DWORD dwPixel = *pSrcWord;

                        r = GET_RED_BYTE(dwPixel);
                        g = GET_GREEN_BYTE(dwPixel);
                        b = GET_BLUE_BYTE(dwPixel);

                        *pDstWord     = r;
                        *(pDstWord+1) = g;
                        *(pDstWord+2) = b;
                    }
                }
                break;
            }


        case Conv24to24: {
            #ifdef PANDA_BGRA_ORDER
                memcpy(pDDSurfBytes,(BYTE*)pbuf,dwOrigHeight*dwOrigWidth*3);
            #else
                BYTE *pSrcWord = (BYTE *) pbuf;
                BYTE *pDstWord;
            
                for(DWORD y=0; y<dwOrigHeight; y++,pDDSurfBytes += ddsd.lPitch) {
                    pDstWord = (BYTE*)pDDSurfBytes;

                    for(DWORD x=0; x<dwOrigWidth; x++,pSrcWord+=3,pDstWord+=3) {
                        BYTE r,g,b;

                        b = *pSrcWord;
                        g = *(pSrcWord+1);
                        r = *(pSrcWord+2);

                        *pDstWord     = r;
                        *(pDstWord+1) = g;
                        *(pDstWord+2) = b;
                    }
                }
             #endif
                break;
            }

        case Conv24to16_X555: {
                BYTE *pSrcWord = (BYTE *) pbuf;
                WORD *pDstWord;

                assert(ddsd.ddpfPixelFormat.dwRBitMask==0x7C00);
             // for some reason, bits are 'in-order' when converting to 16bit

                for(DWORD y=0; y<dwOrigHeight; y++,pDDSurfBytes += ddsd.lPitch) {
                    pDstWord = (WORD*)pDDSurfBytes;

                    for(DWORD x=0; x<dwOrigWidth; x++,pSrcWord+=3,pDstWord++) {
                        BYTE r,g,b;

                    #ifdef PANDA_BGRA_ORDER
                        b = *pSrcWord       >> 3;
                        r = *(pSrcWord+2)   >> 3;
                    #else
                        r = *pSrcWord       >> 3;
                        b = *(pSrcWord+2)   >> 3;                   
                    #endif
                        g = *(pSrcWord+1)   >> 3;

                        *pDstWord = 0x8000 | (r << 10)| (g << 5) | b;
                    }
                }
                break;
            }

        case Conv24to16_0565: {
                BYTE *pSrcWord = (BYTE *) pbuf;
                WORD *pDstWord;

                assert(ddsd.ddpfPixelFormat.dwRBitMask==0xF800);

                for(DWORD y=0; y<dwOrigHeight; y++,pDDSurfBytes += ddsd.lPitch) {
                    pDstWord = (WORD*)pDDSurfBytes;

                    for(DWORD x=0; x<dwOrigWidth; x++,pSrcWord+=3,pDstWord++) {
                        BYTE r,g,b;

                    #ifdef PANDA_BGRA_ORDER
                        b = *pSrcWord       >> 3;
                        g = *(pSrcWord+1)   >> 2;
                        r = *(pSrcWord+2)   >> 3;
                    #else
                        r = *pSrcWord       >> 3;
                        g = *(pSrcWord+1)   >> 2;
                        b = *(pSrcWord+2)   >> 3;
                    #endif
                     // code truncates 8 bit values to 5 bit
                     *pDstWord = (r << 11)| (g << 5) | b;                   
                    }
                }
                break;
            }

        case Conv24to32: {
                BYTE *pSrcWord = (BYTE *) pbuf;
                DWORD *pDstWord;

                for(DWORD y=0; y<dwOrigHeight; y++,pDDSurfBytes += ddsd.lPitch) {
                    pDstWord = (DWORD *)pDDSurfBytes;

                    for(DWORD x=0; x<dwOrigWidth; x++,pSrcWord+=3,pDstWord++) {
                        BYTE r,g,b;
                        // pixel buffer is in ABGR format(it stores big-endian RGBA)
                        // need to change byte order to ARGB

                    #ifdef PANDA_BGRA_ORDER
                        b = *pSrcWord;
                        r = *(pSrcWord+2);
                    #else
                        r = *pSrcWord;
                        b = *(pSrcWord+2);
                    #endif
                        g = *(pSrcWord+1);

                        *pDstWord = 0xFF000000 | (r << 16) | (g << 8) | b;
                    }
                }
                break;
            }

        case ConvLum16to32: {
                WORD *pSrcWord = (WORD *) pbuf;
                DWORD *pDstWord;

                for(DWORD y=0; y<dwOrigHeight; y++,pDDSurfBytes += ddsd.lPitch) {
                    pDstWord = (DWORD *)pDDSurfBytes;

                    for(DWORD x=0; x<dwOrigWidth; x++,pSrcWord++,pDstWord++) {
                        // pixel buffer stores LA bytes, which in a little-endian word is AL

                        DWORD dwPixel=*pSrcWord;
                        BYTE lum,a;

                        a = dwPixel >> 8;
                        lum = dwPixel & 0xFF;
                        *pDstWord = (a<<24) | lum | (lum << 8) | (lum << 16);
                    }
                }
                break;
            }

        case ConvLum16to16_4444: {
                WORD *pSrcWord = (WORD *) pbuf;
                WORD *pDstWord;

                assert(ddsd.ddpfPixelFormat.dwRGBAlphaBitMask==0xf000);  // assumes ARGB

                for(DWORD y=0; y<dwOrigHeight; y++,pDDSurfBytes+=ddsd.lPitch) {
                    pDstWord = (WORD*)pDDSurfBytes;

                    for(DWORD x=0; x<dwOrigWidth; x++,pSrcWord++,pDstWord++) {
                        DWORD dwPixel=*pSrcWord;
                        BYTE lum,a;
                        dwPixel = *pSrcWord;

                        a =   (BYTE)(dwPixel>>8)           >> 4;
                        lum = (BYTE)(dwPixel & 0x000000ff) >> 4;

                        *pDstWord = (a << 12) | lum | (lum << 4)| (lum << 8);
                    }
                }
                break;
            }

        case ConvLum16to16_1555: {
                WORD *pSrcWord = (WORD *) pbuf;
                WORD *pDstWord;

                assert(ddsd.ddpfPixelFormat.dwRGBAlphaBitMask==0x8000);  // assumes ARGB

                for(DWORD y=0; y<dwOrigHeight; y++,pDDSurfBytes+=ddsd.lPitch) {
                    pDstWord = (WORD*)pDDSurfBytes;

                    for(DWORD x=0; x<dwOrigWidth; x++,pSrcWord++,pDstWord++) {
                        WORD dwPixel=*pSrcWord;
                        BYTE lum;

                        lum = (BYTE)(dwPixel & 0x00FF) >> 3;

                        *pDstWord = (dwPixel & 0x8000) | lum | (lum << 5) | (lum << 10);
                    }
                }
                break;
            }

        case ConvLum16to16: {
                // All bytes are in same order?
                CopyMemory(pDDSurfBytes,pbuf,dwOrigWidth*dwOrigHeight*2);
                break;
            }

        case ConvLum8to16_0565:
        case ConvLum8to16_X555: {
                BYTE *pSrcWord = (BYTE *) pbuf;
                WORD *pDstWord;
                DWORD FarShift,OrVal,MiddleRoundShift;

                if(ConvNeeded==ConvLum8to16_X555) {
                    FarShift=10;  OrVal=0x8000;  // turn on alpha bit, just in case
                    MiddleRoundShift = 3;
                    assert(ddsd.ddpfPixelFormat.dwRBitMask==0x7C00);
                } else {
                    FarShift=11;  OrVal=0x0;
                    MiddleRoundShift = 2;
                    assert(ddsd.ddpfPixelFormat.dwRBitMask==0xF800);
                }

                for(DWORD y=0; y<dwOrigHeight; y++,pDDSurfBytes+=ddsd.lPitch) {
                    pDstWord = (WORD*)pDDSurfBytes;

                    for(DWORD x=0; x<dwOrigWidth; x++,pSrcWord++,pDstWord++) {
                        DWORD dwPixel=*pSrcWord,GrnVal;
                        BYTE r;

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

                for(DWORD y=0; y<dwOrigHeight; y++,pDDSurfBytes += ddsd.lPitch) {
                    pDstWord = (DWORD *)pDDSurfBytes;

                    for(DWORD x=0; x<dwOrigWidth; x++,pSrcWord++,pDstWord++) {
                        DWORD dwPixel=*pSrcWord;

                        *pDstWord = 0xFF000000 | dwPixel | (dwPixel << 8) | (dwPixel<<16);
                    }
                }
                break;
            }

        case ConvLum8to24: {
                // this is kind of a waste of space, but we trade it for lum resolution
                BYTE *pSrcWord = (BYTE *) pbuf;
                BYTE *pDstWord;

                for(DWORD y=0; y<dwOrigHeight; y++,pDDSurfBytes += ddsd.lPitch) {
                    pDstWord = (BYTE *)pDDSurfBytes;

                    for(DWORD x=0; x<dwOrigWidth; x++,pSrcWord++,pDstWord++) {
                        DWORD dwPixel=*pSrcWord;

                        *pDstWord++ = dwPixel;
                        *pDstWord++ = dwPixel;
                        *pDstWord   = dwPixel;
                    }
                }
                break;
            }

        case ConvAlpha8to32: {
              //  huge waste of space, but this may be only fmt where we get 8bits alpha resolution
                BYTE *pSrcWord = (BYTE *) pbuf;
                DWORD *pDstWord;

                for(DWORD y=0; y<dwOrigHeight; y++,pDDSurfBytes += ddsd.lPitch) {
                    pDstWord = (DWORD *)pDDSurfBytes;

                    for(DWORD x=0; x<dwOrigWidth; x++,pSrcWord++,pDstWord++) {
                        // OR alpha with full white
                        *pDstWord = (*pSrcWord << 24) | 0x00FFFFFF;
                    }
                }
                break;
            }

        case ConvAlpha8to16_4444: {
                BYTE *pSrcWord = (BYTE *) pbuf;
                WORD *pDstWord;

                assert(ddsd.ddpfPixelFormat.dwRGBAlphaBitMask==0xf000);  // assumes ARGB order

                for(DWORD y=0; y<dwOrigHeight; y++,pDDSurfBytes += ddsd.lPitch) {
                    pDstWord = (WORD*)pDDSurfBytes;

                    for(DWORD x=0; x<dwOrigWidth; x++,pSrcWord++,pDstWord++) {
                        WORD a = (BYTE)(*pSrcWord>>4);
                        *pDstWord = (a << 12) | 0x0FFF; // OR alpha with full white
                    }
                }
                break;
            }

        default:
            dxgsg7_cat.error() << "CreateTexture failed! unhandled texture conversion type: "<< ConvNeeded <<" \n";
            pDDSurf->Unlock(NULL);
            return E_INVALIDARG;
    }

    pDDSurf->Unlock(NULL);

    return S_OK;
}

HRESULT ConvertDDSurftoPixBuf(PixelBuffer *pixbuf,LPDIRECTDRAWSURFACE7 pDDSurf) {

    HRESULT hr;
    DX_DECLARE_CLEAN(DDSURFACEDESC2, ddsd);

    DWORD dwNumComponents=pixbuf->get_num_components();

    assert(pixbuf->get_component_width()==sizeof(BYTE));   // cant handle anything else now
    assert(pixbuf->get_image_type()==PixelBuffer::T_unsigned_byte);   // cant handle anything else now
    assert((dwNumComponents==3) || (dwNumComponents==4));  // cant handle anything else now

    BYTE *pbuf=pixbuf->_image.p();

    if(IsBadWritePtr(pDDSurf,sizeof(DWORD))) {
        dxgsg7_cat.error() << "ConvertDDSurftoPixBuf failed: bad pDDSurf ptr value (" << ((void*)pDDSurf) << ")\n";
        exit(1);
    }

    if(FAILED( hr = pDDSurf->Lock( NULL, &ddsd,  DDLOCK_NOSYSLOCK | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL ))) {
        dxgsg7_cat.error() << "ConvertDDSurftoPixBuf Lock() failed! hr = " << ConvD3DErrorToString(hr) << "\n";
        return hr;
    }

    DWORD dwXWindowOffset=0,dwYWindowOffset=0;
    DWORD dwCopyWidth=ddsd.dwWidth,dwCopyHeight=ddsd.dwHeight;

   // get window offset so we know where to start grabbing pixels.  note for
   // fullscreen primary no clipper will be attached, that 'error' should be ignored
    LPDIRECTDRAWCLIPPER pDDClipper;
    hr = pDDSurf->GetClipper(&pDDClipper);

#ifdef _DEBUG
    if(FAILED(hr) && !((hr == DDERR_NOCLIPPERATTACHED) && dx_full_screen)) {
        dxgsg7_cat.error() << "ConvertDDSurftoPixBuf GetClipper failed! hr = " << ConvD3DErrorToString(hr) << "\n";
        return hr;
    }
#endif

    if(hr==S_OK) {
        HWND hWin;

        if(FAILED(hr = pDDClipper->GetHWnd(&hWin))) {
            dxgsg7_cat.error() << "ConvertDDSurftoPixBuf GetHwnd failed! hr = " << ConvD3DErrorToString(hr) << "\n";
            return hr;
        }

        RECT view_rect;
        GetClientRect( hWin, &view_rect );
        ClientToScreen( hWin, (POINT*)&view_rect.left );
        ClientToScreen( hWin, (POINT*)&view_rect.right );

        dwXWindowOffset=view_rect.left;
        dwYWindowOffset=view_rect.top;
        dwCopyWidth=view_rect.right-view_rect.left;
        dwCopyHeight=view_rect.bottom-view_rect.top;

        pDDClipper->Release();  // dec ref cnt
    }

    //make sure there's enough space in the pixbuf, its size must match (especially xsize)
   // or scanlines will be too long

    if(!((dwCopyWidth==pixbuf->get_xsize()) && (dwCopyHeight<=(DWORD)pixbuf->get_ysize()))) {
        pDDSurf->Unlock(NULL);
        assert(0);
        dxgsg7_cat.error() << "ConvertDDSurftoPixBuf, PixBuf incorrect size to hold display surface!\n";
        return E_FAIL;
    }

    // others not handled yet
    assert((ddsd.ddpfPixelFormat.dwRGBBitCount==32)||(ddsd.ddpfPixelFormat.dwRGBBitCount==16)||(ddsd.ddpfPixelFormat.dwRGBBitCount==24));

    //pbuf contains raw ARGB in PixelBuffer byteorder

    DWORD lPitch = ddsd.lPitch;
    BYTE* pDDSurfBytes = (BYTE*)ddsd.lpSurface;

    // writes out last line in DDSurf first in PixelBuf, so Y line order precedes inversely

    if(dxgsg7_cat.is_debug())
        dxgsg7_cat.debug() << "ConvertDDSurftoPixBuf converting " << ddsd.ddpfPixelFormat.dwRGBBitCount << "bpp DDSurf to " 
                          <<  dwNumComponents << "-channel panda PixelBuffer\n";


    if(dwNumComponents==4) {
        DWORD *pDstWord = (DWORD *) pbuf;
        switch(ddsd.ddpfPixelFormat.dwRGBBitCount) {

            case 32: {
                    DWORD *pSrcWord;
                   #ifdef PANDA_BGRA_ORDER 
                    BYTE *pDstLine = (BYTE*)pDstWord;
                   #endif

                    pDDSurfBytes+=ddsd.lPitch*(dwYWindowOffset+dwCopyHeight-1);
                    for(DWORD y=0; y<dwCopyHeight; y++,pDDSurfBytes-=ddsd.lPitch) {
                        pSrcWord = ((DWORD*)pDDSurfBytes)+dwXWindowOffset;
                        #ifdef PANDA_BGRA_ORDER 
                            memcpy(pDstLine,pSrcWord,ddsd.lPitch);
                            pDstLine+=ddsd.lPitch;
                        #else
                            for(DWORD x=0; x<dwCopyWidth; x++,pSrcWord++,pDstWord++) {
                                  DWORD dwPixel = *pSrcWord;
                                  BYTE r,b;
                                
                                  // DDsurf is in ARGB
                                  r=  (BYTE) ((dwPixel >> 16) & g_LowByteMask);
                                  b = (BYTE)  (dwPixel & g_LowByteMask);

                                  // want to write out ABGR
                                  *pDstWord = (dwPixel & 0xFF00FF00) | (b<<16) | r;
                            }
                        #endif
                    }
                    break;
                }

            case 24: {
                    BYTE *pSrcByte;

                    pDDSurfBytes+=ddsd.lPitch*(dwYWindowOffset+dwCopyHeight-1);
                    for(DWORD y=0; y<dwCopyHeight; y++,pDDSurfBytes-=ddsd.lPitch) {
                        pSrcByte = pDDSurfBytes+dwXWindowOffset*3*sizeof(BYTE);
                        for(DWORD x=0; x<dwCopyWidth; x++,pDstWord++) {
                            DWORD r,g,b;

                            b = *pSrcByte++;
                            g = *pSrcByte++;
                            r = *pSrcByte++;

                            #ifdef PANDA_BGRA_ORDER                             
                               *pDstWord = 0xFF000000 | (r << 16) | (g << 8) | b;
                            #else
                               *pDstWord = 0xFF000000 | (b << 16) | (g << 8) | r;                           
                            #endif
                        }
                    }
                    break;
                }

            case 16: {
                    assert((ddsd.ddpfPixelFormat.dwRBitMask==0xF800)  ||  // 565
                           (ddsd.ddpfPixelFormat.dwRBitMask==0x0F00)  ||  // 4444
                           (ddsd.ddpfPixelFormat.dwRBitMask==0x7C00));    // 555 or 1555

                    WORD  *pSrcWord;
                    // handle 0555,1555,0565 in same loop
                    BYTE redshift,greenshift,blueshift;
                    DWORD redmask,greenmask,bluemask;

                    if(ddsd.ddpfPixelFormat.dwRBitMask==0xF800) {
                        redshift=(11-3);
                        redmask=0xF800;
                        greenmask=0x07E0;
                        greenshift=(5-2);
                        bluemask=0x001F;
                        blueshift=3;
                    } else if(ddsd.ddpfPixelFormat.dwRBitMask==0x7C00) {
                        redmask=0x7C00;
                        redshift=(10-3);
                        greenmask=0x03E0;
                        greenshift=(5-3);
                        bluemask=0x001F;
                        blueshift=3;
                    } else {
                        assert(ddsd.ddpfPixelFormat.dwRBitMask==0x0F00);
                        redmask=0x0F00;
                        redshift=4;
                        greenmask=0x00F0;
                        greenshift=0;
                        bluemask=0x000F;
                        blueshift=4;
                    }

                    pDDSurfBytes+=ddsd.lPitch*(dwYWindowOffset+dwCopyHeight-1);
                    for(DWORD y=0; y<dwCopyHeight; y++,pDDSurfBytes-=ddsd.lPitch) {
                        pSrcWord = ((WORD*)pDDSurfBytes)+dwXWindowOffset;
                        for(DWORD x=0; x<dwCopyWidth; x++,pSrcWord++,pDstWord++) {
                            WORD dwPixel = *pSrcWord;
                            BYTE r,g,b;

                            b = (dwPixel & bluemask) << blueshift;
                            g = (dwPixel & greenmask) >> greenshift;
                            r = (dwPixel & redmask) >> redshift;

                            #ifdef PANDA_BGRA_ORDER
                              *pDstWord = 0xFF000000 | (r << 16) | (g << 8) | b;
                            #else
                              *pDstWord = 0xFF000000 | (b << 16) | (g << 8) | r;   
                            #endif
                        }
                    }
                }
                break;
        }
    } else {  // convert to 24bpp pixbuf
        BYTE *pDstByte = (BYTE *) pbuf;
        switch(ddsd.ddpfPixelFormat.dwRGBBitCount) {

            case 32: {
                    DWORD *pSrcWord;

                    pDDSurfBytes+=ddsd.lPitch*(dwYWindowOffset+dwCopyHeight-1);
                    for(DWORD y=0; y<dwCopyHeight; y++,pDDSurfBytes-=ddsd.lPitch) {
                        pSrcWord = ((DWORD*)pDDSurfBytes)+dwXWindowOffset;

                        for(DWORD x=0; x<dwCopyWidth; x++,pSrcWord++) {
                            BYTE r,g,b;
                            DWORD dwPixel = *pSrcWord;

                            r = (BYTE)((dwPixel>>16) & g_LowByteMask);
                            g = (BYTE)((dwPixel>> 8) & g_LowByteMask);
                            b = (BYTE)((dwPixel    ) & g_LowByteMask);

                            #ifdef PANDA_BGRA_ORDER
                                *pDstByte++ = b;
                                *pDstByte++ = g;
                                *pDstByte++ = r;                            
                            #else
                                *pDstByte++ = r;
                                *pDstByte++ = g;
                                *pDstByte++ = b;
                            #endif
                        }
                    }
                    break;
                }

            case 24: {
                        BYTE *pSrcByte;

                        pDDSurfBytes+=ddsd.lPitch*(dwYWindowOffset+dwCopyHeight-1);
                        for(DWORD y=0; y<dwCopyHeight; y++,pDDSurfBytes-=ddsd.lPitch) {
                            pSrcByte = pDDSurfBytes+dwXWindowOffset*3*sizeof(BYTE);
                         #ifdef PANDA_BGRA_ORDER 
                            memcpy(pDstByte,pSrcByte,ddsd.lPitch);
                            pDstByte+=ddsd.lPitch;
                         #else
                            for(DWORD x=0; x<dwCopyWidth; x++) {
                                BYTE r,g,b;

                                // pixel buffer is in ABGR format(it stores big-endian RGBA)
                                // need to change byte order from ARGB

                                b = *pSrcByte++;
                                g = *pSrcByte++;
                                r = *pSrcByte++;

                                *pDstByte++ = r;
                                *pDstByte++ = g;
                                *pDstByte++ = b;
                            }
                          #endif
                        }
                        break;
                    }

            case 16: {  // handle 555,1555,565,4444 cases  (not 8-8 yet)

                    assert((ddsd.ddpfPixelFormat.dwRBitMask==0xF800)  ||  // 565
                           (ddsd.ddpfPixelFormat.dwRBitMask==0x0F00)  ||  // 4444
                           (ddsd.ddpfPixelFormat.dwRBitMask==0x7C00));    // 555 or 1555

                    WORD  *pSrcWord;
                        // handle 0555,1555,0565 in same loop
                    BYTE redshift,greenshift,blueshift;
                    DWORD redmask,greenmask,bluemask;

                    if(ddsd.ddpfPixelFormat.dwRBitMask==0xF800) {
                        redshift=(11-3);
                        redmask=0xF800;
                        greenmask=0x07E0;
                        greenshift=(5-2);
                        bluemask=0x001F;
                        blueshift=3;
                    } else if(ddsd.ddpfPixelFormat.dwRBitMask==0x7C00) {
                        redmask=0x7C00;
                        redshift=(10-3);
                        greenmask=0x03E0;
                        greenshift=(5-3);
                        bluemask=0x001F;
                        blueshift=3;
                    } else {
                        assert(ddsd.ddpfPixelFormat.dwRBitMask==0x0F00);
                        redmask=0x0F00;
                        redshift=4;
                        greenmask=0x00F0;
                        greenshift=0;
                        bluemask=0x000F;
                        blueshift=4;
                    }

                    pDDSurfBytes+=ddsd.lPitch*(dwYWindowOffset+dwCopyHeight-1);
                    for(DWORD y=0; y<dwCopyHeight; y++,pDDSurfBytes-=ddsd.lPitch) {
                        pSrcWord = ((WORD*)pDDSurfBytes)+dwXWindowOffset;
                        for(DWORD x=0; x<dwCopyWidth; x++,pSrcWord++) {
                            WORD dwPixel = *pSrcWord;
                            BYTE r,g,b;

                            b = (dwPixel & bluemask) << blueshift;
                            g = (dwPixel & greenmask) >> greenshift;
                            r = (dwPixel & redmask) >> redshift;

                            #ifdef PANDA_BGRA_ORDER
                                *pDstByte++ = b;
                                *pDstByte++ = g;
                                *pDstByte++ = r;
                            #else
                                *pDstByte++ = r;
                                *pDstByte++ = g;
                                *pDstByte++ = b;                            
                            #endif
                        }
                    }
                }
                break;
        }
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
LPDIRECTDRAWSURFACE7 DXTextureContext7::CreateTexture(LPDIRECT3DDEVICE7 pd3dDevice, 
#ifdef USE_TEXFMTVEC
                                        DDPixelFormatVec &TexFmts,LPD3DDEVICEDESC7 pD3DDevDesc)
#else
                                        int cNumTexPixFmts, DDPIXELFORMAT *pTexFmts,LPD3DDEVICEDESC7 pD3DDevDesc)
#endif
   {
    HRESULT hr;
    int i,cNumAlphaBits;     //  number of alpha bits in texture pixfmt
    DDPIXELFORMAT *pDesiredPixFmt;
    LPDIRECTDRAWSURFACE7 pddsRender;
    LPDIRECTDRAW7        pDD = NULL;
    ConversionType ConvNeeded;

    assert(_texture!=NULL);

    PixelBuffer *pbuf = _texture->_pbuffer;

#ifdef USE_TEXFMTVEC
    int cNumTexPixFmts=TexturePixelFormats.size();
#endif
    DDPIXELFORMAT *pTexPixFmts = new DDPIXELFORMAT[cNumTexPixFmts];

    // make local copy of array so I can muck with it during searches for this texture fmt
    // (such as marking pixfmts that no search will be interested in)
    // probably should do this faster way

#ifdef USE_TEXFMTVEC
    memcpy(pTexPixFmts,&TexturePixelFormats[0],cNumTexPixFmts*sizeof(DDPIXELFORMAT));
#else
    memcpy(pTexPixFmts,pTexFmts,cNumTexPixFmts*sizeof(DDPIXELFORMAT));
#endif

    // bpp indicates requested fmt, not pixbuf fmt
    DWORD bpp = get_bits_per_pixel(pbuf->get_format(), &cNumAlphaBits);
    PixelBuffer::Type pixbuf_type = pbuf->get_image_type();
    DWORD cNumColorChannels = pbuf->get_num_components();

    assert(pbuf->get_component_width()==sizeof(BYTE));   // cant handle anything else now
    assert(pixbuf_type==PixelBuffer::T_unsigned_byte);   // cant handle anything else now

    if((pixbuf_type != PixelBuffer::T_unsigned_byte) || (pbuf->get_component_width()!=1)) {
        dxgsg7_cat.error() << "CreateTexture failed, havent handled non 8-bit channel pixelbuffer types yet! \n";
        return NULL;
    }

    DWORD dwOrigWidth  = (DWORD)pbuf->get_xsize();
    DWORD dwOrigHeight = (DWORD)pbuf->get_ysize();

    // Use the device caps so we can check if the device has any constraints
    // when using textures. 

    assert((pD3DDevDesc->dwMaxTextureWidth>0) && (pD3DDevDesc->dwMaxTextureHeight>0));

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

    if(cNumAlphaBits) {
        if(bpp == 8 && cNumAlphaBits == 8) { // handle special case:  Alpha only buffer
            ddsd.dwFlags |= DDPF_ALPHA;
            ddsd.dwAlphaBitDepth = 8;
            ddsd.ddpfPixelFormat.dwAlphaBitDepth = 8;
            ddsd.ddpfPixelFormat.dwFlags = DDPF_ALPHA;
            ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0xff;
        } else {
            ddsd.ddpfPixelFormat.dwFlags |= DDPF_ALPHAPIXELS;
            if(cNumAlphaBits == 8)
                ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0xff000000;
            else if(cNumAlphaBits == 4)
                ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0xf000;
        }
    }

    ddsd.dwWidth         = dwOrigWidth;
    ddsd.dwHeight        = dwOrigHeight;

    if(!ISPOW2(ddsd.dwWidth) || !ISPOW2(ddsd.dwHeight)) {
        dxgsg7_cat.error() << "ERROR: texture dimensions are not a power of 2 for " << _tex->get_name() << "!!!!! \n";
        #ifdef _DEBUG
          exit(1);  // want to catch badtexsize errors
        #else
          goto error_exit;
        #endif
    }

    bool bShrinkOriginal;
    bShrinkOriginal=false;

    if((dwOrigWidth>pD3DDevDesc->dwMaxTextureWidth)||(dwOrigHeight>pD3DDevDesc->dwMaxTextureHeight)) {
        #ifdef _DEBUG
           dxgsg7_cat.error() << "WARNING: " <<_tex->get_name() << ": Image size exceeds max texture dimensions of (" << pD3DDevDesc->dwMaxTextureWidth << "," << pD3DDevDesc->dwMaxTextureHeight << ") !!\n"
           << "Scaling "<< _tex->get_name() << " ("<< dwOrigWidth<<"," <<dwOrigHeight << ") => ("<<  pD3DDevDesc->dwMaxTextureWidth << "," << pD3DDevDesc->dwMaxTextureHeight << ") !\n";
        #endif

        if(dwOrigWidth>pD3DDevDesc->dwMaxTextureWidth)
            ddsd.dwWidth=pD3DDevDesc->dwMaxTextureWidth;
        if(dwOrigHeight>pD3DDevDesc->dwMaxTextureHeight)
            ddsd.dwHeight=pD3DDevDesc->dwMaxTextureHeight;
        bShrinkOriginal=true;
    }

    // checks for SQUARE reqmt (nvidia riva128 needs this)
    if((ddsd.dwWidth != ddsd.dwHeight) && (pD3DDevDesc->dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY )) {

        // assume pow2 textures.   sum exponents, divide by 2 rounding down to get sq size
        int i,width_exp,height_exp;
        for(i=ddsd.dwWidth,width_exp=0;i>1;width_exp++,i>>=1);
        for(i=ddsd.dwHeight,height_exp=0;i>1;height_exp++,i>>=1);
        ddsd.dwHeight = ddsd.dwWidth = 1<<((width_exp+height_exp)>>1);
        bShrinkOriginal=true;

#ifdef _DEBUG
        dxgsg7_cat.debug() << "Scaling "<< _tex->get_name() << " ("<< dwOrigWidth<<"," <<dwOrigHeight << ") => ("<< ddsd.dwWidth<<"," << ddsd.dwHeight << ") to meet HW square texture reqmt\n";
#endif
    }

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
// easier to use dxcapsviewer instead of this
    { static BOOL bPrinted=FALSE;
        if(!bPrinted) {
            dxgsg7_cat.debug() << "Gfx card supported TexFmts:\n";
            for(i=0;i<cNumTexPixFmts;i++) {
                DebugPrintPixFmt(&pTexPixFmts[i]);
            }
            bPrinted=TRUE;
        }
    }
#endif

    // first search for an exact match
    pDesiredPixFmt = &ddsd.ddpfPixelFormat;

    LPDDPIXELFORMAT pCurPixFmt;
    char *szErrorMsg;

    szErrorMsg = "CreateTexture failed: couldn't find compatible Tex DDPIXELFORMAT!\n";

    if(dxgsg7_cat.is_spam())
        dxgsg7_cat.spam() << "CreateTexture handling bitdepth: " << bpp << " alphabits: " << cNumAlphaBits << "\n";

    // Mark formats I dont want to deal with
    for(i=0,pCurPixFmt=pTexPixFmts;i<cNumTexPixFmts;i++,pCurPixFmt++) {
        if(( pCurPixFmt->dwFlags & (DDPF_BUMPLUMINANCE|DDPF_BUMPDUDV) )  ||
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

            if(!dx_force_16bpptextures)
                for(i=0,pCurPixFmt=pTexPixFmts;i<cNumTexPixFmts;i++,pCurPixFmt++) {
                    if((pCurPixFmt->dwFlags & DDPF_RGB)&&(pCurPixFmt->dwRGBBitCount==24)) {
                        ConvNeeded=((cNumColorChannels==3) ? Conv24to24 : Conv32to24);
                        goto found_matching_format;
                    }
                }

            if(!dx_force_16bpptextures) {
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
            }

          // no 24-bit or 32 fmt.  look for 16 bit fmt
            for(i=0,pCurPixFmt=&pTexPixFmts[cNumTexPixFmts-1];i<cNumTexPixFmts;i++,pCurPixFmt--) {
              // assume RGBMASK is x7fff to simplify 32->16 conversion, should be true on most cards.
                if((pCurPixFmt->dwFlags & DDPF_RGB) && (pCurPixFmt->dwRGBBitCount==16)
                   && !(pCurPixFmt->dwFlags & DDPF_ALPHAPIXELS)) {  // no alpha fmts
                        // if numchan==4,this means user has requested we throw away the input alpha channel
                    if(pCurPixFmt->dwGBitMask==0x7E0) {
                            // assumes GBitMask is the biggest one, if we have 16 bit 3-channel
                        ConvNeeded=((cNumColorChannels==3) ? Conv24to16_0565 : Conv32to16_0565);
                    } else {
                        assert((pCurPixFmt->dwRBitMask|pCurPixFmt->dwGBitMask|pCurPixFmt->dwBBitMask)==0x7FFF);
                        ConvNeeded=((cNumColorChannels==3) ? Conv24to16_X555 : Conv32to16_X555);
                    }
                    goto found_matching_format;
                }
            }

          // at this point, bail.
            break;

        case 16:

            if(ddsd.ddpfPixelFormat.dwFlags & DDPF_LUMINANCE) {
           // look for native lum fmt
                if(!dx_force_16bpptextures) {
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
                                        ConvNeeded=((cNumColorChannels==3) ? Conv24to16_X555 : Conv32to16_X555);
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
                if(!dx_force_16bpptextures)
                {
                    for(i=0,pCurPixFmt=&pTexPixFmts[cNumTexPixFmts-1];i<cNumTexPixFmts;i++,pCurPixFmt--) {
                        if((pCurPixFmt->dwRGBBitCount==8) && (pCurPixFmt->dwFlags & DDPF_LUMINANCE) &&
                           (pCurPixFmt->dwLuminanceBitMask==0xFF)) {
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
                DWORD dwMasks[2] = {0xF800, 0x7C00};
                ConversionType ConvType[2] = {ConvLum8to16_0565,ConvLum8to16_X555};

                for(DWORD modenum=0;modenum<2;modenum++)
                    for(i=0,pCurPixFmt=&pTexPixFmts[0];i<cNumTexPixFmts;i++,pCurPixFmt++) {
                        if((pCurPixFmt->dwRGBBitCount==16) && (pCurPixFmt->dwFlags & DDPF_RGB)
                           && (!(pCurPixFmt->dwFlags & DDPF_ALPHAPIXELS))
                           && (pCurPixFmt->dwRBitMask==dwMasks[modenum])) {
                            ConvNeeded=ConvType[modenum];
                            goto found_matching_format;
                        }
                    }
            } else if(ddsd.ddpfPixelFormat.dwFlags & DDPF_ALPHA) {
                // look for 32-bit ARGB, else 16-4444.
                // skip 8bit alpha only, because I think only voodoo supports it
                // and the voodoo support isn't the kind of blending model we need 
                // w/color assumed to be white (but need to verify this)
                for(i=0,pCurPixFmt=&pTexPixFmts[cNumTexPixFmts-1];i<cNumTexPixFmts;i++,pCurPixFmt--) {
                        if((pCurPixFmt->dwRGBBitCount==32) && (pCurPixFmt->dwFlags & DDPF_RGB) &&
                           (pCurPixFmt->dwFlags & DDPF_ALPHAPIXELS)) {
                            ConvNeeded=ConvAlpha8to32;
                            goto found_matching_format;
                        }
                }

                for(i=0,pCurPixFmt=&pTexPixFmts[cNumTexPixFmts-1];i<cNumTexPixFmts;i++,pCurPixFmt--) {
                        if((pCurPixFmt->dwRGBBitCount==16) 
                           && (pCurPixFmt->dwFlags & DDPF_RGB)
                           && (pCurPixFmt->dwFlags & DDPF_ALPHAPIXELS)
                           && (pCurPixFmt->dwRGBAlphaBitMask==0xF000)) {
                            ConvNeeded=ConvAlpha8to16_4444;
                            goto found_matching_format;
                        }
                }
            }
            break;

        default:
            szErrorMsg = "CreateTexture failed: unhandled pixel bitdepth in DX loader";
    }

    // if we've gotten here, haven't found a match

    dxgsg7_cat.error() << szErrorMsg << ";  requested tex bitdepth: " << bpp << "\n";
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

    if(pD3DDevDesc->dwDevCaps & D3DDEVCAPS_HWRASTERIZATION) {
        ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE  // Turn on texture management
                               | DDSCAPS2_HINTSTATIC;  // BUGBUG:  is this ok for ALL textures?
    } else {
        ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
    }

    // validate magfilter setting
    // degrade filtering if no HW support

    Texture::FilterType ft;

    ft =_tex->get_magfilter();
    if((ft!=Texture::FT_linear) && ft!=Texture::FT_nearest) {
        if(ft==Texture::FT_nearest_mipmap_nearest)
            ft=Texture::FT_nearest;
        else ft=Texture::FT_linear;
    }

    if((ft==Texture::FT_linear) && !(pD3DDevDesc->dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEAR))
        ft=Texture::FT_nearest;
    _tex->set_magfilter(ft);

    // figure out if we are mipmapping this texture
    ft =_tex->get_minfilter();
    _bHasMipMaps=FALSE;

    if(!dx_ignore_mipmaps) {  // set if no HW mipmap capable
        switch(ft) {
            case Texture::FT_nearest_mipmap_nearest:
            case Texture::FT_linear_mipmap_nearest:
            case Texture::FT_nearest_mipmap_linear:  // pick nearest in each, interpolate linearly b/w them
            case Texture::FT_linear_mipmap_linear:
                _bHasMipMaps=TRUE;
        }

        if(dx_mipmap_everything) {  // debug toggle, ok to leave in since its just a creation cost
           _bHasMipMaps=TRUE;
           if(ft != Texture::FT_linear_mipmap_linear) {
               dxgsg7_cat.spam() << "Forcing mipmap filtering on DX texture [" << _tex->get_name() << "]\n";
           }
           ft = Texture::FT_linear_mipmap_linear;
           _tex->set_minfilter(ft);
        }
    } else if((ft==Texture::FT_nearest_mipmap_nearest) ||   // cvt to no-mipmap filter types
              (ft==Texture::FT_nearest_mipmap_linear)) {
        ft=Texture::FT_nearest;
    } else if((ft==Texture::FT_linear_mipmap_nearest) ||
              (ft==Texture::FT_linear_mipmap_linear)) {
        ft=Texture::FT_linear;
    }

    assert((pD3DDevDesc->dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_NEAREST)!=0);

    switch(ft) {
        case Texture::FT_nearest_mipmap_linear:
            if(!(pD3DDevDesc->dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEARMIPNEAREST))
                ft=Texture::FT_nearest_mipmap_nearest;
            break;
        case Texture::FT_linear_mipmap_nearest:
            if(!(pD3DDevDesc->dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MIPLINEAR))
                ft=Texture::FT_nearest_mipmap_nearest;
            break;
        case Texture::FT_linear_mipmap_linear:
            if(!(pD3DDevDesc->dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEARMIPLINEAR)) {
                if(pD3DDevDesc->dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MIPLINEAR)
                    ft=Texture::FT_linear_mipmap_nearest;
                else ft=Texture::FT_nearest_mipmap_nearest;  // if you cant do linear in a level, you probably cant do linear b/w levels, so just do nearest-all
            }
            break;
        case Texture::FT_linear:
            if(!(pD3DDevDesc->dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEAR))
                ft=Texture::FT_nearest;
            break;
    }

    _tex->set_minfilter(ft);

    uint aniso_degree;

    aniso_degree=1;
    if(pD3DDevDesc->dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANISOTROPY) {
        aniso_degree=_tex->get_anisotropic_degree();
        if((aniso_degree>pD3DDevDesc->dwMaxAnisotropy)
#ifdef _DEBUG
           || dx_force_anisotropic_filtering
#endif
          )
            aniso_degree=pD3DDevDesc->dwMaxAnisotropy;
    }
    _tex->set_anisotropic_degree(aniso_degree);
#ifdef _DEBUG
    dxgsg7_cat.spam() << "CreateTexture: setting aniso degree for "<< _tex->get_name() << " to: " << aniso_degree << endl;
#endif

    if(_bHasMipMaps) {
       // We dont specify mipmapcount, so CreateSurface will auto-create surfs
       // for all mipmaps down to 1x1 (if driver supports deep-mipmaps, otherwise Nx1)
        ddsd.ddsCaps.dwCaps |= (DDSCAPS_MIPMAP | DDSCAPS_COMPLEX);
        dxgsg7_cat.debug() << "CreateTexture: generating mipmaps for "<< _tex->get_name() << endl;
    }

    if(pD3DDevDesc->dwDevCaps & D3DDEVCAPS_SEPARATETEXTUREMEMORIES) {
        // must assign a texture to a specific stage
        // for now I'm just going to use stage 0 for all
        ddsd.dwTextureStage=0;
        ddsd.dwFlags |= DDSD_TEXTURESTAGE;
    }

    PRINTVIDMEM(pDD,&ddsd.ddsCaps,"texture surf (includes AGP mem)");

    // Create a new surface for the texture
    if(FAILED( hr = pDD->CreateSurface( &ddsd, &_surface, NULL ) )) {
        dxgsg7_cat.error() << "CreateTexture failed: pDD->CreateSurface() failed!  hr = " << ConvD3DErrorToString(hr) << "\n";
        goto error_exit;
    }


#ifdef _DEBUG
    dxgsg7_cat.debug() << "CreateTexture: "<< _tex->get_name() <<" converted " << ConvNameStrs[ConvNeeded] << " \n";
#endif

    _PixBufConversionType=ConvNeeded;

    hr = FillDDSurfTexturePixels();
    if(FAILED(hr)) {
        goto error_exit;
    }

    // Done with DDraw
    pDD->Release();

    delete [] pTexPixFmts;

    // Return the newly created texture
    return _surface;

  error_exit:

    if(pDD!=NULL)
        pDD->Release();
    if(_surface!=NULL) {
        _surface->Release();
        _surface = NULL;
    }

    delete [] pTexPixFmts;
    return NULL;
}

HRESULT DXTextureContext7::
FillDDSurfTexturePixels(void) {
    
    PixelBuffer *pbuf = _texture->get_ram_image();
    if (pbuf == (PixelBuffer *)NULL) {
      dxgsg7_cat.fatal() << "CreateTexture: get_ram_image() failed\n";
      // The texture doesn't have an image to load.
      return E_FAIL;
    }

    HRESULT hr = ConvertPixBuftoDDSurf((ConversionType)_PixBufConversionType,pbuf->_image.p(),_surface);
    if(FAILED(hr)) {
        return hr;
    }

    DX_DECLARE_CLEAN(DDSURFACEDESC2, ddsd);

    _surface->GetSurfaceDesc(&ddsd);

    if(_bHasMipMaps) {
        DWORD i,oldcurxsize,oldcurysize,curxsize,curysize,cMipMapCount=ddsd.dwMipMapCount;
        assert(ddsd.dwMipMapCount<20);

        DWORD cNumColorChannels = pbuf->get_num_components();

        curxsize=ddsd.dwWidth; curysize=ddsd.dwHeight;

        assert(pbuf->get_image_type()==PixelBuffer::T_unsigned_byte);    // cant handle anything else now

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

        #define GENMIPMAP_DO_INTEGER_DIV    // should be a little faster, but no rounding up
        #ifdef GENMIPMAP_DO_INTEGER_DIV
            DWORD DivShift=2;
            if((oldcurxsize==1)||(oldcurysize==1))
                DivShift = 1;
        #else
            float numpixels_per_filter=4.0f;
            if((oldcurxsize==1)||(oldcurysize==1))
                numpixels_per_filter=2.0f;                
        #endif

            DWORD x_srcptr_inc = ((oldcurxsize==1)? cPixelSize: (2*cPixelSize));

            // box-filter shrink down, avg 4 pixels at a time
            for(y=0; y<curysize; y++,pSrcLineStart+=two_src_row_bytelength) {
                pSrcWord=pSrcLineStart;
                for(x=0; x<curxsize; x++,pSrcWord+=x_srcptr_inc,pDstWord+=cPixelSize) {
                  // fetches, stores byte at a time.
                  // inefficient, but works for all channel sizes

                    for(DWORD c=0;c<cPixelSize;c++) {
                        DWORD colr;
                        colr =  *(pSrcWord+c);
                        if(oldcurxsize>1)  // handle 1x[X], [X]x1 cases
                            colr += *(pSrcWord+cPixelSize+c);
                        if(oldcurysize>1) {
                            colr += *(pSrcWord+src_row_bytelength+c);
                            if(oldcurxsize>1)
                                colr += *(pSrcWord+src_row_bytelength+cPixelSize+c);
                        }
                        #ifdef GENMIPMAP_DO_INTEGER_DIV
                           colr >>= DivShift;
                        #else
                           colr = (DWORD) ((((float)colr)/numpixels_per_filter)+0.5f);
                        #endif

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
            if(FAILED(hr)) {
                dxgsg7_cat.error() << "CreateTexture failed creating mipmaps: GetAttachedSurf hr = " << ConvD3DErrorToString(hr) << "\n";
                delete [] pMipMapPixBufSpace;
                pCurDDSurf->Release();
                return hr;
            }

            hr = ConvertPixBuftoDDSurf((ConversionType)_PixBufConversionType,pLastMipLevelStart,pMipLevel_DDSurf);
            if(FAILED(hr)) {
                delete [] pMipMapPixBufSpace;
                pCurDDSurf->Release();
                return hr;
            }

            pCurDDSurf->Release();
            pCurDDSurf=pMipLevel_DDSurf;
        }

        //   finish1=clock();
        //   double elapsed_time  = (double)(finish1 - start1) / CLOCKS_PER_SEC;
        //   cerr <<  "mipmap gen takes " << elapsed_time << " secs for this texture\n";

        delete [] pMipMapPixBufSpace;
        pCurDDSurf->Release();

#ifdef _DEBUG
        if(dx_debug_view_mipmaps) {
#if 0
            if(!(ddcaps.dwCaps & DDCAPS_BLTSTRETCH)) {
                dxgsg7_cat.error() << "CreateTexture failed debug-viewing mipmaps, BLT stretching not supported!  ( we need to do SW stretch) \n";
                return hr;
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

            for(i = 0,curx=scrnrect.left,cury=scrnrect.top; i < ddsd.dwMipMapCount; i++) {

                DX_DECLARE_CLEAN(DDSURFACEDESC2, ddsd_cur);
                pTexturePrev->GetSurfaceDesc(&ddsd_cur);

                hr = pTexturePrev->GetDC(&hTexDC);
                if(FAILED(hr)) {
                    dxgsg7_cat.error() << "GetDC failed hr = " << ConvD3DErrorToString(hr) << "\n";
                    break;
                }

                BOOL res;
                        // res = BitBlt(_dxgsg7->_hdc,0,0,ddsd.dwWidth,ddsd.dwHeight, TexDC,0,0,SRCCOPY);
                        // loader inverts y, so use StretchBlt to re-invert it
                        // res = StretchBlt(_dxgsg7->hdc,0,ddsd_cur.dwHeight+cury,ddsd_cur.dwWidth,-ddsd_cur.dwHeight, TexDC,0,0,ddsd_cur.dwWidth,ddsd_cur.dwHeight,SRCCOPY);
#if 0
                if(cNumAlphaBits>0) {
                    BLENDFUNCTION  bf;
                    bf.BlendOp = AC_SRC_OVER;  bf.BlendFlags=0;
                    bf.SourceConstantAlpha=255; bf.AlphaFormat=AC_SRC_ALPHA;
                    res = AlphaBlend(hScreenDC,curx,cury,ddsd_cur.dwWidth,ddsd_cur.dwHeight, TexDC,0,0,ddsd_cur.dwWidth,ddsd_cur.dwHeight,bf);
                    if(!res) {
                        PrintLastError(msg);
                        dxgsg7_cat.error() << "AlphaBlend BLT failed: "<<msg<<"\n";
                    }

                } else
#endif
                {
                    res = StretchBlt(hScreenDC,curx,ddsd_cur.dwHeight+cury,ddsd_cur.dwWidth,-((int)ddsd_cur.dwHeight), hTexDC,0,0,ddsd_cur.dwWidth,ddsd_cur.dwHeight,SRCCOPY);
                    if(!res) {
                        PrintLastError(msg);
                        dxgsg7_cat.error() << "StretchBLT failed: "<<msg<<"\n";

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

                if(FAILED(hr)) {
                    dxgsg7_cat.error() << "tex ReleaseDC failed for mip "<<i<<" hr = " << ConvD3DErrorToString(hr) << "\n";
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
                if(FAILED(hr)) {
                    dxgsg7_cat.error() << " failed displaying mipmaps: GetAttachedSurf hr = " << ConvD3DErrorToString(hr) << "\n";
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
                dxgsg7_cat.error() << " MsgWaitForMultipleObjects returns " << val << "  " <<msg << endl;
            } else {
                dxgsg7_cat.error() << " MsgWaitForMultipleObjects returns " << val << endl;
            }
        }
#endif
    }
    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: DeleteTexture()
// Desc: Release the surface used to store the texture
//-----------------------------------------------------------------------------
void DXTextureContext7::
DeleteTexture( ) {
    if(dxgsg7_cat.is_spam()) {
        dxgsg7_cat.spam() << "Deleting DX texture for " << _tex->get_name() << "\n";
    }

    ULONG refcnt;

#ifdef DEBUG_RELEASES
    if(_surface) {
        LPDIRECTDRAW7 pDD;
        _surface->GetDDInterface( (VOID**)&pDD );
        pDD->Release();

        PRINTREFCNT(pDD,"before DeleteTex, IDDraw7");
        RELEASE(_surface,dxgsg7,"texture",false);
        PRINTREFCNT(pDD,"after DeleteTex, IDDraw7");
    }
#else
    RELEASE(_surface,dxgsg7,"texture",false);
#endif
}


////////////////////////////////////////////////////////////////////
//     Function: DXTextureContext7::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXTextureContext7::
DXTextureContext7(Texture *tex) :
TextureContext(tex) {
//#ifdef NDEBUG
    if(dxgsg7_cat.is_spam()) {
       dxgsg7_cat.spam() << "Creating DX texture [" << tex->get_name() << "], minfilter(" << PandaFilterNameStrs[tex->get_minfilter()] << "), magfilter("<<PandaFilterNameStrs[tex->get_magfilter()] << "), anisodeg(" << tex->get_anisotropic_degree() << ")\n";
    }
//#endif
    _surface = NULL;
    _bHasMipMaps = FALSE;
    _tex = tex;
}

DXTextureContext7::
~DXTextureContext7() {
    DeleteTexture();
    TextureContext::~TextureContext();
    _tex = NULL;
}

