// Filename: dxgsg8base.h
// Created by:  georges (07Oct01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
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

#ifndef DXGSG8BASE_H
#define DXGSG8BASE_H

#include <pandabase.h>
#include <graphicsWindow.h>

// include win32 defns for everything up to WinServer2003, and assume I'm smart enough to
// use GetProcAddress for backward compat on newer fns
// Note DX8 cannot be installed on w95, so OK to assume base of win98
#define _WIN32_WINNT 0x0502

#define WIN32_LEAN_AND_MEAN   // get rid of mfc win32 hdr stuff
#ifndef STRICT
// enable strict type checking in windows.h, see msdn
#define STRICT
#endif

#include <windows.h>

#define D3D_OVERLOADS   //  get D3DVECTOR '+' operator, etc from d3dtypes.h
#include <d3d8.h>
#include <d3dx8.h>
#include <dxerr8.h>
#undef WIN32_LEAN_AND_MEAN

#if D3D_SDK_VERSION != 220
#error you have DX 8.0 headers, not DX 8.1, you need to install DX 8.1 SDK!
#endif

#if DIRECT3D_VERSION != 0x0800
#error DX8.1 headers not available, you need to install newer MS Platform SDK!
#endif

#ifndef D3DCAPS3_ALPHA_FULLSCREEN_FLIP_OR_DISCARD
#error you have pre-release DX8.1 headers, you need to install final DX 8.1 SDK!
#endif

#ifndef D3DERRORSTRING
#ifdef NDEBUG
#define D3DERRORSTRING(HRESULT) " at (" << __FILE__ << ":" << __LINE__ << "), hr=" <<  DXGetErrorString8(HRESULT) << endl  // leave out descriptions to shrink release build
#else
#define D3DERRORSTRING(HRESULT) " at (" << __FILE__ << ":" << __LINE__ << "), hr=" <<  DXGetErrorString8(HRESULT) << ": " << DXGetErrorDescription8(HRESULT) << endl
#endif
#endif

#define D3D_MAXTEXTURESTAGES 8

typedef enum {VertexShader,PixelShader} ShaderType;
typedef DWORD DXShaderHandle;

#define ISPOW2(X) (((X) & ((X)-1))==0)
#define IS_VALID_PTR(PTR)  (!IsBadWritePtr(PTR,sizeof(void*)))

#define DX_DECLARE_CLEAN(type, var) \
    type var;                       \
    ZeroMemory(&var, sizeof(type)); \
    var.dwSize = sizeof(type);
    
#define SAFE_DELSHADER(TYPE,HANDLE,PDEVICE)  \
  if((HANDLE!=NULL)&&IS_VALID_PTR(PDEVICE)) { PDEVICE->Delete##TYPE##Shader(HANDLE);  HANDLE=NULL; }

#define SAFE_DELETE(p)       { if(p) { assert(IS_VALID_PTR(p));   delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { assert(IS_VALID_PTR(p));   delete [] (p);   (p)=NULL; } }

// for stuff outside a panda class
#define SAFE_RELEASE(p)      { if(p) { assert(IS_VALID_PTR(p)); (p)->Release(); (p)=NULL; } }

// this is bDoDownToZero argument to RELEASE()
#define RELEASE_DOWN_TO_ZERO true
#define RELEASE_ONCE false

//#define DEBUG_RELEASES

#ifdef DEBUG_RELEASES
#define RELEASE(OBJECT,MODULE,DBGSTR,bDoDownToZero)             {  \
   ULONG refcnt;                                                \
   if(IS_VALID_PTR(OBJECT)) {                                   \
        refcnt = (OBJECT)->Release();                           \
        MODULE##_cat.debug() << DBGSTR << " released, refcnt = " << refcnt << " at " << __FILE__ << ":" << __LINE__ << endl; \
        if((bDoDownToZero) && (refcnt>0)) {                     \
              MODULE##_cat.warning() << DBGSTR << " released but still has a non-zero refcnt(" << refcnt << "), multi-releasing it down to zero!\n"; \
              do {                                \
                refcnt = (OBJECT)->Release();     \
              } while(refcnt>0);                  \
        }                                         \
        (OBJECT) = NULL;                          \
      } else {                                    \
        MODULE##_cat.debug() << DBGSTR << " not released, ptr == NULL" << endl;  \
      }}

#define PRINT_REFCNT(MODULE,p) { ULONG refcnt;  (p)->AddRef();  refcnt=(p)->Release(); \
                                 MODULE##_cat.debug() << #p << " has refcnt = " << refcnt << " at " << __FILE__ << ":" << __LINE__ << endl; }
                                 
