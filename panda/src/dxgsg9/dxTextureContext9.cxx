// Filename: dxTextureContext8.cxx
// Created by:   masad (02Jan04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <time.h>
#include "dxTextureContext9.h"
#include "config_dxgsg9.h"
#include "dxGraphicsStateGuardian9.h"
//#include "pnmImage.h"
#include "d3dx9tex.h"

//#define FORCE_16bpp_1555
static const DWORD g_LowByteMask = 0x000000FF;

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

#ifdef DO_CUSTOM_CONVERSIONS
typedef enum {
    None,Conv32to32,Conv32to32_NoAlpha,Conv32to24,Conv32to16_X555,
    Conv32to16_1555,Conv32to16_0565,Conv32to16_4444,Conv24to32,Conv24to24,
    Conv24to16_X555,Conv24to16_0565,ConvLum16to16_1555,ConvLum16to16_4444,
    ConvLum16to32,ConvLum16to16,ConvLum8to8,ConvLum8to24,ConvLum8to32,ConvLum8to16_X555,ConvLum8to16_0565,ConvLum8to16_A8L8,
    ConvAlpha8to16_4444,ConvAlpha8to32,ConvAlpha8to8,ConvAlpha8to16_A8L8
} ConversionType;

#ifndef NDEBUG
char *ConvNameStrs[] = {"None","Conv32to32","Conv32to32_NoAlpha","Conv32to24","Conv32to16_X555",
    "Conv32to16_1555","Conv32to16_0565","Conv32to16_4444","Conv24to32","Conv24to24","Conv24to16_X555","Conv24to16_0565",
    "ConvLum16to16_1555","ConvLum16to16_4444","ConvLum16to32","ConvLum16to16","ConvLum8to8","ConvLum8to24","ConvLum8to32",
    "ConvLum8to16_X555","ConvLum8to16_0565","ConvLum8to16_A8L8",
    "ConvAlpha8to16_4444","ConvAlpha8to32","ConvAlpha8to8","ConvAlpha8to16_A8L8"
};
#endif
#endif

char *PandaFilterNameStrs[] = {"FT_nearest","FT_linear","FT_nearest_mipmap_nearest","FT_linear_mipmap_nearest",
    "FT_nearest_mipmap_linear", "FT_linear_mipmap_linear"
};


TypeHandle DXTextureContext9::_type_handle;

#define SWAPDWORDS(X,Y)  { DWORD temp=X;  X=Y; Y=temp; }

#ifdef _DEBUG
/*
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
*/
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
//     Function: DXTextureContext9::get_bits_per_pixel
//       Access: Protected
//  Description: Maps from the PixelBuffer's Format symbols
//               to bpp.  returns # of alpha bits
//               Note: PixelBuffer's format indicates REQUESTED final format,
//                     not the stored format, which is indicated by pixelbuffer type
////////////////////////////////////////////////////////////////////

unsigned int DXTextureContext9::
get_bits_per_pixel(PixelBuffer::Format format, int *alphbits) {
    *alphbits = 0;      // assume no alpha bits
    switch(format) {
        case PixelBuffer::F_alpha:
            *alphbits = 8;
        case PixelBuffer::F_color_index:
        case PixelBuffer::F_red:
        case PixelBuffer::F_green:
        case PixelBuffer::F_blue:
        case PixelBuffer::F_rgb332:
            return 8;
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
        case PixelBuffer::F_rgbm:
            if(format==PixelBuffer::F_rgbm)   // does this make any sense?
             *alphbits = 1;
            else *alphbits = 8;
            return 32;
        case PixelBuffer::F_rgb12:
            return 36;
        case PixelBuffer::F_rgba12:
            *alphbits = 12;
            return 48;
    }
    return 8;
}

/*   // This is superseded by D3DXLoadSurfaceFromMemory(), but keep this stuff around in case its needed

#ifdef DO_CUSTOM_CONVERSIONS
HRESULT ConvertPixBuftoDDSurf(ConversionType ConvNeeded,BYTE *pbuf,LPDIRECTDRAWSURFACE7 pDDSurf) {
    HRESULT hr;
    DX_DECLARE_CLEAN(DDSURFACEDESC2, ddsd);

    if(IsBadWritePtr(pDDSurf,sizeof(DWORD))) {
        dxgsg_cat.error() << "ConvertPixBuftoDDSurf failed: bad pDDSurf ptr value (" << ((void*)pDDSurf) << ")\n";
        exit(1);
    }

    if(FAILED( hr = pDDSurf->Lock( NULL, &ddsd,  DDLOCK_NOSYSLOCK | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL ))) {
        dxgsg_cat.error() << "CreateTexture failed: _surface->Lock() failed on texture! hr = " << ConvD3DErrorToString(hr) << "\n";
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
                        // pixel buffer is in ABGR format(it stores big-endian RGBA)
                        // need to change byte order to ARGB
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

       case ConvLum8to16_A8L8: {
                // wastes space, since alpha is just fully opaque, but Lum-only may not be avail
                BYTE *pSrcWord = (BYTE *) pbuf;
                WORD *pDstWord;

                for(DWORD y=0; y<dwOrigHeight; y++,pDDSurfBytes += ddsd.lPitch) {
                    pDstWord = (DWORD *)pDDSurfBytes;

                    for(DWORD x=0; x<dwOrigWidth; x++,pSrcWord++,pDstWord++) {
                        *pDstWord = 0xFF00 | *pSrcWord;   // add fully-opaque alpha
                    }
                }
                break;
        }

        case ConvAlpha8to16_A8L8: {
            // need to investigate why alpha-only A8 surfaces dont work on voodoo's
                BYTE *pSrcWord = (BYTE *) pbuf;
                WORD *pDstWord;

                for(DWORD y=0; y<dwOrigHeight; y++,pDDSurfBytes += ddsd.lPitch) {
                    pDstWord = (DWORD *)pDDSurfBytes;

                    for(DWORD x=0; x<dwOrigWidth; x++,pSrcWord++,pDstWord++) {
                        *pDstWord = (*pSrcWord << 8) | 0x00FF;   // add full white
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
            dxgsg_cat.error() << "CreateTexture failed! unhandled texture conversion type: "<< ConvNeeded <<" \n";
            pDDSurf->Unlock(NULL);
            return E_INVALIDARG;
    }

    pDDSurf->Unlock(NULL);

    return S_OK;
}
#endif
*/

