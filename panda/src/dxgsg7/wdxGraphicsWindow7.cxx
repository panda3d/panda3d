// Filename: wdxGraphicsWindow7.cxx
// Created by:  drose (20Dec02)
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

#include "wdxGraphicsWindow7.h"
#include "config_dxgsg7.h"
#include "config_windisplay.h"
#include "wdxGraphicsPipe7.h"
#include "dxGraphicsStateGuardian7.h"

#include <wingdi.h>

static DWORD
BitDepth_2_DDBDMask(DWORD iBitDepth) {
  switch (iBitDepth) {
  case 16: return DDBD_16;
  case 32: return DDBD_32;
  case 24: return DDBD_24;
  case 8: return DDBD_8;
  case 1: return DDBD_1;
  case 2: return DDBD_2;
  case 4: return DDBD_4;
  }
  return 0x0;
}

#define MAX_DX_ZBUF_FMTS 20
static int cNumZBufFmts;

HRESULT CALLBACK
EnumZBufFmtsCallback(LPDDPIXELFORMAT pddpf, VOID *param)  {
  DDPIXELFORMAT *ZBufFmtsArr = (DDPIXELFORMAT *) param;
  assert(cNumZBufFmts < MAX_DX_ZBUF_FMTS);
  memcpy(&(ZBufFmtsArr[cNumZBufFmts]), pddpf, sizeof(DDPIXELFORMAT));
  cNumZBufFmts++;
  return DDENUMRET_OK;
}

HRESULT CALLBACK
EnumDevicesCallback(LPSTR pDeviceDescription, LPSTR pDeviceName,
                    LPD3DDEVICEDESC7 pD3DDeviceDesc,LPVOID pContext) {
  D3DDEVICEDESC7 *pd3ddevs = (D3DDEVICEDESC7 *)pContext;
#ifdef _DEBUG
  wdxdisplay7_cat.spam()
    << "Enumerating Device " << pDeviceName << " : " 
    << pDeviceDescription << endl;
#endif

#define REGHALIDX 0
#define TNLHALIDX 1
#define SWRASTIDX 2
  
  if (IsEqualGUID(pD3DDeviceDesc->deviceGUID, IID_IDirect3DHALDevice)) {
    CopyMemory(&pd3ddevs[REGHALIDX], pD3DDeviceDesc, sizeof(D3DDEVICEDESC7));
  } else if (IsEqualGUID(pD3DDeviceDesc->deviceGUID, IID_IDirect3DTnLHalDevice)) {
    CopyMemory(&pd3ddevs[TNLHALIDX], pD3DDeviceDesc, sizeof(D3DDEVICEDESC7));
  } else if(IsEqualGUID(pD3DDeviceDesc->deviceGUID, IID_IDirect3DRGBDevice)) {
    CopyMemory(&pd3ddevs[SWRASTIDX], pD3DDeviceDesc, sizeof(D3DDEVICEDESC7));
  }
  return DDENUMRET_OK;
}

#define MAX_DISPLAY_MODES 100  // probably dont need this much, since i already screen for width&hgt
typedef struct {
  DWORD maxWidth,maxHeight;
  DWORD supportedBitDepths;    // uses DDBD_* flags
  LPDDSURFACEDESC2 pDDSD_Arr;
  DWORD cNumSurfDescs;
} DisplayModeInfo;

HRESULT WINAPI
EnumDisplayModesCallBack(LPDDSURFACEDESC2 lpDDSurfaceDesc,LPVOID lpContext) {
  DisplayModeInfo *pDMI = (DisplayModeInfo *) lpContext;
  
  // ddsd_search should assure this is true
  assert((lpDDSurfaceDesc->dwWidth == pDMI->maxWidth) && (lpDDSurfaceDesc->dwHeight == pDMI->maxHeight));
  
  // ignore refresh rates under 60Hz (and special values of 0 & 1)
  if ((lpDDSurfaceDesc->dwRefreshRate > 1) && 
      (lpDDSurfaceDesc->dwRefreshRate < 60)) {
    return DDENUMRET_OK;
  }
  
  assert(pDMI->cNumSurfDescs < MAX_DISPLAY_MODES);
  memcpy(&(pDMI->pDDSD_Arr[pDMI->cNumSurfDescs]), lpDDSurfaceDesc, sizeof(DDSURFACEDESC2));
  pDMI->cNumSurfDescs++;
  pDMI->supportedBitDepths |= BitDepth_2_DDBDMask(lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount);
  
  return DDENUMRET_OK;
}

// imperfect method to ID NVid? could also scan desc str, but that isnt fullproof either
#define IS_NVIDIA(DDDEVICEID) ((DDDEVICEID.dwVendorId==0x10DE) || (DDDEVICEID.dwVendorId==0x12D2))
#define IS_ATI(DDDEVICEID) (DDDEVICEID.dwVendorId==0x1002) 
#define IS_MATROX(DDDEVICEID) (DDDEVICEID.dwVendorId==0x102B)

