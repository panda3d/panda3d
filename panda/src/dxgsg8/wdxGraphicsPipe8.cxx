// Filename: wdxGraphicsPipe8.cxx
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

#include "wdxGraphicsPipe8.h"
#include "dxGraphicsDevice8.h"
#include "wdxGraphicsWindow8.h"
#include "config_dxgsg8.h"

TypeHandle wdxGraphicsPipe8::_type_handle;

// #define LOWVIDMEMTHRESHOLD 3500000
#define LOWVIDMEMTHRESHOLD 5700000  // 4MB cards should fall below this
#define CRAPPY_DRIVER_IS_LYING_VIDMEMTHRESHOLD 1000000  // if # is > 1MB, card is lying and I cant tell what it is
#define UNKNOWN_VIDMEM_SIZE 0xFFFFFFFF

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsPipe8::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
wdxGraphicsPipe8::
wdxGraphicsPipe8() {
  _hDDrawDLL = NULL;
  _hD3D8_DLL = NULL;
  _pD3D8 = NULL;
  _is_valid = init();
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsPipe8::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
wdxGraphicsPipe8::
~wdxGraphicsPipe8() {

  RELEASE(_pD3D8,wdxdisplay8,"ID3D8",RELEASE_DOWN_TO_ZERO);
  SAFE_FREELIB(_hD3D8_DLL);
  SAFE_FREELIB(_hDDrawDLL);
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsPipe8::get_interface_name
//       Access: Published, Virtual
//  Description: Returns the name of the rendering interface
//               associated with this GraphicsPipe.  This is used to
//               present to the user to allow him/her to choose
//               between several possible GraphicsPipes available on a
//               particular platform, so the name should be meaningful
//               and unique for a given platform.
////////////////////////////////////////////////////////////////////
string wdxGraphicsPipe8::
get_interface_name() const {
  return "DirectX8";
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsPipe8::pipe_constructor
//       Access: Public, Static
//  Description: This function is passed to the GraphicsPipeSelection
//               object to allow the user to make a default
//               wdxGraphicsPipe8.
////////////////////////////////////////////////////////////////////
PT(GraphicsPipe) wdxGraphicsPipe8::
pipe_constructor() {
  return new wdxGraphicsPipe8;
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsPipe8::make_window
//       Access: Protected, Virtual
//  Description: Creates a new window on the pipe, if possible.
////////////////////////////////////////////////////////////////////
PT(GraphicsWindow) wdxGraphicsPipe8::
make_window(GraphicsStateGuardian *gsg) {
  if (!_is_valid) {
    return NULL;
  }

  // thanks to the dumb threading requirements this constructor actually does nothing but create an empty c++ object
  // no windows are really opened until wdxGraphicsWindow8->open_window() is called
  return new wdxGraphicsWindow8(this, gsg);
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsPipe8::init
//       Access: Private
//  Description: Performs some initialization steps to load up
//               function pointers from the relevant DLL's, and
//               determine the number and type of available graphics
//               adapters, etc.  Returns true on success, false on
//               failure.
////////////////////////////////////////////////////////////////////
bool wdxGraphicsPipe8::
init() {
  if(!MyLoadLib(_hDDrawDLL,"ddraw.dll")) {
      goto error;
  }

  if(!MyGetProcAddr(_hDDrawDLL, (FARPROC*)&_DirectDrawCreateEx, "DirectDrawCreateEx")) {
      goto error;
  }

  if(!MyGetProcAddr(_hDDrawDLL, (FARPROC*)&_DirectDrawEnumerateExA, "DirectDrawEnumerateExA")) {
      goto error;
  }

  if(!MyLoadLib(_hD3D8_DLL,"d3d8.dll")) {
      goto error;
  }

  if(!MyGetProcAddr(_hD3D8_DLL, (FARPROC*)&_Direct3DCreate8, "Direct3DCreate8")) {
      goto error;
  }
/*
  wdxGraphicsPipe8 *dxpipe;
  DCAST_INTO_V(dxpipe, _pipe);

  nassertv(_gsg == (GraphicsStateGuardian *)NULL);
  _dxgsg = new DXGraphicsStateGuardian8(this);
  _gsg = _dxgsg;

  // Tell the associated dxGSG about the window handle.
  _dxgsg->scrn.hWnd = _hWnd;
 */

  // Create a Direct3D object.

  // these were taken from the 8.0 and 8.1 d3d8.h SDK headers
  #define D3D_SDK_VERSION_8_0  120
  #define D3D_SDK_VERSION_8_1  220

  // are we using 8.0 or 8.1?
  WIN32_FIND_DATA TempFindData;
  HANDLE hFind;
  char tmppath[_MAX_PATH + 128];
  GetSystemDirectory(tmppath, MAX_PATH);
  strcat(tmppath, "\\dpnhpast.dll");
  hFind = FindFirstFile (tmppath, &TempFindData);
  if (hFind != INVALID_HANDLE_VALUE) {
    FindClose(hFind);
    _bIsDX81 = true;
    _pD3D8 = (*_Direct3DCreate8)(D3D_SDK_VERSION_8_1);
  } else {
    _bIsDX81 = false;
    _pD3D8 = (*_Direct3DCreate8)(D3D_SDK_VERSION_8_0);
  }

  if (_pD3D8 == NULL) {
    wdxdisplay8_cat.error() << "Direct3DCreate8(8." << (_bIsDX81 ? "1" : "0") << ") failed!, error=" << GetLastError() << endl;
    //release_gsg();
    goto error;
  }

  Init_D3DFORMAT_map();

  return find_all_card_memavails();

  error:
    // wdxdisplay8_cat.error() << ", error=" << GetLastError << endl;
    return false;
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsPipe8::find_all_card_memavails
//       Access: Private
//  Description: Uses DX7 calls to determine how much video memory is
//               available for each video adapter in the system.
//               Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool wdxGraphicsPipe8::
find_all_card_memavails() {
  HRESULT hr;

  hr = (*_DirectDrawEnumerateExA)(dx7_driver_enum_callback, this, 
                                  DDENUM_ATTACHEDSECONDARYDEVICES | DDENUM_NONDISPLAYDEVICES);
  if (FAILED(hr)) {
    wdxdisplay8_cat.fatal()
      << "DirectDrawEnumerateEx failed" << D3DERRORSTRING(hr);
    return false;
  }

  if (_card_ids.empty()) {
    wdxdisplay8_cat.error()
      << "DirectDrawEnumerateEx enum'ed no devices!\n";
    return false;
  }

  GUID ZeroGUID;
  ZeroMemory(&ZeroGUID, sizeof(GUID));

  if (_card_ids.size() > 1) {
    assert(IsEqualGUID(ZeroGUID, _card_ids[0].DX7_DeviceGUID));
    // delete enum of primary display (always the first), since it is
    // duplicated by explicit entry
    _card_ids.erase(_card_ids.begin());
  }

  for (UINT i=0; i < _card_ids.size(); i++) {
    LPDIRECTDRAW7 pDD;
    BYTE ddd_space[sizeof(DDDEVICEIDENTIFIER2)+4];  //bug in DX7 requires 4 extra bytes for GetDeviceID
    DDDEVICEIDENTIFIER2 *pDX7DeviceID=(DDDEVICEIDENTIFIER2 *)&ddd_space[0];
    GUID *pGUID= &(_card_ids[i].DX7_DeviceGUID);

    if (IsEqualGUID(*pGUID, ZeroGUID)) {
      pGUID=NULL;
    }

    // Create the Direct Draw Object
    hr = (*_DirectDrawCreateEx)(pGUID,(void **)&pDD, IID_IDirectDraw7, NULL);
    if (FAILED(hr)) {
      wdxdisplay8_cat.error()
        << "DirectDrawCreateEx failed for device (" << i
        << ")" << D3DERRORSTRING(hr);
      continue;
    }

    ZeroMemory(ddd_space, sizeof(DDDEVICEIDENTIFIER2));

    hr = pDD->GetDeviceIdentifier(pDX7DeviceID, 0x0);
    if (FAILED(hr)) {
      wdxdisplay8_cat.error()
        << "GetDeviceID failed for device ("<< i << ")" << D3DERRORSTRING(hr);
      continue;
    }

    _card_ids[i].DeviceID = pDX7DeviceID->dwDeviceId;
    _card_ids[i].VendorID = pDX7DeviceID->dwVendorId;

    // Get Current VidMem avail.  Note this is only an estimate, when
    // we switch to fullscreen mode from desktop, more vidmem will be
    // available (typically 1.2 meg).  I dont want to switch to
    // fullscreen more than once due to the annoying monitor flicker,
    // so try to figure out optimal mode using this estimate
    DDSCAPS2 ddsGAVMCaps;
    DWORD dwVidMemTotal,dwVidMemFree;
    dwVidMemTotal=dwVidMemFree=0;
    {
      // print out total INCLUDING AGP just for information purposes
      // and future use.  The real value I'm interested in for
      // purposes of measuring possible valid screen sizes shouldnt
      // include AGP.
      ZeroMemory(&ddsGAVMCaps, sizeof(DDSCAPS2));
      ddsGAVMCaps.dwCaps = DDSCAPS_VIDEOMEMORY;

      hr = pDD->GetAvailableVidMem(&ddsGAVMCaps, &dwVidMemTotal, &dwVidMemFree);
      if (FAILED(hr)) {
        wdxdisplay8_cat.error()
          << "GetAvailableVidMem failed for device #" << i 
          << D3DERRORSTRING(hr);
        //goto skip_device;
        //exit(1);  // probably want to exit, since it may be my fault
      }
    }

    wdxdisplay8_cat.info()
      << "GetAvailableVidMem (including AGP) returns Total: "
      << dwVidMemTotal <<", Free: " << dwVidMemFree
      << " for device #" << i << endl;

    ZeroMemory(&ddsGAVMCaps, sizeof(DDSCAPS2));

    // just want to measure localvidmem, not AGP texmem
    ddsGAVMCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;

    hr = pDD->GetAvailableVidMem(&ddsGAVMCaps, &dwVidMemTotal, &dwVidMemFree);
    if (FAILED(hr)) {
      wdxdisplay8_cat.error() << "GetAvailableVidMem failed for device #"<< i<< D3DERRORSTRING(hr);
      // sometimes GetAvailableVidMem fails with hr=DDERR_NODIRECTDRAWHW for some unknown reason (bad drivers?)
      // see bugs: 15327,18122, others.  is it because D3D8 object has already been created?
      if(hr==DDERR_NODIRECTDRAWHW)
          continue;
      exit(1);  // probably want to exit, since it may be my fault
    }

    wdxdisplay8_cat.info()
      << "GetAvailableVidMem (no AGP) returns Total: " << dwVidMemTotal
      << ", Free: " << dwVidMemFree << " for device #"<< i<< endl;

    pDD->Release();  // release DD obj, since this is all we needed it for

    if (!dx_do_vidmemsize_check) {
      // still calling the DD stuff to get deviceID, etc.  is this necessary?
      _card_ids[i].MaxAvailVidMem = UNKNOWN_VIDMEM_SIZE;
      _card_ids[i].bIsLowVidMemCard = false;
      continue;
    }

    if (dwVidMemTotal == 0) {  // unreliable driver
      dwVidMemTotal = UNKNOWN_VIDMEM_SIZE;
    } else {
      if (!ISPOW2(dwVidMemTotal)) {
        // assume they wont return a proper max value, so
        // round up to next pow of 2
        UINT count=0;
        while ((dwVidMemTotal >> count) != 0x0) {
          count++;
        }
        dwVidMemTotal = (1 << count);
      }
    }

    // after SetDisplayMode, GetAvailVidMem totalmem seems to go down
    // by 1.2 meg (contradicting above comment and what I think would
    // be correct behavior (shouldnt FS mode release the desktop
    // vidmem?), so this is the true value
    _card_ids[i].MaxAvailVidMem = dwVidMemTotal;

    // I can never get this stuff to work reliably, so I'm just
    // rounding up to nearest pow2.  Could try to get
    // HardwareInformation.Memory_size MB number from registry like
    // video control panel, but its not clear how to find the proper
    // registry location for a given card

    // assume buggy drivers (this means you, FireGL2) may return zero
    // (or small amts) for dwVidMemTotal, so ignore value if its < CRAPPY_DRIVER_IS_LYING_VIDMEMTHRESHOLD
    bool bLowVidMemFlag = 
      ((dwVidMemTotal > CRAPPY_DRIVER_IS_LYING_VIDMEMTHRESHOLD) && 
       (dwVidMemTotal< LOWVIDMEMTHRESHOLD));

    _card_ids[i].bIsLowVidMemCard = bLowVidMemFlag;
    wdxdisplay8_cat.info() 
      << "SetLowVidMem flag to " << bLowVidMemFlag
      << " based on adjusted VidMemTotal: " << dwVidMemTotal << endl;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsPipe8::dx7_driver_enum_callback
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
BOOL WINAPI wdxGraphicsPipe8::
dx7_driver_enum_callback(GUID *pGUID, TCHAR *strDesc, TCHAR *strName,
                         VOID *argptr, HMONITOR hm) {
  //    #define PRNT(XX) ((XX!=NULL) ? XX : "NULL")
  //    cout << "strDesc: "<< PRNT(strDesc) << "  strName: "<< PRNT(strName)<<endl;
  wdxGraphicsPipe8 *self = (wdxGraphicsPipe8 *)argptr;

  CardID card_id;
  ZeroMemory(&card_id, sizeof(CardID));

  if (hm == NULL) {
    card_id.hMon = MonitorFromWindow(GetDesktopWindow(), 
                                     MONITOR_DEFAULTTOPRIMARY);
  } else {
    card_id.hMon = hm;
  }

  if (pGUID != NULL) {
    memcpy(&card_id.DX7_DeviceGUID, pGUID, sizeof(GUID));
  }

  card_id.MaxAvailVidMem = UNKNOWN_VIDMEM_SIZE;

  self->_card_ids.push_back(card_id);

  return DDENUMRET_OK;
}

//////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow8::find_best_depth_format
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
bool wdxGraphicsPipe8::
find_best_depth_format(DXScreenData &Display, D3DDISPLAYMODE &TestDisplayMode,
                       D3DFORMAT *pBestFmt, bool bWantStencil,
                       bool bForce16bpp, bool bVerboseMode) const {
  // list fmts in order of preference
#define NUM_TEST_ZFMTS 3
  static D3DFORMAT NoStencilPrefList[NUM_TEST_ZFMTS]={D3DFMT_D32,D3DFMT_D24X8,D3DFMT_D16};
  static D3DFORMAT StencilPrefList[NUM_TEST_ZFMTS]={D3DFMT_D24S8,D3DFMT_D24X4S4,D3DFMT_D15S1};

  // do not use Display.DisplayMode since that is probably not set yet, use TestDisplayMode instead

  // int want_color_bits = _props._want_color_bits;
  // int want_depth_bits = _props._want_depth_bits;  should we pay attn to these so panda user can select bitdepth?

  *pBestFmt = D3DFMT_UNKNOWN;
  HRESULT hr;

    // nvidia likes zbuf depth to match rendertarget depth
  bool bOnlySelect16bpp = (bForce16bpp ||
                           (IS_NVIDIA(Display.DXDeviceID) && IS_16BPP_DISPLAY_FORMAT(TestDisplayMode.Format)));

  if (bVerboseMode) {
    wdxdisplay8_cat.info()
      << "FindBestDepthFmt: bSelectOnly16bpp: " << bOnlySelect16bpp << endl;
  }

  for (int i=0; i < NUM_TEST_ZFMTS; i++) {
    D3DFORMAT TestDepthFmt = 
      (bWantStencil ? StencilPrefList[i] : NoStencilPrefList[i]);

    if (bOnlySelect16bpp && !IS_16BPP_ZBUFFER(TestDepthFmt)) {
      continue;
    }

    hr = Display.pD3D8->CheckDeviceFormat(Display.CardIDNum,
                                          D3DDEVTYPE_HAL,
                                          TestDisplayMode.Format,
                                          D3DUSAGE_DEPTHSTENCIL,
                                          D3DRTYPE_SURFACE,TestDepthFmt);

    if (FAILED(hr)) {
      if (hr == D3DERR_NOTAVAILABLE) {
        if (bVerboseMode)
          wdxdisplay8_cat.info() 
            << "FindBestDepthFmt: ChkDevFmt returns NotAvail for " 
            << D3DFormatStr(TestDepthFmt) << endl;
        continue;
      }

      wdxdisplay8_cat.error()
        << "unexpected CheckDeviceFormat failure" << D3DERRORSTRING(hr) 
        << endl;
      exit(1);
    }

    hr = Display.pD3D8->CheckDepthStencilMatch(Display.CardIDNum,
                                               D3DDEVTYPE_HAL,
                                               TestDisplayMode.Format,   // adapter format
                                               TestDisplayMode.Format,   // backbuffer fmt  (should be the same in my apps)
                                               TestDepthFmt);
    if (SUCCEEDED(hr)) {
      *pBestFmt = TestDepthFmt;
      break;
    } else {
      if (hr==D3DERR_NOTAVAILABLE) {
        if (bVerboseMode) {
          wdxdisplay8_cat.info()
            << "FindBestDepthFmt: ChkDepMatch returns NotAvail for "
            << D3DFormatStr(TestDisplayMode.Format) << ", " 
            << D3DFormatStr(TestDepthFmt) << endl;
        }
      } else {
        wdxdisplay8_cat.error()
          << "unexpected CheckDepthStencilMatch failure for "
          << D3DFormatStr(TestDisplayMode.Format) << ", " 
          << D3DFormatStr(TestDepthFmt) << endl;
        exit(1);
      }
    }
  }

  if (bVerboseMode) {
    wdxdisplay8_cat.info()
      << "FindBestDepthFmt returns fmt " << D3DFormatStr(*pBestFmt) << endl;
  }

  return (*pBestFmt != D3DFMT_UNKNOWN);
}


////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow8::special_check_fullscreen_resolution
//       Access: Private
//  Description: overrides of the general estimator for known working
//               cases
////////////////////////////////////////////////////////////////////
bool wdxGraphicsPipe8::
special_check_fullscreen_resolution(DXScreenData &scrn,UINT x_size,UINT y_size) {
  DWORD VendorId = scrn.DXDeviceID.VendorId;
  DWORD DeviceId = scrn.DXDeviceID.DeviceId;

  switch (VendorId) {
      case 0x8086:  // Intel
        /*for now, just validate all the intel cards at these resolutions.
          I dont have a complete list of intel deviceIDs (missing 82830, 845, etc)
          // Intel i810,i815,82810
          if ((DeviceId==0x7121)||(DeviceId==0x7123)||(DeviceId==0x7125)||
          (DeviceId==0x1132))
        */
        if ((x_size == 640) && (y_size == 480)) {
          return true;
        }
        if ((x_size == 800) && (y_size == 600)) {
          return true;
        }
        if ((x_size == 1024) && (y_size == 768)) {
          return true;
        }
        break;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow8::search_for_valid_displaymode
//       Access: Private
//  Description: All ptr args are output parameters.  If no valid mode
//               found, returns *pSuggestedPixFmt = D3DFMT_UNKNOWN;
////////////////////////////////////////////////////////////////////
void wdxGraphicsPipe8::
search_for_valid_displaymode(DXScreenData &scrn,
                             UINT RequestedX_Size, UINT RequestedY_Size,
                             bool bWantZBuffer, bool bWantStencil,
                             UINT *pSupportedScreenDepthsMask,
                             bool *pCouldntFindAnyValidZBuf,
                             D3DFORMAT *pSuggestedPixFmt,
                             bool bForce16bppZBuffer,
                             bool bVerboseMode) {

  assert(IS_VALID_PTR(scrn.pD3D8));
  HRESULT hr;

#ifndef NDEBUG
  //   no longer true, due to special_check_fullscreen_res, where lowvidmem cards are allowed higher resolutions
  //    if (_dxgsg->scrn.bIsLowVidMemCard)
  //        nassertv((RequestedX_Size==640)&&(RequestedY_Size==480));
#endif

  *pSuggestedPixFmt = D3DFMT_UNKNOWN;
  *pSupportedScreenDepthsMask = 0x0;
  *pCouldntFindAnyValidZBuf = false;

  int cNumModes = scrn.pD3D8->GetAdapterModeCount(scrn.CardIDNum);
  D3DDISPLAYMODE BestDispMode;
  ZeroMemory(&BestDispMode,sizeof(BestDispMode));

  if (bVerboseMode) {
    wdxdisplay8_cat.info()
      << "searching for valid display modes at res: ("
      << RequestedX_Size << "," << RequestedY_Size
      << "), TotalModes: " << cNumModes << endl;
  }

  // ignore memory based checks for min res 640x480.  some cards just
  // dont give accurate memavails.  (should I do the check anyway for
  // 640x480 32bpp?)
  bool bDoMemBasedChecks = 
    ((!((RequestedX_Size==640)&&(RequestedY_Size==480))) &&
     (scrn.MaxAvailVidMem!=UNKNOWN_VIDMEM_SIZE) &&
     (!special_check_fullscreen_resolution(scrn,RequestedX_Size,RequestedY_Size)));

  if (bVerboseMode || wdxdisplay8_cat.is_spam()) {
    wdxdisplay8_cat.info()
      << "DoMemBasedChecks = " << bDoMemBasedChecks << endl;
  }

  for (int i=0; i < cNumModes; i++) {
    D3DDISPLAYMODE dispmode;
    hr = scrn.pD3D8->EnumAdapterModes(scrn.CardIDNum,i,&dispmode);
    if (FAILED(hr)) {
      wdxdisplay8_cat.error()
        << "EnumAdapterDisplayMode failed for device #"
        << scrn.CardIDNum << D3DERRORSTRING(hr);
      exit(1);
    }

    if ((dispmode.Width!=RequestedX_Size) ||
        (dispmode.Height!=RequestedY_Size)) {
      continue;
    }

    if ((dispmode.RefreshRate<60) && (dispmode.RefreshRate>1)) {
      // dont want refresh rates under 60Hz, but 0 or 1 might indicate
      // a default refresh rate, which is usually >=60
      if (bVerboseMode) {
        wdxdisplay8_cat.info()
          << "skipping mode[" << i << "], bad refresh rate: " 
          << dispmode.RefreshRate << endl;
      }
      continue;
    }

    // Note no attempt is made to verify if format will work at
    // requested size, so even if this call succeeds, could still get
    // an out-of-video-mem error

    hr = scrn.pD3D8->CheckDeviceFormat(scrn.CardIDNum, D3DDEVTYPE_HAL, dispmode.Format,
                                               D3DUSAGE_RENDERTARGET, D3DRTYPE_SURFACE,
                                               dispmode.Format);
    if (FAILED(hr)) {
      if (hr==D3DERR_NOTAVAILABLE) {
        if (bVerboseMode) {
          wdxdisplay8_cat.info()
            << "skipping mode[" << i 
            << "], CheckDevFmt returns NotAvail for fmt: "
            << D3DFormatStr(dispmode.Format) << endl;
        }
        continue;
      } else {
        wdxdisplay8_cat.error()
          << "CheckDeviceFormat failed for device #" 
          << scrn.CardIDNum << D3DERRORSTRING(hr);
        exit(1);
      }
    }

    bool bIs16bppRenderTgt = IS_16BPP_DISPLAY_FORMAT(dispmode.Format);
    float RendTgtMinMemReqmt;

    // if we have a valid memavail value, try to determine if we have
    // enough space
    if (bDoMemBasedChecks) {
      // assume user is testing fullscreen, not windowed, so use the
      // dwTotal value see if 3 scrnbufs (front/back/z)at 16bpp at
      // x_size*y_size will fit with a few extra megs for texmem

      // 8MB Rage Pro says it has 6.8 megs Total free and will run at
      // 1024x768, so formula makes it so that is OK

#define REQD_TEXMEM 1800000

      float bytes_per_pixel = (bIs16bppRenderTgt ? 2 : 4);
      
//    cant do this check yet since gsg doesnt exist!
//    assert((_gsg->get_properties().get_frame_buffer_mode() & FrameBufferProperties::FM_double_buffer) != 0);

      // *2 for double buffer

      RendTgtMinMemReqmt = 
        ((float)RequestedX_Size) * ((float)RequestedY_Size) * 
        bytes_per_pixel * 2 + REQD_TEXMEM;

      if (bVerboseMode || wdxdisplay8_cat.is_spam())
        wdxdisplay8_cat.info()
          << "Testing Mode (" <<RequestedX_Size<<"x" << RequestedY_Size 
          << "," << D3DFormatStr(dispmode.Format) << ")\nReqdVidMem: "
          << (int)RendTgtMinMemReqmt << " AvailVidMem: " 
          << scrn.MaxAvailVidMem << endl;

      if (RendTgtMinMemReqmt > scrn.MaxAvailVidMem) {
        if (bVerboseMode || wdxdisplay8_cat.is_debug())
          wdxdisplay8_cat.info()
            << "not enough VidMem for render tgt, skipping display fmt "
            << D3DFormatStr(dispmode.Format) << " (" 
            << (int)RendTgtMinMemReqmt << " > " 
            << scrn.MaxAvailVidMem << ")\n";
        continue;
      }
    }

    if (bWantZBuffer) {
      D3DFORMAT zformat;
      if (!find_best_depth_format(scrn,dispmode, &zformat,
                                  bWantStencil, bForce16bppZBuffer)) {
        *pCouldntFindAnyValidZBuf=true;
        continue;
      }

      float MinMemReqmt = 0.0f;

      if (bDoMemBasedChecks) {
        // test memory again, this time including zbuf size
        float zbytes_per_pixel = (IS_16BPP_ZBUFFER(zformat) ? 2 : 4);
        float MinMemReqmt = RendTgtMinMemReqmt + ((float)RequestedX_Size)*((float)RequestedY_Size)*zbytes_per_pixel;

        if (bVerboseMode || wdxdisplay8_cat.is_spam())
          wdxdisplay8_cat.info()
            << "Testing Mode w/Z (" << RequestedX_Size << "x"
            << RequestedY_Size << "," << D3DFormatStr(dispmode.Format)
            << ")\nReqdVidMem: "<< (int)MinMemReqmt << " AvailVidMem: " 
            << scrn.MaxAvailVidMem << endl;

        if (MinMemReqmt > scrn.MaxAvailVidMem) {
          if (bVerboseMode || wdxdisplay8_cat.is_debug())
            wdxdisplay8_cat.info()
              << "not enough VidMem for RendTgt+zbuf, skipping display fmt "
              << D3DFormatStr(dispmode.Format) << " (" << (int)MinMemReqmt
              << " > " << scrn.MaxAvailVidMem << ")\n";
          continue;
        }
      }

      if ((!bDoMemBasedChecks) || (MinMemReqmt<scrn.MaxAvailVidMem)) {
        if (!IS_16BPP_ZBUFFER(zformat)) {
          // see if things fit with a 16bpp zbuffer

          if (!find_best_depth_format(scrn, dispmode, &zformat, 
                                      bWantStencil, true, bVerboseMode)) {
            if (bVerboseMode)
              wdxdisplay8_cat.info()
                << "FindBestDepthFmt rejected Mode[" << i << "] (" 
                << RequestedX_Size << "x" << RequestedY_Size 
                << "," << D3DFormatStr(dispmode.Format) << endl;
            *pCouldntFindAnyValidZBuf=true;
            continue;
          }
          
          // right now I'm not going to use these flags, just let the
          // create fail out-of-mem and retry at 16bpp
          *pSupportedScreenDepthsMask |= 
            (IS_16BPP_DISPLAY_FORMAT(dispmode.Format) ? DISPLAY_16BPP_REQUIRES_16BPP_ZBUFFER_FLAG : DISPLAY_32BPP_REQUIRES_16BPP_ZBUFFER_FLAG);
        }
      }
    }

    if (bVerboseMode || wdxdisplay8_cat.is_spam())
      wdxdisplay8_cat.info()
        << "Validated Mode (" << RequestedX_Size << "x" 
        << RequestedY_Size << "," << D3DFormatStr(dispmode.Format) << endl;

    switch (dispmode.Format) {
    case D3DFMT_X1R5G5B5:
      *pSupportedScreenDepthsMask |= X1R5G5B5_FLAG;
      break;
    case D3DFMT_X8R8G8B8:
      *pSupportedScreenDepthsMask |= X8R8G8B8_FLAG;
      break;
    case D3DFMT_R8G8B8:
      *pSupportedScreenDepthsMask |= R8G8B8_FLAG;
      break;
    case D3DFMT_R5G6B5:
      *pSupportedScreenDepthsMask |= R5G6B5_FLAG;
      break;
    default:
      // Render target formats should be only D3DFMT_X1R5G5B5,
      // D3DFMT_R5G6B5, D3DFMT_X8R8G8B8 (or R8G8B8?)
      wdxdisplay8_cat.error()
        << "unrecognized supported fmt "<< D3DFormatStr(dispmode.Format)
        << " returned by EnumAdapterDisplayModes!\n";
    }
  }

  // note: this chooses 32bpp, which may not be preferred over 16 for
  // memory & speed reasons on some older cards in particular
  if (*pSupportedScreenDepthsMask & X8R8G8B8_FLAG) {
    *pSuggestedPixFmt = D3DFMT_X8R8G8B8;
  } else if (*pSupportedScreenDepthsMask & R8G8B8_FLAG) {
    *pSuggestedPixFmt = D3DFMT_R8G8B8;
  } else if (*pSupportedScreenDepthsMask & R5G6B5_FLAG) {
    *pSuggestedPixFmt = D3DFMT_R5G6B5;
  } else if (*pSupportedScreenDepthsMask & X1R5G5B5_FLAG) {
    *pSuggestedPixFmt = D3DFMT_X1R5G5B5;
  }

  if (bVerboseMode || wdxdisplay8_cat.is_spam()) {
    wdxdisplay8_cat.info() 
      << "search_for_valid_device returns fmt: "
      << D3DFormatStr(*pSuggestedPixFmt) << endl;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsPipew8::make_device
//       Access: Public, Virtual
//  Description: Creates a new reference to a particular hardware
//               device and associates it with the pipe.
////////////////////////////////////////////////////////////////////
PT(GraphicsDevice) wdxGraphicsPipe8::
make_device(void *scrn) {
  PT(DXGraphicsDevice8) device = new DXGraphicsDevice8(this);
  device->_pScrn = (DXScreenData*) scrn;
  device->_pD3DDevice = device->_pScrn->pD3DDevice;

  _device = device;
  wdxdisplay8_cat.error() << "walla: device" << device << "\n";

  return device.p();

/*
  nassertv(_gsg == (GraphicsStateGuardian *)NULL);
  _dxgsg = new DXGraphicsStateGuardian8(this);
  _gsg = _dxgsg;

  // Tell the associated dxGSG about the window handle.
  _dxgsg->scrn.hWnd = _hWnd;

  if (pD3D8 == NULL) {
    wdxdisplay8_cat.error()
      << "Direct3DCreate8 failed!\n";
    release_gsg();
    return;
  }

  if (!choose_adapter(pD3D8)) {
    wdxdisplay8_cat.error()
      << "Unable to find suitable rendering device.\n";
    release_gsg();
    return;
  }

  create_screen_buffers_and_device(_dxgsg->scrn, dx_force_16bpp_zbuffer);
  */
}
////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsPipew8::make_gsg
//       Access: Public, Virtual
//  Description: Creates a new empty GSG.  Can do no real initialization
//               until window is opened, and we know the full device requirements
////////////////////////////////////////////////////////////////////

PT(GraphicsStateGuardian) wdxGraphicsPipe8::
make_gsg(const FrameBufferProperties &properties) {


  // FrameBufferProperties really belongs as part of the window/renderbuffer specification
  // put here because of GLX multithreading requirement
  PT(DXGraphicsStateGuardian8) gsg = new DXGraphicsStateGuardian8(properties);
  return gsg.p();

/*
  nassertv(_gsg == (GraphicsStateGuardian *)NULL);
  _dxgsg = new DXGraphicsStateGuardian8(this);
  _gsg = _dxgsg;

  // Tell the associated dxGSG about the window handle.
  _dxgsg->scrn.hWnd = _hWnd;

  if (pD3D8 == NULL) {
    wdxdisplay8_cat.error()
      << "Direct3DCreate8 failed!\n";
    release_gsg();
    return;
  }

  if (!choose_adapter(pD3D8)) {
    wdxdisplay8_cat.error()
      << "Unable to find suitable rendering device.\n";
    release_gsg();
    return;
  }

  create_screen_buffers_and_device(_dxgsg->scrn, dx_force_16bpp_zbuffer);
  */
}

map<D3DFORMAT_FLAG,D3DFORMAT> g_D3DFORMATmap;

void Init_D3DFORMAT_map(void) {
  if(g_D3DFORMATmap.size()!=0)
    return;

    #define INSERT_ELEM(XX)  g_D3DFORMATmap[XX##_FLAG] = D3DFMT_##XX;

    INSERT_ELEM(R8G8B8);
    INSERT_ELEM(A8R8G8B8);
    INSERT_ELEM(X8R8G8B8);
    INSERT_ELEM(R5G6B5);
    INSERT_ELEM(X1R5G5B5);
    INSERT_ELEM(A1R5G5B5);
    INSERT_ELEM(A4R4G4B4);
    INSERT_ELEM(R3G3B2);
    INSERT_ELEM(A8);
    INSERT_ELEM(A8R3G3B2);
    INSERT_ELEM(X4R4G4B4);
    INSERT_ELEM(A2B10G10R10);
    INSERT_ELEM(G16R16);
    INSERT_ELEM(A8P8);
    INSERT_ELEM(P8);
    INSERT_ELEM(L8);
    INSERT_ELEM(A8L8);
    INSERT_ELEM(A4L4);
    INSERT_ELEM(V8U8);
    INSERT_ELEM(L6V5U5);
    INSERT_ELEM(X8L8V8U8);
    INSERT_ELEM(Q8W8V8U8);
    INSERT_ELEM(V16U16);
    INSERT_ELEM(W11V11U10);
    INSERT_ELEM(A2W10V10U10);
    INSERT_ELEM(UYVY);
    INSERT_ELEM(YUY2);
    INSERT_ELEM(DXT1);
    INSERT_ELEM(DXT2);
    INSERT_ELEM(DXT3);
    INSERT_ELEM(DXT4);
    INSERT_ELEM(DXT5);
}


const char *D3DFormatStr(D3DFORMAT fmt) {

#define CASESTR(XX) case XX: return #XX;

  switch(fmt) {
    CASESTR(D3DFMT_UNKNOWN);
    CASESTR(D3DFMT_R8G8B8);
    CASESTR(D3DFMT_A8R8G8B8);
    CASESTR(D3DFMT_X8R8G8B8);
    CASESTR(D3DFMT_R5G6B5);
    CASESTR(D3DFMT_X1R5G5B5);
    CASESTR(D3DFMT_A1R5G5B5);
    CASESTR(D3DFMT_A4R4G4B4);
    CASESTR(D3DFMT_R3G3B2);
    CASESTR(D3DFMT_A8);
    CASESTR(D3DFMT_A8R3G3B2);
    CASESTR(D3DFMT_X4R4G4B4);
    CASESTR(D3DFMT_A2B10G10R10);
    CASESTR(D3DFMT_G16R16);
    CASESTR(D3DFMT_A8P8);
    CASESTR(D3DFMT_P8);
    CASESTR(D3DFMT_L8);
    CASESTR(D3DFMT_A8L8);
    CASESTR(D3DFMT_A4L4);
    CASESTR(D3DFMT_V8U8);
    CASESTR(D3DFMT_L6V5U5);
    CASESTR(D3DFMT_X8L8V8U8);
    CASESTR(D3DFMT_Q8W8V8U8);
    CASESTR(D3DFMT_V16U16);
    CASESTR(D3DFMT_W11V11U10);
    CASESTR(D3DFMT_A2W10V10U10);
    CASESTR(D3DFMT_UYVY);
    CASESTR(D3DFMT_YUY2);
    CASESTR(D3DFMT_DXT1);
    CASESTR(D3DFMT_DXT2);
    CASESTR(D3DFMT_DXT3);
    CASESTR(D3DFMT_DXT4);
    CASESTR(D3DFMT_DXT5);
    CASESTR(D3DFMT_D16_LOCKABLE);
    CASESTR(D3DFMT_D32);
    CASESTR(D3DFMT_D15S1);
    CASESTR(D3DFMT_D24S8);
    CASESTR(D3DFMT_D16);
    CASESTR(D3DFMT_D24X8);
    CASESTR(D3DFMT_D24X4S4);
    CASESTR(D3DFMT_VERTEXDATA);
    CASESTR(D3DFMT_INDEX16);
    CASESTR(D3DFMT_INDEX32);
  }

  return "Invalid D3DFORMAT";
}