// still need custom conversion since d3d/d3dx has no way to convert arbitrary fmt to ARGB in-memory user buffer
HRESULT ConvertD3DSurftoPixBuf(RECT &SrcRect,IDirect3DSurface9 *pD3DSurf9,PixelBuffer *pixbuf) {
// copies SrcRect in pD3DSurf to upper left of pixbuf
    HRESULT hr;
    DWORD dwNumComponents=pixbuf->get_num_components();

    assert(pixbuf->get_component_width()==sizeof(BYTE));   // cant handle anything else now
    assert(pixbuf->get_image_type()==PixelBuffer::T_unsigned_byte);   // cant handle anything else now
    assert((dwNumComponents==3) || (dwNumComponents==4));  // cant handle anything else now
    assert(IS_VALID_PTR(pD3DSurf9));

    BYTE *pbuf=pixbuf->_image.p();

    if(IsBadWritePtr(pD3DSurf9,sizeof(DWORD))) {
        dxgsg9_cat.error() << "ConvertDDSurftoPixBuf failed: bad pD3DSurf ptr value (" << ((void*)pD3DSurf9) << ")\n";
        exit(1);
    }

    DWORD dwXWindowOffset,dwYWindowOffset;
    DWORD dwCopyWidth,dwCopyHeight;

    D3DLOCKED_RECT LockedRect;
    D3DSURFACE_DESC SurfDesc;

    hr = pD3DSurf9->GetDesc(&SurfDesc);

    dwXWindowOffset=SrcRect.left,dwYWindowOffset=SrcRect.top;
    dwCopyWidth=RECT_XSIZE(SrcRect);
    dwCopyHeight=RECT_YSIZE(SrcRect);

    //make sure there's enough space in the pixbuf, its size must match (especially xsize)
   // or scanlines will be too long

    if(!((dwCopyWidth==pixbuf->get_xsize()) && (dwCopyHeight<=(DWORD)pixbuf->get_ysize()))) {
        dxgsg9_cat.error() << "ConvertDDSurftoPixBuf, PixBuf size too small to hold display surface!\n";
        assert(0);
        return E_FAIL;
    }

    hr = pD3DSurf9->LockRect(&LockedRect,(CONST RECT*)NULL,(D3DLOCK_READONLY | D3DLOCK_NO_DIRTY_UPDATE /* | D3DLOCK_NOSYSLOCK */));
    if(FAILED(hr)) {
        dxgsg9_cat.error() << "ConvertDDSurftoPixBuf LockRect() failed!" << D3DERRORSTRING(hr);
        return hr;
    }

    // ones not listed not handled yet
    assert((SurfDesc.Format==D3DFMT_A8R8G8B8)||(SurfDesc.Format==D3DFMT_X8R8G8B8)||(SurfDesc.Format==D3DFMT_R8G8B8)||
           (SurfDesc.Format==D3DFMT_R5G6B5)||(SurfDesc.Format==D3DFMT_X1R5G5B5)||(SurfDesc.Format==D3DFMT_A1R5G5B5)||
           (SurfDesc.Format==D3DFMT_A4R4G4B4));

    //pbuf contains raw ARGB in PixelBuffer byteorder

    DWORD BytePitch = LockedRect.Pitch;
    BYTE* pSurfBytes = (BYTE*)LockedRect.pBits;

    // writes out last line in DDSurf first in PixelBuf, so Y line order precedes inversely

    if(dxgsg9_cat.is_debug()) {
        dxgsg9_cat.debug() << "ConvertD3DSurftoPixBuf converting " << D3DFormatStr(SurfDesc.Format) << "bpp DDSurf to "
                          <<  dwNumComponents << "-channel panda PixelBuffer\n";
    }

    DWORD *pDstWord = (DWORD *) pbuf;
    BYTE *pDstByte = (BYTE *) pbuf;

    switch(SurfDesc.Format) {
        case D3DFMT_A8R8G8B8:
        case D3DFMT_X8R8G8B8: {
            if(dwNumComponents==4) {
                    DWORD *pSrcWord;
                   #ifdef PANDA_BGRA_ORDER
                    BYTE *pDstLine = (BYTE*)pDstWord;
                   #endif

                        pSurfBytes+=BytePitch*(dwYWindowOffset+dwCopyHeight-1);
                        for(DWORD y=0; y<dwCopyHeight; y++,pSurfBytes-=BytePitch) {
                            pSrcWord = ((DWORD*)pSurfBytes)+dwXWindowOffset;
                            #ifdef PANDA_BGRA_ORDER
                                memcpy(pDstLine,pSrcWord,BytePitch);
                                pDstLine+=BytePitch;
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
            } else {
                // 24bpp pixbuf case (numComponents==3)
                DWORD *pSrcWord;
                pSurfBytes+=BytePitch*(dwYWindowOffset+dwCopyHeight-1);
                for(DWORD y=0; y<dwCopyHeight; y++,pSurfBytes-=BytePitch) {
                    pSrcWord = ((DWORD*)pSurfBytes)+dwXWindowOffset;

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
            }
            break;
        }

        case D3DFMT_R8G8B8: {
                BYTE *pSrcByte;
                pSurfBytes+=BytePitch*(dwYWindowOffset+dwCopyHeight-1);

                if(dwNumComponents==4) {
                    for(DWORD y=0; y<dwCopyHeight; y++,pSurfBytes-=BytePitch) {
                        pSrcByte = pSurfBytes+dwXWindowOffset*3*sizeof(BYTE);
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
                } else {
                    // 24bpp pixbuf case (numComponents==3)
                    for(DWORD y=0; y<dwCopyHeight; y++,pSurfBytes-=BytePitch) {
                        pSrcByte = pSurfBytes+dwXWindowOffset*3*sizeof(BYTE);
                     #ifdef PANDA_BGRA_ORDER
                        memcpy(pDstByte,pSrcByte,BytePitch);
                        pDstByte+=BytePitch;
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
                }
                break;
        }

        case D3DFMT_R5G6B5:
        case D3DFMT_X1R5G5B5:
        case D3DFMT_A1R5G5B5:
        case D3DFMT_A4R4G4B4: {
                WORD  *pSrcWord;
                // handle 0555,1555,0565,4444 in same loop

                BYTE redshift,greenshift,blueshift;
                DWORD redmask,greenmask,bluemask;

                if(SurfDesc.Format==D3DFMT_R5G6B5) {
                    redshift=(11-3);
                    redmask=0xF800;
                    greenmask=0x07E0;
                    greenshift=(5-2);
                    bluemask=0x001F;
                    blueshift=3;
                } else if(SurfDesc.Format==D3DFMT_A4R4G4B4) {
                    redmask=0x0F00;
                    redshift=4;
                    greenmask=0x00F0;
                    greenshift=0;
                    bluemask=0x000F;
                    blueshift=4;
                } else {  // 1555 or x555
                    redmask=0x7C00;
                    redshift=(10-3);
                    greenmask=0x03E0;
                    greenshift=(5-3);
                    bluemask=0x001F;
                    blueshift=3;
                }

                pSurfBytes+=BytePitch*(dwYWindowOffset+dwCopyHeight-1);
                if(dwNumComponents==4) {
                    // Note: these 16bpp loops ignore input alpha completely (alpha is set to fully opaque in pixbuf!)
                    //       if we need to capture alpha, probably need to make separate loops for diff 16bpp fmts
                    //       for best speed

                    for(DWORD y=0; y<dwCopyHeight; y++,pSurfBytes-=BytePitch) {
                        pSrcWord = ((WORD*)pSurfBytes)+dwXWindowOffset;
                        for(DWORD x=0; x<dwCopyWidth; x++,pSrcWord++,pDstWord++) {
                            WORD dwPixel = *pSrcWord;
                            BYTE r,g,b;

                            b = (dwPixel & bluemask) << blueshift;
                            g = (dwPixel & greenmask) >> greenshift;
                            r = (dwPixel & redmask) >> redshift;

                            // alpha is just set to 0xFF

                            #ifdef PANDA_BGRA_ORDER
                              *pDstWord = 0xFF000000 | (r << 16) | (g << 8) | b;
                            #else
                              *pDstWord = 0xFF000000 | (b << 16) | (g << 8) | r;
                            #endif
                        }
                    }
                } else {
                    // 24bpp pixbuf case (numComponents==3)
                    for(DWORD y=0; y<dwCopyHeight; y++,pSurfBytes-=BytePitch) {
                        pSrcWord = ((WORD*)pSurfBytes)+dwXWindowOffset;
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

        default:
            dxgsg9_cat.error() << "ConvertD3DSurftoPixBuf: unsupported D3DFORMAT!\n";
    }

    pD3DSurf9->UnlockRect();
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CreateTexture()
// Desc: Use panda texture's pixelbuffer to create a texture for the specified device.
//       This code gets the attributes of the texture from the bitmap, creates the
//       texture, and then copies the bitmap into the texture.
//-----------------------------------------------------------------------------
IDirect3DTexture9 *DXTextureContext9::CreateTexture(DXScreenData &scrn) {
    HRESULT hr;
    int cNumAlphaBits;     //  number of alpha bits in texture pixfmt
    D3DFORMAT TargetPixFmt=D3DFMT_UNKNOWN;
    bool bNeedLuminance = false;

    assert(IS_VALID_PTR(_texture));

    PixelBuffer *pbuf = _texture->_pbuffer;
    // bpp indicates requested fmt, not pixbuf fmt
    DWORD target_bpp = get_bits_per_pixel(pbuf->get_format(), &cNumAlphaBits);
    PixelBuffer::Type pixbuf_type = pbuf->get_image_type();
    DWORD cNumColorChannels = pbuf->get_num_components();

    assert(pbuf->get_component_width()==sizeof(BYTE));   // cant handle anything else now
    assert(pixbuf_type==PixelBuffer::T_unsigned_byte);   // cant handle anything else now

    //PRINT_REFCNT(dxgsg9,scrn.pD3D9);

    if((pixbuf_type!=PixelBuffer::T_unsigned_byte) || (pbuf->get_component_width()!=1)) {
        dxgsg9_cat.error() << "CreateTexture failed, havent handled non 8-bit channel pixelbuffer types yet! \n";
        return NULL;
    }

    DWORD dwOrigWidth  = (DWORD)pbuf->get_xsize();
    DWORD dwOrigHeight = (DWORD)pbuf->get_ysize();

    if((pbuf->get_format() == PixelBuffer::F_luminance_alpha)||
       (pbuf->get_format() == PixelBuffer::F_luminance_alphamask) ||
       (pbuf->get_format() == PixelBuffer::F_luminance)) {
        bNeedLuminance = true;
    }

    if(cNumAlphaBits>0) {
        if(cNumColorChannels==3) {
            dxgsg9_cat.error() << "ERROR: texture " << _tex->get_name() << " has no inherent alpha channel, but alpha format is requested (that would be wasteful)!\n";
            exit(1);
        }
    }

    _PixBufD3DFmt=D3DFMT_UNKNOWN;

#ifndef DO_CUSTOM_CONVERSIONS
    // figure out what 'D3DFMT' the PixelBuffer is in, so D3DXLoadSurfFromMem knows how to perform copy

    switch(cNumColorChannels) {
        case 1:
            if(cNumAlphaBits>0)
                _PixBufD3DFmt=D3DFMT_A8;
            else if(bNeedLuminance)
                   _PixBufD3DFmt=D3DFMT_L8;
            break;
        case 2:
            assert(bNeedLuminance && (cNumAlphaBits>0));
            _PixBufD3DFmt=D3DFMT_A8L8;
            break;
        case 3:
            _PixBufD3DFmt=D3DFMT_R8G8B8;
            break;
        case 4:
            _PixBufD3DFmt=D3DFMT_A8R8G8B8;
            break;
    }

    // make sure we handled all the possible cases
    assert(_PixBufD3DFmt!=D3DFMT_UNKNOWN);
#endif

    DWORD TargetWidth=dwOrigWidth;
    DWORD TargetHeight=dwOrigHeight;

    if(!ISPOW2(dwOrigWidth) || !ISPOW2(dwOrigHeight)) {
        dxgsg9_cat.error() << "ERROR: texture dimensions are not a power of 2 for " << _tex->get_name() << "! Please rescale them so it doesnt have to be done at runtime.\n";
        #ifndef NDEBUG
          exit(1);  // want to catch badtexsize errors
        #else
          goto error_exit;
        #endif
    }

    bool bShrinkOriginal;
    bShrinkOriginal=false;

    if((dwOrigWidth>scrn.d3dcaps.MaxTextureWidth)||(dwOrigHeight>scrn.d3dcaps.MaxTextureHeight)) {
        #ifdef _DEBUG
           dxgsg9_cat.error() << "WARNING: " <<_tex->get_name() << ": Image size exceeds max texture dimensions of (" << scrn.d3dcaps.MaxTextureWidth << "," << scrn.d3dcaps.MaxTextureHeight << ") !!\n"
           << "Scaling "<< _tex->get_name() << " ("<< dwOrigWidth<<"," <<dwOrigHeight << ") => ("<<  scrn.d3dcaps.MaxTextureWidth << "," << scrn.d3dcaps.MaxTextureHeight << ") !\n";
        #endif

        if(dwOrigWidth>scrn.d3dcaps.MaxTextureWidth)
            TargetWidth=scrn.d3dcaps.MaxTextureWidth;
        if(dwOrigHeight>scrn.d3dcaps.MaxTextureHeight)
            TargetHeight=scrn.d3dcaps.MaxTextureHeight;
        bShrinkOriginal=true;
    }

    // checks for SQUARE reqmt (nvidia riva128 needs this)
    if((TargetWidth != TargetHeight) && (scrn.d3dcaps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY)) {
        // assume pow2 textures.   sum exponents, divide by 2 rounding down to get sq size
        int i,width_exp,height_exp;
        for(i=TargetWidth,width_exp=0;i>1;width_exp++,i>>=1);
        for(i=TargetHeight,height_exp=0;i>1;height_exp++,i>>=1);
        TargetHeight = TargetWidth = 1<<((width_exp+height_exp)>>1);
        bShrinkOriginal=true;

#ifdef _DEBUG
        dxgsg9_cat.debug() << "Scaling "<< _tex->get_name() << " ("<< dwOrigWidth<<"," <<dwOrigHeight << ") => ("<< TargetWidth<<"," << TargetHeight << ") to meet HW square texture reqmt\n";
#endif
    }
/*
    // we now use D3DXLoadSurfFromMem to do resizing as well as fmt conversion
    if(bShrinkOriginal) {
        // need 2 add checks for errors
        PNMImage pnmi_src;
        PNMImage *pnmi = new PNMImage(TargetWidth, TargetHeight, cNumColorChannels);
        pbuf->store(pnmi_src);
        pnmi->quick_filter_from(pnmi_src,0,0);

        pbuf->load(*pnmi);  // violates device independence of pixbufs

        dwOrigWidth  = (DWORD)pbuf->get_xsize();
        dwOrigHeight = (DWORD)pbuf->get_ysize();
        delete pnmi;
    }
*/

    char *szErrorMsg;

    szErrorMsg = "CreateTexture failed: couldn't find compatible device Texture Pixel Format for input texture";

    if(dxgsg9_cat.is_spam())
        dxgsg9_cat.spam() << "CreateTexture handling target bitdepth: " << target_bpp << " alphabits: " << cNumAlphaBits << endl;

    // I could possibly replace some of this logic with D3DXCheckTextureRequirements(), but
    // it wouldnt handle all my specialized low-memory cases perfectly

#ifdef DO_CUSTOM_CONVERSIONS
    ConversionType ConvNeeded;

#define CONVTYPE_STMT ConvNeeded=CONV
#else
#define CONVTYPE_STMT
#endif

#define CHECK_FOR_FMT(FMT,CONV)  \
                    if(scrn.SupportedTexFmtsMask & FMT##_FLAG) {   \
                        CONVTYPE_STMT;                             \
                        TargetPixFmt=D3DFMT_##FMT;                 \
                        goto found_matching_format; }

    // handle each target bitdepth separately.  might be less confusing to reorg by cNumColorChannels (input type, rather
    // than desired 1st target)
    switch(target_bpp) {

    // IMPORTANT NOTE:
    // target_bpp is REQUESTED bpp, not what exists in the pixbuf array (the pixbuf array contains cNumColorChannels*8bits)

        case 32:
            if(!((cNumColorChannels==3) || (cNumColorChannels==4)))
                break; //bail

            if(!dx_force_16bpptextures) {
                if(cNumColorChannels==4) {
                    CHECK_FOR_FMT(A8R8G8B8,Conv32to32);
                } else {
                    CHECK_FOR_FMT(A8R8G8B8,Conv24to32);
                }
            }

            if(cNumAlphaBits>0) {
                assert(cNumColorChannels==4);

            // no 32-bit fmt, look for 16 bit w/alpha  (1-15)

            // 32 bit RGBA was requested, but only 16 bit alpha fmts are avail
            // by default, convert to 4-4-4-4 which has 4-bit alpha for blurry edges
            // if we know tex only needs 1 bit alpha (i.e. for a mask), use 1555 instead

//                ConversionType ConvTo1=Conv32to16_4444,ConvTo2=Conv32to16_1555;
//                DWORD dwAlphaMask1=0xF000,dwAlphaMask2=0x8000;
            // assume ALPHAMASK is x8000 and RGBMASK is x7fff to simplify 32->16 conversion
            // this should be true on most cards.

#ifndef FORCE_16bpp_1555
                if(cNumAlphaBits==1)
#endif
                {
                    CHECK_FOR_FMT(A1R5G5B5,Conv32to16_1555);
                }

                // normally prefer 4444 due to better alpha channel resolution
                CHECK_FOR_FMT(A4R4G4B4,Conv32to16_4444);
                CHECK_FOR_FMT(A1R5G5B5,Conv32to16_1555);

                // at this point, bail.  dont worry about converting to non-alpha formats yet,
                // I think this will be a very rare case
                szErrorMsg = "CreateTexture failed: couldn't find compatible Tex DDPIXELFORMAT! no available 16 or 32-bit alpha formats!";
            } else {
                // convert 3 or 4 channel to closest 16bpp color fmt

                if(cNumColorChannels==3) {
                    CHECK_FOR_FMT(R5G6B5,Conv24to16_4444);
                    CHECK_FOR_FMT(X1R5G5B5,Conv24to16_X555);
                } else {
                    CHECK_FOR_FMT(R5G6B5,Conv32to16_4444);
                    CHECK_FOR_FMT(X1R5G5B5,Conv32to16_X555);
                }
            }
            break;

        case 24:
            assert(cNumColorChannels==3);

            if(!dx_force_16bpptextures) {
                CHECK_FOR_FMT(R8G8B8,Conv24to24);

                // no 24-bit fmt.  look for 32 bit fmt  (note: this is memory-hogging choice
                // instead I could look for memory-conserving 16-bit fmt).

                CHECK_FOR_FMT(X8R8G8B8,Conv24to32);
            }

             // no 24-bit or 32 fmt.  look for 16 bit fmt (higher res 565 1st)
            CHECK_FOR_FMT(R5G6B5,Conv24to16_0565);
            CHECK_FOR_FMT(X1R5G5B5,Conv24to16_X555);
            break;

        case 16:
            if(bNeedLuminance) {
                assert(cNumAlphaBits>0);
                assert(cNumColorChannels==2);

                CHECK_FOR_FMT(A8L8,ConvLum16to16);

                if(!dx_force_16bpptextures) {
                    CHECK_FOR_FMT(A8R8G8B8,ConvLum16to32);
                }

              #ifndef FORCE_16bpp_1555
                if(cNumAlphaBits==1)
              #endif
                {
                    CHECK_FOR_FMT(A1R5G5B5,ConvLum16to16_1555);
                }

                // normally prefer 4444 due to better alpha channel resolution
                CHECK_FOR_FMT(A4R4G4B4,ConvLum16to16_4444);
                CHECK_FOR_FMT(A1R5G5B5,ConvLum16to16_1555);
            } else {
               assert((cNumColorChannels==3)||(cNumColorChannels==4));
          // look for compatible 16bit fmts, if none then give up
          // (dont worry about other bitdepths for 16 bit)
                switch(cNumAlphaBits) {
                    case 0:
                      if(cNumColorChannels==3) {
                          CHECK_FOR_FMT(R5G6B5,Conv24to16_0565);
                          CHECK_FOR_FMT(X1R5G5B5,Conv24to16_X555);
                      } else {
                          assert(cNumColorChannels==4);
                        // it could be 4 if user asks us to throw away the alpha channel
                          CHECK_FOR_FMT(R5G6B5,Conv32to16_0565);
                          CHECK_FOR_FMT(X1R5G5B5,Conv32to16_X555);
                      }
                      break;
                    case 1:
                      // app specifically requests 1-5-5-5 F_rgba5 case, where you explicitly want 1-5-5-5 fmt, as opposed
                      // to F_rgbm, which could use 32bpp ARGB.  fail if this particular fmt not avail.
                      assert(cNumColorChannels==4);
                      CHECK_FOR_FMT(X1R5G5B5,Conv32to16_X555);
                      break;
                    case 4:
                      // app specifically requests 4-4-4-4 F_rgba4 case, as opposed to F_rgba, which could use 32bpp ARGB
                      assert(cNumColorChannels==4);
                      CHECK_FOR_FMT(A4R4G4B4,Conv32to16_4444);
                      break;
                    default: assert(0);  // problem in get_bits_per_pixel()?
                }
            }
        case 8:
            if(bNeedLuminance) {
                // dont bother handling those other 8bit lum fmts like 4-4, since 16 8-8 is usually supported too
                assert(cNumColorChannels==1);

                // look for native lum fmt first
                CHECK_FOR_FMT(L8,ConvLum8to8);
                CHECK_FOR_FMT(L8,ConvLum8to16_A8L8);

                if(!dx_force_16bpptextures) {
                    CHECK_FOR_FMT(R8G8B8,ConvLum8to24);
                    CHECK_FOR_FMT(X8R8G8B8,ConvLum8to32);
                }

                CHECK_FOR_FMT(R5G6B5,ConvLum8to16_0565);
                CHECK_FOR_FMT(X1R5G5B5,ConvLum8to16_X555);

            } else if(cNumAlphaBits==8) {
                // look for 16bpp A8L8, else 32-bit ARGB, else 16-4444.

                // skip 8bit alpha only (D3DFMT_A8), because I think only voodoo supports it
                // and the voodoo support isn't the kind of blending model we need somehow
                // (is it that voodoo assumes color is white?  isnt that what we do in ConvAlpha8to32 anyway?)

                CHECK_FOR_FMT(A8L8,ConvAlpha8to16_A8L8);

                if(!dx_force_16bpptextures) {
                    CHECK_FOR_FMT(A8R8G8B8,ConvAlpha8to32);
                }

                CHECK_FOR_FMT(A4R4G4B4,ConvAlpha8to16_4444);
            }
            break;

        default:
            szErrorMsg = "CreateTexture failed: unhandled pixel bitdepth in DX loader";
    }

    // if we've gotten here, haven't found a match
    dxgsg9_cat.error() << szErrorMsg << ": " << _tex->get_name() << endl
                      << "NumColorChannels: " <<cNumColorChannels << "; NumAlphaBits: " << cNumAlphaBits
                      << "; targetbpp: " <<target_bpp << "; SupportedTexFmtsMask: 0x" << (void*)scrn.SupportedTexFmtsMask
                      << "; NeedLuminance: " << bNeedLuminance << endl;
    goto error_exit;

    ///////////////////////////////////////////////////////////

 found_matching_format:
    // validate magfilter setting
    // degrade filtering if no HW support

    Texture::FilterType ft;

    ft =_tex->get_magfilter();
    if((ft!=Texture::FT_linear) && ft!=Texture::FT_nearest) {
    // mipmap settings make no sense for magfilter
        if(ft==Texture::FT_nearest_mipmap_nearest)
            ft=Texture::FT_nearest;
        else ft=Texture::FT_linear;
    }

    if((ft==Texture::FT_linear) && !(scrn.d3dcaps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR))
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
           if(dxgsg9_cat.is_spam()) {
               if(ft != Texture::FT_linear_mipmap_linear)
                   dxgsg9_cat.spam() << "Forcing trilinear mipmapping on DX texture [" << _tex->get_name() << "]\n";
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

    assert((scrn.d3dcaps.TextureFilterCaps & D3DPTFILTERCAPS_MINFPOINT)!=0);

#define TRILINEAR_MIPMAP_TEXFILTERCAPS (D3DPTFILTERCAPS_MIPFLINEAR | D3DPTFILTERCAPS_MINFLINEAR)

    // do any other filter type degradations necessary
    switch(ft) {
        case Texture::FT_linear_mipmap_linear:
            if((scrn.d3dcaps.TextureFilterCaps & TRILINEAR_MIPMAP_TEXFILTERCAPS)!=TRILINEAR_MIPMAP_TEXFILTERCAPS) {
               if(scrn.d3dcaps.TextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR)
                   ft=Texture::FT_linear_mipmap_nearest;
                else ft=Texture::FT_nearest_mipmap_nearest;  // if you cant do linear in a level, you probably cant do linear b/w levels, so just do nearest-all
            }
            break;
        case Texture::FT_nearest_mipmap_linear:
            // if we dont have bilinear, do nearest_nearest
            if(!((scrn.d3dcaps.TextureFilterCaps & D3DPTFILTERCAPS_MIPFPOINT) &&
                 (scrn.d3dcaps.TextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR)))
                ft=Texture::FT_nearest_mipmap_nearest;
            break;
        case Texture::FT_linear_mipmap_nearest:
            // if we dont have mip linear, do nearest_nearest
            if(!(scrn.d3dcaps.TextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR))
                ft=Texture::FT_nearest_mipmap_nearest;
            break;
        case Texture::FT_linear:
            if(!(scrn.d3dcaps.TextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR))
                ft=Texture::FT_nearest;
            break;
    }

    _tex->set_minfilter(ft);

    uint aniso_degree;

    aniso_degree=1;
    if(scrn.d3dcaps.RasterCaps & D3DPRASTERCAPS_ANISOTROPY) {
        aniso_degree=_tex->get_anisotropic_degree();
        if((aniso_degree>scrn.d3dcaps.MaxAnisotropy) || dx_force_anisotropic_filtering)
            aniso_degree=scrn.d3dcaps.MaxAnisotropy;
    }
    _tex->set_anisotropic_degree(aniso_degree);

#ifdef _DEBUG
    dxgsg9_cat.spam() << "CreateTexture: setting aniso degree for "<< _tex->get_name() << " to: " << aniso_degree << endl;
#endif

    UINT cMipLevelCount;

    if(_bHasMipMaps) {
        cMipLevelCount=0;  // tell CreateTex to alloc space for all mip levels down to 1x1

        if(dxgsg9_cat.is_debug())
            dxgsg9_cat.debug() << "CreateTexture: generating mipmaps for "<< _tex->get_name() << endl;
    } else cMipLevelCount=1;

    if(FAILED( hr = scrn.pD3DDevice->CreateTexture(TargetWidth,TargetHeight,cMipLevelCount,0x0,
                                                   TargetPixFmt,D3DPOOL_MANAGED,&_pD3DTexture9, NULL) )) {
        dxgsg9_cat.error() << "D3D CreateTexture failed!" << D3DERRORSTRING(hr);
        goto error_exit;
    }

#ifdef DO_CUSTOM_CONVERSIONS
    _PixBufConversionType=ConvNeeded;
#endif

#ifdef _DEBUG
#ifdef DO_CUSTOM_CONVERSIONS
    dxgsg9_cat.debug() << "CreateTexture: "<< _tex->get_name() <<" converting " << ConvNameStrs[ConvNeeded] << " \n";
#else
    dxgsg9_cat.debug() << "CreateTexture: "<< _tex->get_name() <<" converting panda equivalent of " << D3DFormatStr(_PixBufD3DFmt) << " => " << D3DFormatStr(TargetPixFmt) << endl;
#endif
#endif

    // Note: user may want to create an empty "texture" that will be written to by rendering and copy operations.
    //       this will never have a backing store of main memory in panda fmt, and on disk in a file.
    //       so for this case, you dont want to call FillDDSurf.
    //       need a better way for user to indicate this usage than lack of ram_image, because it conflicts
    //       with the multi-open case mentioned below

    if(_texture->has_ram_image()) {
        hr = FillDDSurfTexturePixels();
        if(FAILED(hr)) {
            goto error_exit;
        }
    }

    // PRINT_REFCNT(dxgsg9,scrn.pD3D9);

    // Return the newly created texture
    return _pD3DTexture9;

  error_exit:

    RELEASE(_pD3DTexture9,dxgsg9,"texture",RELEASE_ONCE);
    return NULL;
}

HRESULT DXTextureContext9::
FillDDSurfTexturePixels(void) {
    HRESULT hr=E_FAIL;
    assert(IS_VALID_PTR(_texture));

    // It is a mistake to insist that has_ram_image() be true before
    // we try to load the texture.  This function only indicates
    // whether the texture image is already present in main ram or
    // not; it has nothing to do with whether get_ram_image() will
    // fail.  When there is only one GSG in the world, has_ram_image()
    // will generally be true whenever the texture has not been loaded
    // before, but when there are multiple GSG's (for instance, if we
    // close and reopen the main window), then has_ram_image() is
    // largely irrelevant to the GSG.
    /*
    if(!_texture->has_ram_image()) {
      dxgsg9_cat.warning() << "CreateTexture: tried to fill surface that has no ram image!\n";
      return S_OK;
    }
    */

    PixelBuffer *pbuf = _texture->get_ram_image();
    if (pbuf == (PixelBuffer *)NULL) {
      dxgsg9_cat.fatal() << "CreateTexture: get_ram_image() failed\n";
      // The texture doesn't have an image to load.
      return E_FAIL;
    }

    assert(IS_VALID_PTR(_pD3DTexture9));

    DWORD OrigWidth  = (DWORD) pbuf->get_xsize();
    DWORD OrigHeight = (DWORD) pbuf->get_ysize();
    DWORD cNumColorChannels = pbuf->get_num_components();
    D3DFORMAT SrcFormat=_PixBufD3DFmt;
    BYTE *pPixels=(BYTE*)pbuf->_image.p();

    assert(IS_VALID_PTR(pPixels));

    IDirect3DSurface9 *pMipLevel0;
    hr=_pD3DTexture9->GetSurfaceLevel(0,&pMipLevel0);
    if(FAILED(hr)) {
       dxgsg9_cat.error() << "FillDDSurfaceTexturePixels failed for "<< _tex->get_name() <<", GetSurfaceLevel failed" << D3DERRORSTRING(hr);
       return E_FAIL;
    }

    RECT SrcSize;
    SrcSize.left = SrcSize.top = 0;
    SrcSize.right = OrigWidth;
    SrcSize.bottom = OrigHeight;

    UINT SrcPixBufRowByteLength=OrigWidth*cNumColorChannels;

    DWORD Lev0Filter,MipFilterFlags;
    bool bUsingTempPixBuf=false;

    // need filtering if size changes, (also if bitdepth reduced (need dithering)??)
    Lev0Filter = D3DX_FILTER_LINEAR ; //| D3DX_FILTER_DITHER;  //dithering looks ugly on i810 for 4444 textures

    // D3DXLoadSurfaceFromMemory will load black luminance and we want full white,
    // so convert to explicit luminance-alpha format
    if(_PixBufD3DFmt==D3DFMT_A8) {
        // alloc buffer for explicit D3DFMT_A8L8
        USHORT *pTempPixBuf=new USHORT[OrigWidth*OrigHeight];
        if(!IS_VALID_PTR(pTempPixBuf)) {
            dxgsg9_cat.error() << "FillDDSurfaceTexturePixels couldnt alloc mem for temp pixbuf!\n";
            goto exit_FillDDSurf;
        }
        bUsingTempPixBuf=true;

        USHORT *pOutPix=pTempPixBuf;
        BYTE *pSrcPix=pPixels;
        for(UINT y=0;y<OrigHeight;y++)
          for(UINT x=0;x<OrigWidth;x++,pSrcPix++,pOutPix++)
              *pOutPix = ((*pSrcPix) << 8 ) | 0xFF;  // add full white, which is our interpretation of alpha-only (similar to default adding full opaque alpha 0xFF to RGB-only textures)

        SrcFormat=D3DFMT_A8L8;
        SrcPixBufRowByteLength=OrigWidth*sizeof(USHORT);
        pPixels=(BYTE*)pTempPixBuf;
    }

    // filtering may be done here if texture if targetsize!=origsize
    hr=D3DXLoadSurfaceFromMemory(pMipLevel0,(PALETTEENTRY*)NULL,(RECT*)NULL,(LPCVOID)pPixels,SrcFormat,
                                 SrcPixBufRowByteLength,(PALETTEENTRY*)NULL,&SrcSize,Lev0Filter,(D3DCOLOR)0x0);
    if(FAILED(hr)) {
      dxgsg9_cat.error() << "FillDDSurfaceTexturePixels failed for "<< _tex->get_name() <<", D3DXLoadSurfFromMem failed" << D3DERRORSTRING(hr);
      goto exit_FillDDSurf;
    }

    if(_bHasMipMaps) {
      if(!dx_use_triangle_mipgen_filter)
        MipFilterFlags = D3DX_FILTER_BOX;
      else MipFilterFlags = D3DX_FILTER_TRIANGLE;
      
      //    MipFilterFlags|= D3DX_FILTER_DITHER;
      
      hr=D3DXFilterTexture(_pD3DTexture9,(PALETTEENTRY*)NULL,0,MipFilterFlags);
      if(FAILED(hr)) {
        dxgsg9_cat.error() << "FillDDSurfaceTexturePixels failed for "<< _tex->get_name() <<", D3DXFilterTex failed" << D3DERRORSTRING(hr);
        goto exit_FillDDSurf;
      }
    }

 exit_FillDDSurf:
    if(bUsingTempPixBuf) {
      SAFE_DELETE_ARRAY(pPixels);
    }
    RELEASE(pMipLevel0,dxgsg9,"FillDDSurf MipLev0 texture ptr",RELEASE_ONCE);
    return hr;
}

//-----------------------------------------------------------------------------
// Name: DeleteTexture()
// Desc: Release the surface used to store the texture
//-----------------------------------------------------------------------------
void DXTextureContext9::
DeleteTexture( ) {
    if(_pD3DTexture9==NULL) {
        // dont bother printing the msg below, since we already released it.
        return;
    }

    if(dxgsg9_cat.is_spam()) {
        dxgsg9_cat.spam() << "Deleting DX texture for " << _tex->get_name() << "\n";
    }

    RELEASE(_pD3DTexture9,dxgsg9,"texture",RELEASE_ONCE);
/*
#ifdef DEBUG_RELEASES
    if(_surface) {
        LPDIRECTDRAW7 pDD;
        _surface->GetDDInterface( (VOID**)&pDD );
        pDD->Release();

        PRINTREFCNT(pDD,"before DeleteTex, IDDraw7");
        RELEASE(_surface,dxgsg9,"texture",false);
        PRINTREFCNT(pDD,"after DeleteTex, IDDraw7");
    }
#else

    RELEASE(_pD3DSurf9,dxgsg9,"texture",false);
 #endif
*/
}


////////////////////////////////////////////////////////////////////
//     Function: DXTextureContext9::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXTextureContext9::
DXTextureContext9(Texture *tex) :
TextureContext(tex) {

    if(dxgsg9_cat.is_spam()) {
       dxgsg9_cat.spam() << "Creating DX texture [" << tex->get_name() << "], minfilter(" << PandaFilterNameStrs[tex->get_minfilter()] << "), magfilter("<<PandaFilterNameStrs[tex->get_magfilter()] << "), anisodeg(" << tex->get_anisotropic_degree() << ")\n";
    }

    _pD3DTexture9 = NULL;
    _bHasMipMaps = FALSE;
    _tex = tex;
}

DXTextureContext9::
~DXTextureContext9() {
    if(dxgsg9_cat.is_spam()) {
        dxgsg9_cat.spam() << "Deleting DX9 TexContext for " << _tex->get_name() << "\n";
    }
    DeleteTexture();
    TextureContext::~TextureContext();
    _tex = NULL;
}

