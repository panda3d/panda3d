// Filename: dxTextureContext7.h
// Created by:  drose (07Oct99)
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

#ifndef DXGSGBASE_H
#define DXGSGBASE_H

// include win32 defns for everything up to WinServer2003, and assume I'm smart enough to
// use GetProcAddress for backward compat on w95/w98 for newer fns
#define _WIN32_WINNT 0x0502

#define WIN32_LEAN_AND_MEAN   // get rid of mfc win32 hdr stuff
#ifndef STRICT
// enable strict type checking in windows.h, see msdn
#define STRICT
#endif

#include <windows.h>
#include <ddraw.h>

#define D3D_OVERLOADS   //  get D3DVECTOR '+' operator, etc from d3dtypes.h
#include <d3d.h>
#undef WIN32_LEAN_AND_MEAN

#if DIRECT3D_VERSION != 0x0700
#error DX7 headers not available, you need to install MS Platform SDK or DirectX 8+ SDK!
#endif

#include <pandabase.h>

// disable nameless struct 'warning'
#pragma warning (disable : 4201)

//#define USE_TEXFMTVEC
// USE_TEXFMTVEC caused crash on dealloc

#ifdef USE_TEXFMTVEC
typedef pvector<DDPIXELFORMAT> DDPixelFormatVec;
#else
#define MAX_DX_TEXPIXFMTS 20    // should be enough for any card
#endif

#define ISPOW2(X) (((X) & ((X)-1))==0)

#define DX_DECLARE_CLEAN(type, var) \
    type var;                       \
    ZeroMemory(&var, sizeof(type)); \
    var.dwSize = sizeof(type);

#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }

// this is bDoDownToZero argument to RELEASE()
#define RELEASE_DOWN_TO_ZERO true
#define RELEASE_ONCE false

// #define DEBUG_RELEASES

#ifdef DEBUG_RELEASES
#define RELEASE(OBJECT,MODULE,DBGSTR,bDoDownToZero)             \
   if(((OBJECT)!=NULL)&&(!IsBadWritePtr((OBJECT),4))) {         \
        refcnt = (OBJECT)->Release();                           \
        MODULE##_cat.debug() << DBGSTR << " released, refcnt = " << refcnt << endl;  \
        if((bDoDownToZero) && (refcnt>0)) {                     \
              MODULE##_cat.warning() << DBGSTR << " released but still has a non-zero refcnt(" << refcnt << "), multi-releasing it down to zero!\n"; \
              do {                                \
                refcnt = (OBJECT)->Release();     \
              } while(refcnt>0);                  \
        }                                         \
        (OBJECT) = NULL;                          \
      } else {                                    \
        MODULE##_cat.debug() << DBGSTR << " not released, ptr == NULL" << endl;  \
      } 

#define PRINTREFCNT(OBJECT,STR)  {  (OBJECT)->AddRef();  dxgsg7_cat.debug() << STR << " refcnt = " << (OBJECT)->Release() << endl; }
#else
#define RELEASE(OBJECT,MODULE,DBGSTR,bDoDownToZero)     \
   if(((OBJECT)!=NULL)&&(!IsBadWritePtr((OBJECT),4))) { \
        refcnt=(OBJECT)->Release();                     \
        if((bDoDownToZero) && (refcnt>0)) {             \
              MODULE##_cat.warning() << DBGSTR << " released but still has a non-zero refcnt(" << refcnt << "), multi-releasing it down to zero!\n"; \
              do {                                \
                refcnt = (OBJECT)->Release();     \
              } while(refcnt>0);                  \
        }                                         \
        (OBJECT) = NULL;                          \
   }

#define PRINTREFCNT(OBJECT,STR)  
#endif    

#ifdef DO_PSTATS
#define DO_PSTATS_STUFF(XX) XX;
#else
#define DO_PSTATS_STUFF(XX)
#endif

const char *ConvD3DErrorToString(HRESULT error);

typedef struct {
      LPDIRECT3DDEVICE7 pD3DDevice;
      LPDIRECTDRAW7     pDD;
      LPDIRECT3D7       pD3D;
      LPDIRECTDRAWSURFACE7 pddsPrimary,pddsBack,pddsZBuf;
      HWND              hWnd;
      HMONITOR          hMon;
      DWORD             dwRenderWidth,dwRenderHeight,dwFullScreenBitDepth;
      RECT              view_rect,clip_rect;
      DWORD             MaxAvailVidMem;
      bool              bIsLowVidMemCard;
      bool              bIsTNLDevice;
      bool              bIsSWRast;
      WORD              depth_buffer_bitdepth;  //GetSurfaceDesc is not reliable so must store this explicitly
      WORD              CardIDNum;  // its posn in DisplayArray, for dbgprint purposes
      DDDEVICEIDENTIFIER2 DXDeviceID;
      D3DDEVICEDESC7    D3DDevDesc;
#ifdef USE_TEXFMTVEC
      DDPixelFormatVec  TexPixFmts;
#endif
} DXScreenData;
#endif