TypeHandle wdxGraphicsWindow7::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow7::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
wdxGraphicsWindow7::
wdxGraphicsWindow7(GraphicsPipe *pipe, GraphicsStateGuardian *gsg) :
  WinGraphicsWindow(pipe, gsg) 
{
  _dxgsg = DCAST(DXGraphicsStateGuardian7, gsg);
  ZeroMemory(&_wcontext,sizeof(_wcontext));
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow7::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
wdxGraphicsWindow7::
~wdxGraphicsWindow7() {
}

void wdxGraphicsWindow7::
make_current(void) {
  DXGraphicsStateGuardian7 *dxgsg;
  DCAST_INTO_V(dxgsg, _gsg);
  //wglMakeCurrent(_hdc, wdxgsg->_context);
  dxgsg->set_context(&_wcontext);

  // Now that we have made the context current to a window, we can
  // reset the GSG state if this is the first time it has been used.
  // (We can't just call reset() when we construct the GSG, because
  // reset() requires having a current context.)
  dxgsg->reset_if_new();
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow7::open_window
//       Access: Protected, Virtual
//  Description: Opens the window right now.  Called from the window
//               thread.  Returns true if the window is successfully
//               opened, or false if there was a problem.
////////////////////////////////////////////////////////////////////
bool wdxGraphicsWindow7::
open_window(void) {
  
  if (!choose_device(0, NULL)) {
    wdxdisplay7_cat.error() << "Unable to find suitable rendering device.\n";
    return false;
  }

  if (!WinGraphicsWindow::open_window()) {
    return false;
  }

  _wcontext.hWnd = _hWnd;
  set_coop_levels_and_display_modes();
  create_screen_buffers_and_device(_wcontext, dx_force_16bpp_zbuffer);

  return true;
}

/*
////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow7::make_gsg
//       Access: Public, Virtual
//  Description: Creates a new GSG for the window and stores it in the
//               _gsg pointer.  This should only be called from within
//               the draw thread.
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow7::
make_gsg() {
  nassertv(_gsg == (GraphicsStateGuardian *)NULL);
  _dxgsg = new DXGraphicsStateGuardian7(this);
  _gsg = _dxgsg;
  // Tell the associated dxGSG about the window handle.
  _wcontext.hWnd = _hWnd;

  if (!choose_device(0, NULL)) {
    wdxdisplay7_cat.error()
      << "Unable to find suitable rendering device.\n";
    release_gsg();
    return;
  }

  set_coop_levels_and_display_modes();
  create_screen_buffers_and_device(_dxgsg->scrn, dx_force_16bpp_zbuffer);
}
*/

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow7::end_flip
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after begin_flip() has been called on all windows, to
//               finish the exchange of the front and back buffers.
//
//               This should cause the window to wait for the flip, if
//               necessary.
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow7::
end_flip() {
  if (_dxgsg != (DXGraphicsStateGuardian7 *)NULL && is_active()) {
    _dxgsg->show_frame();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow7::fullscreen_restored
//       Access: Protected, Virtual
//  Description: This is a hook for derived classes to do something
//               special, if necessary, when a fullscreen window has
//               been restored after being minimized.  The given
//               WindowProperties struct will be applied to this
//               window's properties after this function returns.
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow7::
fullscreen_restored(WindowProperties &properties) {
  // For some reason, this message comes in twice, once while the
  // fullscreen window is still minimized, and once again when it has
  // been restored.

  // In DX7, it seems to be necessary to render one frame between the
  // two cases, so we must set the minimized property to false even in
  // the first case.
  properties.set_minimized(false);
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow7::handle_reshape
//       Access: Protected, Virtual
//  Description: Called in the window thread when the window size or
//               location is changed, this updates the properties
//               structure accordingly.
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow7::
handle_reshape() {
  GdiFlush();
  WinGraphicsWindow::handle_reshape();

  if (_dxgsg!=NULL) {
    HRESULT hr;

    if (_wcontext.pddsBack == NULL) {
      // assume this is initial creation reshape and ignore this call
      return;
    }

    // Clear the back/primary surface to black using ddraw
    DX_DECLARE_CLEAN(DDBLTFX, bltfx);
    bltfx.dwDDFX |= DDBLTFX_NOTEARING;
    hr = _wcontext.pddsPrimary->Blt
      (NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &bltfx);
    if (FAILED(hr)) {
      wdxdisplay7_cat.fatal()
        << "Blt to Black of Primary Surf failed! : result = "
        << ConvD3DErrorToString(hr) << endl;
      exit(1);
    }

    hr = _wcontext.pDD->TestCooperativeLevel();
    if (FAILED(hr)) {
      wdxdisplay7_cat.error()
        << "TestCooperativeLevel failed : result = "
        << ConvD3DErrorToString(hr) << endl;
      return;
    }

    _dxgsg->RestoreAllVideoSurfaces();

    set_to_temp_rendertarget();

    // create the new resized rendertargets
    //RECT view_rect;
    //get_client_rect_screen(hWnd, &view_rect);
    //_dxgsg->dx_setup_after_resize(view_rect, &_wcontext);

    RECT view_rect;
    get_client_rect_screen(_wcontext.hWnd, &view_rect);
    _dxgsg->dx_setup_after_resize(&view_rect);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow7::do_fullscreen_resize
//       Access: Protected, Virtual
//  Description: Called in the window thread to resize a fullscreen
//               window.
////////////////////////////////////////////////////////////////////
bool wdxGraphicsWindow7::
do_fullscreen_resize(int x_size, int y_size) {
  _dxgsg->SetDXReady(false);

  HRESULT hr;

  DX_DECLARE_CLEAN(DDSURFACEDESC2,ddsd_curmode);

  hr = _wcontext.pDD->GetDisplayMode(&ddsd_curmode);
  if (FAILED(hr)) {
    wdxdisplay7_cat.fatal() 
      << "resize() - GetDisplayMode failed, result = "
      << ConvD3DErrorToString(hr) << endl;
    exit(1);
  }

  DX_DECLARE_CLEAN(DDSURFACEDESC2, ddsd_search);

  ddsd_search.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
  ddsd_search.dwWidth=x_size;  
  ddsd_search.dwHeight=y_size;

  // not requesting same refresh rate since changing res might not
  // support same refresh rate

  DDSURFACEDESC2 DDSD_Arr[MAX_DISPLAY_MODES];
  DisplayModeInfo DMI;
  ZeroMemory(&DDSD_Arr,sizeof(DDSD_Arr));
  ZeroMemory(&DMI,sizeof(DMI));
  DMI.maxWidth = x_size;  
  DMI.maxHeight = y_size;
  DMI.pDDSD_Arr = DDSD_Arr;

  hr = _wcontext.pDD->EnumDisplayModes(DDEDM_REFRESHRATES, &ddsd_search,
                                          &DMI, EnumDisplayModesCallBack);
  if (FAILED(hr)) {
    wdxdisplay7_cat.fatal()
      << "resize() - EnumDisplayModes failed, result = "
      << ConvD3DErrorToString(hr) << endl;
    return false;
  }

  DMI.supportedBitDepths &= _wcontext.D3DDevDesc.dwDeviceRenderBitDepth;

  DWORD dwFullScreenBitDepth;
  DWORD requested_bpp = ddsd_curmode.ddpfPixelFormat.dwRGBBitCount;

  // would like to match current bpp first.  if that is not possible,
  // try 16bpp, then 32
  DWORD requested_bpp_DDBD = BitDepth_2_DDBDMask(requested_bpp);

  if (DMI.supportedBitDepths & requested_bpp_DDBD) {
    dwFullScreenBitDepth = requested_bpp;
  } else if (DMI.supportedBitDepths & DDBD_16) {
    dwFullScreenBitDepth = 16;
  } else if (DMI.supportedBitDepths & DDBD_32) {
    dwFullScreenBitDepth = 32;
  } else {
    wdxdisplay7_cat.error()
      << "resize failed, no fullScreen resolutions at " << x_size
      << "x" << y_size << endl;
    return false;
  }

  hr = _wcontext.pDD->TestCooperativeLevel();
  if (FAILED(hr)) {
    wdxdisplay7_cat.error()
      << "TestCooperativeLevel failed : result = " 
      << ConvD3DErrorToString(hr) << endl;
    wdxdisplay7_cat.error()
      << "Full screen app failed to get exclusive mode on resize, exiting..\n";
    return false;
  }

  _dxgsg->free_dxgsg_objects();

   // let driver choose default refresh rate (hopefully its >=60Hz)   
  hr = _wcontext.pDD->SetDisplayMode(x_size, y_size, dwFullScreenBitDepth,
                                        0L, 0L);
  if (FAILED(hr)) {
    wdxdisplay7_cat.error()
      << "resize failed to reset display mode to (" 
      << x_size << "x" << y_size << "x" 
      << dwFullScreenBitDepth << "): result = " 
      << ConvD3DErrorToString(hr) << endl;
  }

  if (wdxdisplay7_cat.is_debug()) {
    DX_DECLARE_CLEAN(DDSURFACEDESC2,ddsd34); 
    _wcontext.pDD->GetDisplayMode(&ddsd34);
    wdxdisplay7_cat.debug()
      << "set displaymode to " << ddsd34.dwWidth << "x" << ddsd34.dwHeight
      << " at "<< ddsd34.ddpfPixelFormat.dwRGBBitCount << "bpp, "
      << ddsd34.dwRefreshRate << "Hz\n";
  }

  _wcontext.dwRenderWidth = x_size;
  _wcontext.dwRenderHeight = y_size;

  create_screen_buffers_and_device(_wcontext, dx_force_16bpp_zbuffer);
  _dxgsg->RecreateAllVideoSurfaces();
  _dxgsg->SetDXReady(true);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow7::support_overlay_window
//       Access: Protected, Virtual
//  Description: Some windows graphics contexts (e.g. DirectX)
//               require special support to enable the displaying of
//               an overlay window (particularly the IME window) over
//               the fullscreen graphics window.  This is a hook for
//               the window to enable or disable that mode when
//               necessary.
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow7::
support_overlay_window(bool flag) {
  if (_dxgsg != (DXGraphicsStateGuardian7 *)NULL) {
    _dxgsg->support_overlay_window(flag);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow7::set_to_temp_rendertarget
//       Access: Private
//  Description: Constructs a temporary tiny render target for the d3d
//               device (and sets the d3d device to use it).  This is
//               called by handle_reshape() just before the full-sized
//               rendertargets are constructed, in order to save video
//               memory and thereby allow resizing of the window to
//               work on 4MB cards.
////////////////////////////////////////////////////////////////////
bool wdxGraphicsWindow7::
set_to_temp_rendertarget() {
  LPDIRECTDRAWSURFACE7 pddsDummy = NULL, pddsDummyZ = NULL;
  ULONG refcnt;
  HRESULT hr;
    
  DX_DECLARE_CLEAN(DDSURFACEDESC2, ddsd);

  _wcontext.pddsBack->GetSurfaceDesc(&ddsd);
  LPDIRECTDRAW7 pDD = _wcontext.pDD;
    
  ddsd.dwFlags &= ~DDSD_PITCH;
  ddsd.dwWidth = 1; 
  ddsd.dwHeight = 1;
  ddsd.ddsCaps.dwCaps &= ~(DDSCAPS_COMPLEX | DDSCAPS_FLIP | DDSCAPS_FRONTBUFFER | DDSCAPS_BACKBUFFER);
  
  PRINTVIDMEM(pDD, &ddsd.ddsCaps, "dummy backbuf");
  
  hr = pDD->CreateSurface(&ddsd, &pddsDummy, NULL);    
  if (FAILED(hr)) {
    wdxdisplay7_cat.error()
      << "Resize CreateSurface for temp backbuf failed : result = "
      << ConvD3DErrorToString(hr) << endl;
    return false;
  }
  
  if (_wcontext.pddsZBuf != NULL) {
    DX_DECLARE_CLEAN(DDSURFACEDESC2, ddsdZ);
    _wcontext.pddsZBuf->GetSurfaceDesc(&ddsdZ);
    ddsdZ.dwFlags &= ~DDSD_PITCH;
    ddsdZ.dwWidth = 1;
    ddsdZ.dwHeight = 1;
    
    PRINTVIDMEM(pDD,&ddsdZ.ddsCaps,"dummy zbuf");
    
    hr = pDD->CreateSurface(&ddsdZ, &pddsDummyZ, NULL);
    if (FAILED(hr)) {
      wdxdisplay7_cat.error()
        << "Resize CreateSurface for temp zbuf failed : result = "
        << ConvD3DErrorToString(hr) << endl;
      return false;
    }

    hr = pddsDummy->AddAttachedSurface(pddsDummyZ);        
    if (FAILED(hr)) {
      wdxdisplay7_cat.error()
        << "Resize AddAttachedSurf for temp zbuf failed : result = "
        << ConvD3DErrorToString(hr) << endl;
      return false;
    }
  }

  hr = _wcontext.pD3DDevice->SetRenderTarget(pddsDummy, 0x0);
  if (FAILED(hr)) {
    wdxdisplay7_cat.error()
      << "Resize failed to set render target to temporary surface, result = " 
      << ConvD3DErrorToString(hr) << endl;
    return false;
  }
  
  RELEASE(pddsDummyZ, wdxdisplay7, "dummy resize zbuffer", false);
  RELEASE(pddsDummy, wdxdisplay7, "dummy resize rendertarget buffer", false);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow7::create_screen_buffers_and_device
//       Access: Private
//  Description: Called whenever the window is resized, this recreates
//               the necessary buffers for rendering.
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow7::
create_screen_buffers_and_device(DXScreenData &Display, bool force_16bpp_zbuffer) {
  DWORD dwRenderWidth=Display.dwRenderWidth;
  DWORD dwRenderHeight=Display.dwRenderHeight;
  LPDIRECT3D7 pD3DI=Display.pD3D;
  LPDIRECTDRAW7 pDD=Display.pDD;
  D3DDEVICEDESC7 *pD3DDevDesc=&Display.D3DDevDesc;

  LPDIRECTDRAWSURFACE7 pPrimaryDDSurf,pBackDDSurf,pZDDSurf;
  LPDIRECT3DDEVICE7 pD3DDevice;
  RECT view_rect;
  int i;
  HRESULT hr;
  DX_DECLARE_CLEAN(DDSURFACEDESC2, SurfaceDesc);

  assert(pDD!=NULL);
  assert(pD3DI!=NULL);

  DX_DECLARE_CLEAN(DDCAPS, DDCaps);
  pDD->GetCaps(&DDCaps, NULL);

  // if window is not foreground in exclusive mode, ddraw thinks you
  // are 'not active', so it changes your WM_ACTIVATEAPP from true to
  // false, causing us to go into a 'wait-for WM_ACTIVATEAPP true'
  // loop, and the event never comes so we hang in fullscreen wait.
  SetForegroundWindow(Display.hWnd);

  if (is_fullscreen()) {
    // Setup to create the primary surface w/backbuffer
    DX_DECLARE_CLEAN(DDSURFACEDESC2,ddsd);
    ddsd.dwFlags = DDSD_CAPS|DDSD_BACKBUFFERCOUNT;
    ddsd.ddsCaps.dwCaps = 
      DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE |
      DDSCAPS_FLIP | DDSCAPS_COMPLEX;
    ddsd.dwBackBufferCount = 1;
    Display.bIsFullScreen=true;

    if (dx_full_screen_antialiasing) {
      // cant check that d3ddevice has this capability yet, so got to
      // set it anyway.  hope this is OK.
      ddsd.ddsCaps.dwCaps2 |= DDSCAPS2_HINTANTIALIASING; 
    }

    PRINTVIDMEM(pDD, &ddsd.ddsCaps, "initial primary & backbuf");

    // Create the primary surface for the fullscreen window
    hr = pDD->CreateSurface(&ddsd, &pPrimaryDDSurf, NULL);
    if (FAILED(hr)) {
      wdxdisplay7_cat.fatal()
        << "CreateSurface failed for primary surface: result = " 
        << ConvD3DErrorToString(hr) << endl;

      if (((hr == DDERR_OUTOFVIDEOMEMORY)||(hr == DDERR_OUTOFMEMORY)) &&
          (Display.dwFullScreenBitDepth > 16)) {
        // emergency fallback to 16bpp (shouldnt have to do this
        // unless GetAvailVidMem lied) will this work for multimon?
        // what if surfs are already created on 1st mon?
        Display.dwFullScreenBitDepth=16;

        wdxdisplay7_cat.info()
          << "GetAvailVidMem lied, not enough VidMem for 32bpp, so trying 16bpp on device #"
          << Display.CardIDNum << endl;

        hr = pDD->SetDisplayMode(Display.dwRenderWidth, 
                                 Display.dwRenderHeight,
                                 Display.dwFullScreenBitDepth, 0, 0);
        if (FAILED(hr)) {
          wdxdisplay7_cat.fatal()
            << "SetDisplayMode failed to set ("
            << Display.dwRenderWidth << "x" << Display.dwRenderHeight
            << "x" << Display.dwFullScreenBitDepth << ") on device #"
            << Display.CardIDNum << ": result = "
            << ConvD3DErrorToString(hr) << endl;
          exit(1);
        }
        create_screen_buffers_and_device(Display, true);
        return;
      } else {
        exit(1);
      }
    }

    // Clear the primary surface to black
    DX_DECLARE_CLEAN(DDBLTFX, bltfx);
    bltfx.dwDDFX |= DDBLTFX_NOTEARING;
    hr = pPrimaryDDSurf->Blt(NULL, NULL, NULL, 
                             DDBLT_COLORFILL | DDBLT_WAIT, &bltfx);

    if (FAILED(hr)) {
      wdxdisplay7_cat.fatal()
        << "Blt to Black of Primary Surf failed! : result = " 
        << ConvD3DErrorToString(hr) << endl;
      exit(1);
    }

    // Get the backbuffer, which was created along with the primary.
    DDSCAPS2 ddscaps = { 
      DDSCAPS_BACKBUFFER, 0, 0, 0
    };
    hr = pPrimaryDDSurf->GetAttachedSurface(&ddscaps, &pBackDDSurf);
    if (FAILED(hr)) {
      wdxdisplay7_cat.fatal() 
        << "Can't get the backbuffer: result = " 
        << ConvD3DErrorToString(hr) << endl;
      exit(1);
    }

    hr = pBackDDSurf->Blt(NULL,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT,&bltfx);
    if (FAILED(hr)) {
      wdxdisplay7_cat.fatal() 
        << "Blt to Black of Back Surf failed! : result = "
        << ConvD3DErrorToString(hr) << endl;
      exit(1);
    }

    SetRect(&view_rect, 0, 0, dwRenderWidth, dwRenderHeight);
    // end create full screen buffers

  } else {          // CREATE WINDOWED BUFFERS
    if (!(DDCaps.dwCaps2 & DDCAPS2_CANRENDERWINDOWED)) {
      wdxdisplay7_cat.fatal()
        << "the 3D HW cannot render windowed, exiting..." << endl;
      exit(1);
    }

    hr = pDD->GetDisplayMode(&SurfaceDesc);
    if (FAILED(hr)) {
      wdxdisplay7_cat.fatal() 
        << "GetDisplayMode failed result = " 
        << ConvD3DErrorToString(hr) << endl;
      exit(1);
    }
    if (SurfaceDesc.ddpfPixelFormat.dwRGBBitCount <= 8) {
      wdxdisplay7_cat.fatal()
        << "Can't run windowed in an 8-bit or less display mode" << endl;
      exit(1);
    }

    if (!(BitDepth_2_DDBDMask(SurfaceDesc.ddpfPixelFormat.dwRGBBitCount) & pD3DDevDesc->dwDeviceRenderBitDepth)) {
      wdxdisplay7_cat.fatal() 
        << "3D Device doesnt support rendering at " 
        << SurfaceDesc.ddpfPixelFormat.dwRGBBitCount 
        << "bpp (current desktop bitdepth)" << endl;
      exit(1);
    }

    // Get the dimensions of the viewport and screen bounds
    get_client_rect_screen(Display.hWnd, &view_rect);

    dwRenderWidth = view_rect.right - view_rect.left;
    dwRenderHeight = view_rect.bottom - view_rect.top;

    // _properties should reflect view rectangle
    WindowProperties properties;
    properties.set_origin(view_rect.left, view_rect.top);
    properties.set_size(dwRenderWidth, dwRenderHeight);
    system_changed_properties(properties);

    // Initialize the description of the primary surface
    ZeroMemory(&SurfaceDesc, sizeof(DDSURFACEDESC2));
    SurfaceDesc.dwSize = sizeof(DDSURFACEDESC2);
    SurfaceDesc.dwFlags = DDSD_CAPS ;
    SurfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    PRINTVIDMEM(pDD, &SurfaceDesc.ddsCaps, "initial primary surface");

    // Create the primary surface for windowed mode.  This includes
    // all of the visible window, so no need to specify height/width.
    hr = pDD->CreateSurface(&SurfaceDesc, &pPrimaryDDSurf, NULL);
    if (FAILED(hr)) {
      wdxdisplay7_cat.fatal()
        << "CreateSurface failed for primary surface: result = "
        << ConvD3DErrorToString(hr) << endl;
      exit(1);
    }

    // Create a clipper object which handles all our clipping for
    // cases when our window is partially obscured by other windows.
    LPDIRECTDRAWCLIPPER Clipper;
    hr = pDD->CreateClipper(0, &Clipper, NULL);
    if (FAILED(hr)) {
      wdxdisplay7_cat.fatal()
        << "CreateClipper failed : result = "
        << ConvD3DErrorToString(hr) << endl;
      exit(1);
    }

    // Associate the clipper with our window.  Note that, afterwards,
    // the clipper is internally referenced by the primary surface, so
    // it is safe to release our local reference to it.
    Clipper->SetHWnd(0, Display.hWnd);
    pPrimaryDDSurf->SetClipper(Clipper);
    Clipper->Release();
   
    // Clear the primary surface to black
    DX_DECLARE_CLEAN(DDBLTFX, bltfx);

    hr = pPrimaryDDSurf->Blt(NULL,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT,&bltfx);
    if (FAILED(hr)) {
      wdxdisplay7_cat.fatal()
        << "Blt to Black of Primary Surf failed! : result = " << ConvD3DErrorToString(hr) << endl;
      exit(1);
    }

    // Set up a surface description to create a backbuffer.
    SurfaceDesc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
    SurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;
    SurfaceDesc.dwWidth = dwRenderWidth;
    SurfaceDesc.dwHeight = dwRenderHeight;

    if (dx_full_screen_antialiasing) {
      // cant check that d3ddevice has this capability yet, so got to
      // set it anyway.  hope this is OK.
      SurfaceDesc.ddsCaps.dwCaps2 |= DDSCAPS2_HINTANTIALIASING; 
    }

    PRINTVIDMEM(pDD, &SurfaceDesc.ddsCaps, "initial backbuf");

    // Create the backbuffer. (might want to handle failure due to
    // running out of video memory)
    hr = pDD->CreateSurface(&SurfaceDesc, &pBackDDSurf, NULL);
    if (FAILED(hr)) {
      wdxdisplay7_cat.fatal()
        << "CreateSurface failed for backbuffer : result = "
        << ConvD3DErrorToString(hr) << endl;
      exit(1);
    }

    hr = pBackDDSurf->Blt(NULL,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT,&bltfx);
    if (FAILED(hr)) {
      wdxdisplay7_cat.fatal()
        << "Blt to Black of Back Surf failed! : result = "
        << ConvD3DErrorToString(hr) << endl;
      exit(1);
    }
  }  // end create windowed buffers

  //  ========================================================

  //  resized(dwRenderWidth, dwRenderHeight);  // update panda channel/display rgn info

  int frame_buffer_mode = _gsg->get_properties().get_frame_buffer_mode();

#ifndef NDEBUG
  if ((frame_buffer_mode & FrameBufferProperties::FM_depth) == 0) {
    wdxdisplay7_cat.info()
      << "no zbuffer requested, skipping zbuffer creation\n";
  }
#endif

  // Check if the device supports z-bufferless hidden surface
  // removal. If so, we don't really need a z-buffer
  if ((!(pD3DDevDesc->dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ZBUFFERLESSHSR )) &&
      ((frame_buffer_mode & FrameBufferProperties::FM_depth) != 0)) {

    // Get z-buffer dimensions from the render target
    DX_DECLARE_CLEAN(DDSURFACEDESC2,ddsd);
    pBackDDSurf->GetSurfaceDesc( &ddsd );

    // Setup the surface desc for the z-buffer.
    ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | ((_wcontext.bIsSWRast) ?  DDSCAPS_SYSTEMMEMORY : DDSCAPS_VIDEOMEMORY);

    DDPIXELFORMAT ZBufPixFmts[MAX_DX_ZBUF_FMTS];
    cNumZBufFmts=0;

    // Get an appropiate pixel format from enumeration of the
    // formats. On the first pass, we look for a zbuffer dpeth which
    // is equal to the frame buffer depth (as some cards unfornately
    // require this).
    hr = pD3DI->EnumZBufferFormats((Display.bIsTNLDevice ? IID_IDirect3DHALDevice : IID_IDirect3DTnLHalDevice),
                                   EnumZBufFmtsCallback,
                                   (VOID*)&ZBufPixFmts);
    if (FAILED(hr)) {
      wdxdisplay7_cat.fatal() << "EnumZBufferFormats failed" << endl;
      exit(1);
    }

#ifdef _DEBUG
    {
      static BOOL bPrinted=FALSE;
      if (!bPrinted) {
        for (i = 0; i < cNumZBufFmts; i++) {
          DebugPrintPixFmt(&ZBufPixFmts[i]);
        }
        bPrinted=TRUE;
      }
    }
#endif

    // should we pay attn to these at some point?  
    //int want_depth_bits = _props._want_depth_bits; 
    //int want_color_bits = _props._want_color_bits;
    bool bWantStencil = ((frame_buffer_mode & FrameBufferProperties::FM_stencil) != 0);

    LPDDPIXELFORMAT pCurPixFmt, pz16 = NULL, pz24 = NULL, pz32 = NULL;
    for (i = 0, pCurPixFmt = ZBufPixFmts; 
         i < cNumZBufFmts; 
         i++, pCurPixFmt++) {
      if (bWantStencil == ((pCurPixFmt->dwFlags & DDPF_STENCILBUFFER)!=0)) {
        switch (pCurPixFmt->dwRGBBitCount) {
        case 16:
          pz16 = pCurPixFmt;
          break;
        case 24:
          pz24 = pCurPixFmt;
          break;
        case 32:
          pz32 = pCurPixFmt;
          break;
        }
      }
    }

    if ((pz16 == NULL) && (pz24 == NULL) && (pz32 == NULL)) {
      if (bWantStencil) {
        wdxdisplay7_cat.fatal()
          << "stencil buffer requested, device has no stencil capability\n";
      } else {
        wdxdisplay7_cat.fatal()
          << "failed to find adequate zbuffer capability\n";
      }
      exit(1);
    }

#define SET_ZBUF_DEPTH(DEPTH) { assert(pz##DEPTH != NULL); Display.depth_buffer_bitdepth=DEPTH; ddsd.ddpfPixelFormat = *pz##DEPTH;}
        
    if (_wcontext.bIsSWRast) {
      SET_ZBUF_DEPTH(16);    // need this for fast path rasterizers
    } else {
      if (IS_NVIDIA(Display.DXDeviceID)) {
        DX_DECLARE_CLEAN(DDSURFACEDESC2,ddsd_pri);
        pPrimaryDDSurf->GetSurfaceDesc(&ddsd_pri);

        // must pick zbuf depth to match primary surface depth for nvidia
        if (ddsd_pri.ddpfPixelFormat.dwRGBBitCount==16) {
          SET_ZBUF_DEPTH(16);
        } else {
          if (force_16bpp_zbuffer) {
            wdxdisplay7_cat.fatal()
              << "'dx-force-16bpp-zbuffer #t' requires a 16bpp desktop on nvidia cards\n";
            exit(1);
          }
          // take the smaller of 24 or 32.  (already assured to match stencil capability)
          if(pz24 != NULL) {
            SET_ZBUF_DEPTH(24);
          } else {
            SET_ZBUF_DEPTH(32);
          }
        }
      } else {
        if (force_16bpp_zbuffer) {
          if (pz16 == NULL) {
            wdxdisplay7_cat.fatal()
              << "'dx-force-16bpp-zbuffer #t', but no 16bpp zbuf fmts available on this card\n";
            exit(1);
          }

          if(wdxdisplay7_cat.is_debug()) {
            wdxdisplay7_cat.debug() << "forcing use of 16bpp Z-Buffer\n";
          }
          SET_ZBUF_DEPTH(16);
          ddsd.ddpfPixelFormat = *pz16;

        } else {
          // pick the highest res zbuffer format avail.  Note: this is
          // choosing to waste vid-memory and possibly perf for more
          // accuracy, less z-fighting at long distance (std 16bpp
          // would be smaller, maybe faster) order of preference 24:
          // (should be enough), 32: probably means 24 of Z, then 16

          if (bWantStencil && (pz32!=NULL)) {
            // dont want to select 16/8 z/stencil over 24/8 z/stenc
            SET_ZBUF_DEPTH(32);
          } else {
            if (pz24!=NULL) {
              SET_ZBUF_DEPTH(24);
            } else if (pz32!=NULL) {
              SET_ZBUF_DEPTH(32);
            } else {
              SET_ZBUF_DEPTH(16);
            }
          }
        }
      }
    }

    PRINTVIDMEM(pDD, &ddsd.ddsCaps, "initial zbuf");

#ifdef _DEBUG
    wdxdisplay7_cat.info()
      << "Creating " << ddsd.ddpfPixelFormat.dwRGBBitCount << "bpp zbuffer\n";
#endif

    // Create and attach a z-buffer
    hr = pDD->CreateSurface(&ddsd, &pZDDSurf, NULL);
    if (FAILED(hr)) {
      wdxdisplay7_cat.fatal()
        << "CreateSurface failed for Z buffer: result = "
        <<  ConvD3DErrorToString(hr) << endl;

      if (((hr==DDERR_OUTOFVIDEOMEMORY)||(hr==DDERR_OUTOFMEMORY)) &&
          ((Display.dwFullScreenBitDepth>16)||(ddsd.ddpfPixelFormat.dwRGBBitCount>16))) {
        Display.dwFullScreenBitDepth=16;
        // emergency fallback to 16bpp (shouldnt have to do this
        // unless GetAvailVidMem lied) will this work for multimon?
        // what if surfs are already created on 1st mon?

        wdxdisplay7_cat.info()
          << "GetAvailVidMem lied, not enough VidMem for 32bpp, so trying 16bpp on device #"
          << Display.CardIDNum << endl;
        
        ULONG refcnt;

        // free pri and back (maybe should just free pri since created
        // as complex chain?)
        RELEASE(pBackDDSurf, wdxdisplay7, "backbuffer", false);
        RELEASE(pPrimaryDDSurf, wdxdisplay7, "primary surface", false);

        hr = pDD->SetDisplayMode(Display.dwRenderWidth,
                                 Display.dwRenderHeight,
                                 Display.dwFullScreenBitDepth, 
                                 0, 0);
        if (FAILED(hr)) {
          wdxdisplay7_cat.fatal()
            << "SetDisplayMode failed to set (" 
            << Display.dwRenderWidth << "x" << Display.dwRenderHeight 
            << "x" << Display.dwFullScreenBitDepth << ") on device #"
            << Display.CardIDNum << ": result = " 
            << ConvD3DErrorToString(hr) << endl;
          exit(1);
        }
        create_screen_buffers_and_device(Display, true);
        return;

      } else {
        exit(1);
      }
    }

    hr = pBackDDSurf->AddAttachedSurface(pZDDSurf);
    if (FAILED(hr)) {
      wdxdisplay7_cat.fatal()
        << "AddAttachedSurface failed : result = " 
        << ConvD3DErrorToString(hr) << endl;
      exit(1);
    }
  }

  // Create the device. The device is created off of our back buffer,
  // which becomes the render target for the newly created device.
  hr = pD3DI->CreateDevice(pD3DDevDesc->deviceGUID, pBackDDSurf, &pD3DDevice);
  if (hr != DD_OK) {
    wdxdisplay7_cat.fatal() 
      << "CreateDevice failed : result = " 
      << ConvD3DErrorToString(hr) << endl;
    exit(1);
  }

  // No reason to create a viewport at this point.
  /*
  // Create the viewport
  WindowProperties properties = get_properties();
  D3DVIEWPORT7 vp = { 
    0, 0,
    properties.get_x_size(), properties.get_y_size(),
    0.0f, 1.0f
  };
  hr = pD3DDevice->SetViewport(&vp);
  if (hr != DD_OK) {
    wdxdisplay7_cat.fatal()
      << "SetViewport failed : result = " << ConvD3DErrorToString(hr) << endl;
    exit(1);
  }
  */

  Display.pD3DDevice = pD3DDevice;
  Display.pddsPrimary = pPrimaryDDSurf;
  Display.pddsBack = pBackDDSurf;
  Display.pddsZBuf = pZDDSurf;
  Display.view_rect = view_rect;

  _dxgsg->set_context(&Display);
  //pDD, pPrimaryDDSurf, pBackDDSurf, pZDDSurf, pD3DI, pD3DDevice, view_rect);
  _dxgsg->dx_init();

  // do not SetDXReady() yet since caller may want to do more work
  // before letting rendering proceed

  // Oh, go ahead and call it.
  _dxgsg->SetDXReady(true);
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow7::choose_device
//       Access: Private
//  Description: Searches for a suitable hardware device for
//               rendering.
////////////////////////////////////////////////////////////////////
bool wdxGraphicsWindow7::
choose_device(int devnum, DXDeviceInfo *pDevinfo) {
  wdxGraphicsPipe7 *dxpipe;
  DCAST_INTO_R(dxpipe, _pipe, false);

  DWORD dwRenderWidth = get_properties().get_x_size();
  DWORD dwRenderHeight = get_properties().get_y_size();
  LPDIRECTDRAW7 pDD=NULL;
  HRESULT hr;

  assert(_dxgsg != NULL);

  GUID *pDDDeviceGUID;
  if (pDevinfo == NULL) {
    pDDDeviceGUID = NULL;
  } else {
    pDDDeviceGUID = &pDevinfo->guidDeviceIdentifier;
  }

  assert(dxpipe->_DirectDrawCreateEx != NULL);

  // Create the Direct Draw Objects
  hr = (*dxpipe->_DirectDrawCreateEx)(pDDDeviceGUID, (void **)&pDD, 
                                      IID_IDirectDraw7, NULL);
  if ((hr != DD_OK) || (pDD == NULL)) {
    wdxdisplay7_cat.fatal()
      << "DirectDrawCreateEx failed for monitor(" << devnum
      << "): result = " << ConvD3DErrorToString(hr) << endl;
    return false;
  }

  _wcontext.pDD = pDD;

  // GetDeviceID bug writes an extra 4 bytes, so need xtra space
  BYTE id_arr[sizeof(DDDEVICEIDENTIFIER2) + 4];
  pDD->GetDeviceIdentifier((DDDEVICEIDENTIFIER2 *)&id_arr, 0x0);

  memcpy(&_wcontext.DXDeviceID, id_arr, sizeof(DDDEVICEIDENTIFIER2));

  if (wdxdisplay7_cat.is_info()) {
    DDDEVICEIDENTIFIER2 *pDevID = &_wcontext.DXDeviceID;
    wdxdisplay7_cat.info() 
      << "GfxCard: " << pDevID->szDescription <<  "; DriverFile: '" 
      << pDevID->szDriver  
      << "'; VendorID: 0x" <<  (void*)pDevID->dwVendorId 
      << "; DeviceID: 0x" <<  (void*)pDevID->dwDeviceId 
      << "; DriverVer: " 
      << HIWORD(pDevID->liDriverVersion.HighPart) << "." 
      << LOWORD(pDevID->liDriverVersion.HighPart) << "."
      << HIWORD(pDevID->liDriverVersion.LowPart) << "." 
      << LOWORD(pDevID->liDriverVersion.LowPart) << endl;
  }

  // Query DirectDraw for access to Direct3D
  hr = pDD->QueryInterface(IID_IDirect3D7, (VOID**)&_wcontext.pD3D);
  if(hr != DD_OK) {
    wdxdisplay7_cat.fatal()
      << "QI for D3D failed : result = " << ConvD3DErrorToString(hr) << endl;
    goto error_exit;
  }

  D3DDEVICEDESC7 d3ddevs[3];  // put HAL in 0, TnLHAL in 1, SW rast in 2

  // just look for HAL and TnL devices right now.  I dont think
  // we have any interest in the sw rasts at this point

  ZeroMemory(d3ddevs,3*sizeof(D3DDEVICEDESC7));

  hr = _wcontext.pD3D->EnumDevices(EnumDevicesCallback, d3ddevs);
  if(hr != DD_OK) {
    wdxdisplay7_cat.fatal()
      << "EnumDevices failed : result = " << ConvD3DErrorToString(hr) << endl;
    goto error_exit;
  }
    
  WORD DeviceIdx;

  // select TNL if present
  if (d3ddevs[TNLHALIDX].dwDevCaps & D3DDEVCAPS_HWRASTERIZATION) {
    DeviceIdx = TNLHALIDX;
  } else if (d3ddevs[REGHALIDX].dwDevCaps & D3DDEVCAPS_HWRASTERIZATION) {
    DeviceIdx = REGHALIDX;
  } else if (dx_allow_software_renderer || dx_force_software_renderer) {
    DeviceIdx = SWRASTIDX;      
  } else {
    wdxdisplay7_cat.error()
      << "No 3D HW present on device #" << devnum << ", skipping it... ("
      << _wcontext.DXDeviceID.szDescription<<")\n";
    goto error_exit;
  }

  if (dx_force_software_renderer) {
    DeviceIdx = SWRASTIDX; 
  }
    
  memcpy(&_wcontext.D3DDevDesc, &d3ddevs[DeviceIdx], 
         sizeof(D3DDEVICEDESC7));

  _wcontext.bIsTNLDevice = (DeviceIdx == TNLHALIDX);

  // Get Current VidMem avail.  Note this is only an estimate, when we
  // switch to fullscreen mode from desktop, more vidmem will be
  // available (typically 1.2 meg).  I dont want to switch to
  // fullscreen more than once due to the annoying monitor flicker, so
  // try to figure out optimal mode using this estimate
  DDSCAPS2 ddsGAVMCaps;
  DWORD dwVidMemTotal, dwVidMemFree;
  dwVidMemTotal = dwVidMemFree = 0;
  ZeroMemory(&ddsGAVMCaps, sizeof(DDSCAPS2));
  ddsGAVMCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;  // dont count AGP mem!
  hr = pDD->GetAvailableVidMem(&ddsGAVMCaps,&dwVidMemTotal,&dwVidMemFree);
  if (FAILED(hr)) {
    wdxdisplay7_cat.error()
      << "GetAvailableVidMem failed for device #" << devnum
      << ": result = " << ConvD3DErrorToString(hr) << endl;
    // goto skip_device;
    exit(1);  // probably want to exit, since it may be my fault
  }
    
  // after SetDisplayMode, GetAvailVidMem totalmem seems to go down by
  // 1.2 meg (contradicting above comment and what I think would be
  // correct behavior (shouldnt FS mode release the desktop vidmem?),
  // so this is the true value
  _wcontext.MaxAvailVidMem = dwVidMemTotal;
    
#define LOWVIDMEMTHRESHOLD 5500000
#define CRAPPY_DRIVER_IS_LYING_VIDMEMTHRESHOLD 1000000     // every vidcard we deal with should have at least 1MB
    
  // assume buggy drivers (this means you, FireGL2) may return zero for dwVidMemTotal, so ignore value if its < CRAPPY_DRIVER_IS_LYING_VIDMEMTHRESHOLD
  _wcontext.bIsLowVidMemCard = 
    ((dwVidMemTotal>CRAPPY_DRIVER_IS_LYING_VIDMEMTHRESHOLD) && 
     (dwVidMemTotal< LOWVIDMEMTHRESHOLD));   

  if (!dx_do_vidmemsize_check) {
    _wcontext.MaxAvailVidMem = 0xFFFFFFFF;
    _wcontext.bIsLowVidMemCard = false; 
  }

  if (DeviceIdx == SWRASTIDX) {
    // this will force 640x480x16, is this what we want for all sw rast?
    _wcontext.bIsLowVidMemCard = true; 
    _wcontext.bIsSWRast = true; 
    dx_force_16bpp_zbuffer = true;
  }

  if (is_fullscreen()) {
    DX_DECLARE_CLEAN(DDSURFACEDESC2,ddsd_search);
    ddsd_search.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
    ddsd_search.dwWidth = dwRenderWidth;
    ddsd_search.dwHeight = dwRenderHeight;
    
    DDSURFACEDESC2 DDSD_Arr[MAX_DISPLAY_MODES];
    DisplayModeInfo DMI;
    ZeroMemory(&DDSD_Arr, sizeof(DDSD_Arr));
    ZeroMemory(&DMI, sizeof(DMI));
    DMI.maxWidth = dwRenderWidth;
    DMI.maxHeight = dwRenderHeight;
    DMI.pDDSD_Arr = DDSD_Arr;

    hr= pDD->EnumDisplayModes(DDEDM_REFRESHRATES, &ddsd_search,
                              &DMI, EnumDisplayModesCallBack);        
    if (FAILED(hr)) {
      wdxdisplay7_cat.fatal()
        << "EnumDisplayModes failed for device #" << devnum 
        << " (" << _wcontext.DXDeviceID.szDescription
        << "), result = " << ConvD3DErrorToString(hr) << endl;
      // goto skip_device;
      exit(1);  // probably want to exit, since it may be my fault
    }
        
    if (wdxdisplay7_cat.is_info()) {
      wdxdisplay7_cat.info()
        << "Before fullscreen switch: GetAvailableVidMem for device #"
        << devnum << " returns Total: " << dwVidMemTotal/1000000.0
        << "  Free: " << dwVidMemFree/1000000.0 << endl;
    }
        
    // Now we try to figure out if we can use requested screen
    // resolution and best rendertarget bpp and still have at least 2
    // meg of texture vidmem
        
    DMI.supportedBitDepths &= _wcontext.D3DDevDesc.dwDeviceRenderBitDepth;

    DWORD dwFullScreenBitDepth;

    // note: this chooses 32bpp, which may not be preferred over 16
    // for memory & speed reasons
    if (DMI.supportedBitDepths & DDBD_32) {
      dwFullScreenBitDepth = 32;              // go for 32bpp if its avail
    } else if (DMI.supportedBitDepths & DDBD_24) {
      dwFullScreenBitDepth = 24;              // go for 24bpp if its avail
    } else if (DMI.supportedBitDepths & DDBD_16) {
      dwFullScreenBitDepth = 16;              // do 16bpp
    } else {
      wdxdisplay7_cat.fatal() 
        << "No Supported FullScreen resolutions at " << dwRenderWidth
        << "x" << dwRenderHeight << " for device #" << devnum 
        << " (" << _wcontext.DXDeviceID.szDescription
        << "), skipping device...\n";
      goto error_exit;
    }
        
    if (_wcontext.bIsLowVidMemCard) {
      {
        // hack: figuring out exactly what res to use is tricky,
        // instead I will just use 640x480 if we have < 3 meg avail
    
        dwFullScreenBitDepth = 16; 
        dwRenderWidth = 640;
        dwRenderHeight = 480;
        dx_force_16bpptextures = true;
        
        if (wdxdisplay7_cat.is_info())
          wdxdisplay7_cat.info() 
            << "Available VidMem (" << dwVidMemFree << ") is under "
            << LOWVIDMEMTHRESHOLD 
            << ", using 640x480 16bpp rendertargets to save tex vidmem.\n";
      }
    }

    _wcontext.dwFullScreenBitDepth = dwFullScreenBitDepth;
  }
    
  _wcontext.dwRenderWidth = dwRenderWidth;
  _wcontext.dwRenderHeight = dwRenderHeight;
  if (pDevinfo) {
    _wcontext.hMon = pDevinfo->hMon;
  }
  _wcontext.CardIDNum = devnum;  // add ID tag for dbgprint purposes

  return true;
    
  // handle errors within this for device loop
    
 error_exit:
  if (_wcontext.pD3D != NULL)
    _wcontext.pD3D->Release();
  if (_wcontext.pDD != NULL)
    _wcontext.pDD->Release();

  _wcontext.pDD = NULL;
  _wcontext.pD3D = NULL;
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow7::set_coop_levels_and_display_modes
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow7::
set_coop_levels_and_display_modes() {
  HRESULT hr;
  DWORD SCL_FPUFlag;

  if (dx_preserve_fpu_state) {
    // tell d3d to preserve the fpu state across calls.  this hurts
    // perf, but is good for dbgging
    SCL_FPUFlag = DDSCL_FPUPRESERVE;
  } else {
    SCL_FPUFlag = DDSCL_FPUSETUP;
  }

  //  DXScreenData *pScrn = &_dxgsg->scrn;

  if (!is_fullscreen()) {
    hr = _wcontext.pDD->SetCooperativeLevel(_hWnd, 
                                            SCL_FPUFlag | DDSCL_NORMAL);
    if (FAILED(hr)) {
      wdxdisplay7_cat.fatal()
        << "SetCooperativeLevel failed : result = "
        << ConvD3DErrorToString(hr) << endl;
      exit(1);
    }
    return; 
  }
    
  DWORD SCL_FLAGS = SCL_FPUFlag | DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE | DDSCL_ALLOWREBOOT;

  // The following code would be done per window if were supporting
  // multiple fullscreen windows on different graphics cards.  TODO:
  // restore that functionality.
  DWORD devnum = 0;
  {
    // DXScreenData *pScrn = &_dxgsg->scrn;

    // need to set focus/device windows for multimon
    // focus window is primary monitor that will receive keybd input
    // all ddraw objs need to have same focus window
    /*
    if(_windows.size()>1) {    
      if(FAILED(hr = pScrn->pDD->SetCooperativeLevel(_hParentWindow, DDSCL_SETFOCUSWINDOW))) {
        wdxdisplay7_cat.fatal() << "SetCooperativeLevel SetFocusWindow failed on device 0: result = " << ConvD3DErrorToString(hr) << endl;
        exit(1);
      }
    }
    */

    // s3 savage2000 on w95 seems to set EXCLUSIVE_MODE only if you call
    // SetCoopLevel twice.  so we do it, it really shouldnt be necessary
    // if drivers werent buggy
    for (int jj=0; jj<2; jj++) {
      hr = _wcontext.pDD->SetCooperativeLevel(_wcontext.hWnd, SCL_FLAGS);
      if (FAILED(hr)) {
        wdxdisplay7_cat.fatal() 
          << "SetCooperativeLevel failed for device #" << devnum 
          << ": result = " << ConvD3DErrorToString(hr) << endl;
        exit(1);
      }
    }
    
    hr = _wcontext.pDD->TestCooperativeLevel();
    if (FAILED(hr)) {
      wdxdisplay7_cat.fatal()
        << "TestCooperativeLevel failed: result = " << ConvD3DErrorToString(hr)
        << endl;
      wdxdisplay7_cat.fatal()
        << "Full screen app failed to get exclusive mode on init, exiting..\n";
      exit(1);
    }

    // note: its important we call SetDisplayMode on all cards before
    // creating surfaces on any of them to let driver choose default
    // refresh rate (hopefully its >=60Hz)
    hr = _wcontext.pDD->SetDisplayMode(_wcontext.dwRenderWidth, 
                                    _wcontext.dwRenderHeight,
                                    _wcontext.dwFullScreenBitDepth, 0, 0);
    if (FAILED(hr)) {
      wdxdisplay7_cat.fatal()
        << "SetDisplayMode failed to set (" << _wcontext.dwRenderWidth
        << "x" <<_wcontext.dwRenderHeight << "x" << _wcontext.dwFullScreenBitDepth
        << ") on device #" << _wcontext.CardIDNum << ": result = " 
        << ConvD3DErrorToString(hr) << endl;
      exit(1);
    }
    
    if(wdxdisplay7_cat.is_debug()) {
      DX_DECLARE_CLEAN(DDSURFACEDESC2,ddsd_temp); 
      _wcontext.pDD->GetDisplayMode(&ddsd_temp);
      wdxdisplay7_cat.debug()
        << "set displaymode to " << ddsd_temp.dwWidth << "x" << ddsd_temp.dwHeight
        << " at " << ddsd_temp.ddpfPixelFormat.dwRGBBitCount << "bpp, " 
        << ddsd_temp.dwRefreshRate<< "Hz\n";
    }
  }
}

#if 0

// probably need this here similar to dx8
////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow8::begin_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               before beginning rendering for a given frame.  It
//               should do whatever setup is required, and return true
//               if the frame should be rendered, or false if it
//               should be skipped.
////////////////////////////////////////////////////////////////////
bool wdxGraphicsWindow7::
begin_frame() {
  if (_awaiting_restore) {
    // The fullscreen window was recently restored; we can't continue
    // until the GSG says we can.
    if (!_dxgsg->CheckCooperativeLevel()) {
      // Keep waiting.
      return false;
    }
    _awaiting_restore = false;

    init_resized_window();
  }

  return WinGraphicsWindow::begin_frame();
}
#endif
