// Filename: dxTextureContext.h
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

#ifndef DXTEXTURECONTEXT_H
#define DXTEXTURECONTEXT_H

#include <pandabase.h>

//#define DO_CUSTOM_CONVERSIONS

#define WIN32_LEAN_AND_MEAN
#ifndef STRICT
// enable strict type checking in windows.h, see msdn
#define STRICT
#endif

#include <windows.h>
#include <ddraw.h>

#define D3D_OVERLOADS   //  get D3DVECTOR '+' operator, etc from d3dtypes.h
#include <d3d8.h>
#include <d3dx8.h>
#include <dxerr8.h>
#undef WIN32_LEAN_AND_MEAN


#ifndef D3DERRORSTRING
#define D3DERRORSTRING(HRESULT) " at (" << __FILE__ << ":" << __LINE__ << "), hr=" <<  DXGetErrorString8(HRESULT) << ": " << DXGetErrorDescription8(HRESULT) << endl
#endif

#include <texture.h>
#include <textureContext.h>

typedef struct {
      LPDIRECT3DDEVICE8 pD3DDevice;
      LPDIRECT3D8       pD3D8;
      HWND              hWnd;
      HMONITOR          hMon;
      RECT              view_rect,clip_rect;
      DWORD             MaxAvailVidMem;
      bool              bIsLowVidMemCard;
      bool              bIsTNLDevice;
      bool              bIsDX81;
      ushort            depth_buffer_bitdepth;  //GetSurfaceDesc is not reliable so must store this explicitly
      ushort            CardIDNum;  // adapter ID
      DWORD             dwSupportedScreenDepthsMask;
      DWORD             SupportedTexFmtsMask;
      D3DCAPS8          d3dcaps;
      D3DDISPLAYMODE    DisplayMode;
      D3DPRESENT_PARAMETERS PresParams;  // not redundant with DisplayMode since width/height must be 0 for windowed mode
      D3DADAPTER_IDENTIFIER8 DXDeviceID;
} DXScreenData;

////////////////////////////////////////////////////////////////////
//   Class : DXTextureContext
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX DXTextureContext : public TextureContext {
  friend class DXGraphicsStateGuardian;
  friend class wdxGraphicsWindow;

public:
  DXTextureContext(Texture *tex);
  ~DXTextureContext();

  IDirect3DTexture8  *_pD3DTexture8;
  Texture *_tex;            // ptr to parent, primarily for access to namestr
  IDirect3DTexture8 *CreateTexture(DXScreenData &scrn);

  bool _bHasMipMaps;

#ifdef DO_CUSTOM_CONVERSIONS
  DWORD _PixBufConversionType;  // enum ConversionType
#endif

  // must be public since called from global callback fns
  void DeleteTexture(void);
  HRESULT FillDDSurfTexturePixels(DWORD TargetWidth,DWORD TargetHeight,D3DFORMAT PixBufD3DFmt);

protected:
    unsigned int get_bits_per_pixel(PixelBuffer::Format format, int *alphbits);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TextureContext::init_type();
    register_type(_type_handle, "DXTextureContext",
          TextureContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

extern HRESULT ConvertD3DSurftoPixBuf(IDirect3DSurface8 *pD3DSurf8,PixelBuffer *pixbuf);

#endif

