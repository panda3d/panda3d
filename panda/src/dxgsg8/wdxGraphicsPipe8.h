// Filename: wdxGraphicsPipe8.h
// Created by:  drose (20Dec02)
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

#ifndef WDXGRAPHICSPIPE8_H
#define WDXGRAPHICSPIPE8_H

#include "pandabase.h"
#include "winGraphicsPipe.h"
#include "pvector.h"
#include "dxgsg8base.h"
#include <ddraw.h>

typedef struct {
   UINT    cardID;
   char    szDriver[MAX_DEVICE_IDENTIFIER_STRING];
   char    szDescription[MAX_DEVICE_IDENTIFIER_STRING];
   GUID    guidDeviceIdentifier;
   DWORD   VendorID, DeviceID;
   HMONITOR hMon;
} DXDeviceInfo;
typedef pvector<DXDeviceInfo> DXDeviceInfoVec;

////////////////////////////////////////////////////////////////////
//       Class : wdxGraphicsPipe8
// Description : This graphics pipe represents the interface for
//               creating DirectX graphics windows.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX wdxGraphicsPipe8 : public WinGraphicsPipe {
public:
  wdxGraphicsPipe8();
  virtual ~wdxGraphicsPipe8();

  virtual string get_interface_name() const;
  static PT(GraphicsPipe) pipe_constructor();

  virtual PT(GraphicsStateGuardian) make_gsg(const FrameBufferProperties &properties,
                                             GraphicsStateGuardian *share_with);
  virtual PT(GraphicsDevice) make_device(void *scrn);

  bool find_best_depth_format(DXScreenData &Display, D3DDISPLAYMODE &TestDisplayMode,
                       D3DFORMAT *pBestFmt, bool bWantStencil,
                       bool bForce16bpp, bool bVerboseMode = false) const;

  void search_for_valid_displaymode(DXScreenData &scrn,
                             UINT RequestedX_Size, UINT RequestedY_Size,
                             bool bWantZBuffer, bool bWantStencil,
                             UINT *pSupportedScreenDepthsMask,
                             bool *pCouldntFindAnyValidZBuf,
                             D3DFORMAT *pSuggestedPixFmt,
                             bool bForce16bppZBuffer,
                             bool bVerboseMode = false);

   bool special_check_fullscreen_resolution(DXScreenData &scrn, UINT x_size,UINT y_size);

protected:
  virtual PT(GraphicsWindow) make_window(GraphicsStateGuardian *gsg, const string &name);

private:
  bool init();
  bool find_all_card_memavails();

  static BOOL WINAPI
  dx7_driver_enum_callback(GUID *pGUID, TCHAR *strDesc, TCHAR *strName,
                           VOID *argptr, HMONITOR hm);

private:
  HINSTANCE _hDDrawDLL;
  HINSTANCE _hD3D8_DLL;
  LPDIRECT3D8 _pD3D8;


  typedef LPDIRECT3D8 (WINAPI *Direct3DCreate8_ProcPtr)(UINT SDKVersion);
  typedef HRESULT (WINAPI * LPDIRECTDRAWCREATEEX)(GUID FAR * lpGuid, LPVOID *lplpDD, REFIID iid, IUnknown FAR *pUnkOuter);

  LPDIRECTDRAWCREATEEX _DirectDrawCreateEx;
  LPDIRECTDRAWENUMERATEEX _DirectDrawEnumerateExA;
  Direct3DCreate8_ProcPtr _Direct3DCreate8;

  // CardID is used in DX7 lowmem card-classification pass so DX8 can
  // establish correspondence b/w DX7 mem info & DX8 device
  struct CardID {
    HMONITOR hMon;
    DWORD MaxAvailVidMem;
    bool  bIsLowVidMemCard;
    GUID  DX7_DeviceGUID;
    DWORD VendorID, DeviceID;
    //   char  szDriver[MAX_DEVICE_IDENTIFIER_STRING];
  };
  
  typedef pvector<CardID> CardIDs;
  CardIDs _card_ids;
  bool _bIsDX81;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    WinGraphicsPipe::init_type();
    register_type(_type_handle, "wdxGraphicsPipe8",
                  WinGraphicsPipe::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class wdxGraphicsWindow8;
};

#include "wdxGraphicsPipe8.I"

#endif