#else
#define RELEASE(OBJECT,MODULE,DBGSTR,bDoDownToZero)   { \
   ULONG refcnt;                                        \
   if(IS_VALID_PTR(OBJECT))                           { \
        refcnt=(OBJECT)->Release();                     \
        if((bDoDownToZero) && (refcnt>0)) {             \
              MODULE##_cat.warning() << DBGSTR << " released but still has a non-zero refcnt(" << refcnt << "), multi-releasing it down to zero!\n"; \
              do {                                \
                refcnt = (OBJECT)->Release();     \
              } while(refcnt>0);                  \
        }                                         \
        (OBJECT) = NULL;                          \
   }}

#define PRINT_REFCNT(MODULE,p)
#endif    

#ifdef DO_PSTATS
#define DO_PSTATS_STUFF(XX) XX;
#else
#define DO_PSTATS_STUFF(XX)
#endif

#define PANDA_MAXNUMVERTS 0xFFFF  // Note Device may support more than this if it supports D3DFMT_INDEX32 indexbufs.

#define FLG(NN) (1<<NN)
#define MAX_POSSIBLE_TEXFMTS 32
typedef enum {
    R8G8B8_FLAG =       FLG(0),
    A8R8G8B8_FLAG =     FLG(1),
    X8R8G8B8_FLAG =     FLG(2),
    R5G6B5_FLAG =       FLG(3),
    X1R5G5B5_FLAG =     FLG(4),
    A1R5G5B5_FLAG =     FLG(5),
    A4R4G4B4_FLAG =     FLG(6),
    R3G3B2_FLAG =       FLG(7),
    A8_FLAG =           FLG(8),
    A8R3G3B2_FLAG =     FLG(9),
    X4R4G4B4_FLAG =     FLG(10),
    A2B10G10R10_FLAG =  FLG(11),
    G16R16_FLAG =       FLG(12),
    A8P8_FLAG =         FLG(13),
    P8_FLAG =           FLG(14),
    L8_FLAG =           FLG(15),
    A8L8_FLAG =         FLG(16),
    A4L4_FLAG =         FLG(17),
    V8U8_FLAG =         FLG(18),
    L6V5U5_FLAG =       FLG(19),
    X8L8V8U8_FLAG =     FLG(20),
    Q8W8V8U8_FLAG =     FLG(21),
    V16U16_FLAG =       FLG(22),
    W11V11U10_FLAG =    FLG(23),
    A2W10V10U10_FLAG =  FLG(24),
    UYVY_FLAG =         FLG(25),
    YUY2_FLAG =         FLG(26),
    DXT1_FLAG =         FLG(27),
    DXT2_FLAG =         FLG(28),
    DXT3_FLAG =         FLG(29),
    DXT4_FLAG =         FLG(30),
    DXT5_FLAG =         FLG(31)
} D3DFORMAT_FLAG;

// this is only used in conjunction w/rendertgt fmts, so just make it something that can never be a rtgt
#define DISPLAY_32BPP_REQUIRES_16BPP_ZBUFFER_FLAG DXT1_FLAG
#define DISPLAY_16BPP_REQUIRES_16BPP_ZBUFFER_FLAG DXT2_FLAG

#define IS_16BPP_DISPLAY_FORMAT(FMT) (((FMT)==D3DFMT_R5G6B5)||((FMT)==D3DFMT_X1R5G5B5)||((FMT)==D3DFMT_A1R5G5B5))
#define IS_16BPP_ZBUFFER(FMT) ((FMT==D3DFMT_D16)||(FMT==D3DFMT_D15S1))
#define IS_STENCIL_FORMAT(FMT) (((FMT)==D3DFMT_D24S8) || ((FMT)==D3DFMT_D15S1) || ((FMT)==D3DFMT_D24X4S4))
#define RECT_XSIZE(REC) (REC.right-REC.left)
#define RECT_YSIZE(REC) (REC.bottom-REC.top)

typedef struct {
      LPDIRECT3DDEVICE8 pD3DDevice;
      LPDIRECT3D8       pD3D8;
      HWND              hWnd;
      HMONITOR          hMon;
      DWORD             MaxAvailVidMem;
      ushort            CardIDNum;  // adapter ID
      ushort            depth_buffer_bitdepth;  //GetSurfaceDesc is not reliable so must store this explicitly
      bool              bCanDirectDisableColorWrites;  // if true, dont need blending for this
      bool              bIsLowVidMemCard;
      bool              bIsTNLDevice;
      bool              bCanUseHWVertexShaders;
      bool              bCanUsePixelShaders;
      bool              bIsDX81;
      UINT              SupportedScreenDepthsMask;
      UINT              SupportedTexFmtsMask;
      D3DCAPS8          d3dcaps;
      D3DDISPLAYMODE    DisplayMode;
      D3DPRESENT_PARAMETERS PresParams;  // not redundant with DisplayMode since width/height must be 0 for windowed mode
      D3DADAPTER_IDENTIFIER8 DXDeviceID;
} DXScreenData;

#endif

