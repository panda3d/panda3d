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
#include "wdxGraphicsWindow8.h"
#include "config_dxgsg8.h"

TypeHandle wdxGraphicsPipe8::_type_handle;

// #define LOWVIDMEMTHRESHOLD 3500000
// #define CRAPPY_DRIVER_IS_LYING_VIDMEMTHRESHOLD 1000000
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
  _is_valid = init();
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsPipe8::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
wdxGraphicsPipe8::
~wdxGraphicsPipe8() {
  if (_hDDrawDLL != NULL) {
    FreeLibrary(_hDDrawDLL);
    _hDDrawDLL = NULL;
  }
  if(_hD3D8_DLL != NULL) {
    FreeLibrary(_hD3D8_DLL);
    _hD3D8_DLL = NULL;
  }
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
make_window() {
  if (!_is_valid) {
    return NULL;
  }

  return new wdxGraphicsWindow8(this);
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
  static const char * const ddraw_name = "ddraw.dll";
  _hDDrawDLL = LoadLibrary(ddraw_name);
  if(_hDDrawDLL == 0) {
    wdxdisplay8_cat.error()
      << "can't locate " << ddraw_name << "!\n";
    return false;
  }

  _DirectDrawCreateEx = 
    (LPDIRECTDRAWCREATEEX)GetProcAddress(_hDDrawDLL, "DirectDrawCreateEx");
  if (_DirectDrawCreateEx == NULL) {
    wdxdisplay8_cat.error()
      << "GetProcAddr failed for DDCreateEx" << endl;
    return false;
  }

  _DirectDrawEnumerateExA = (LPDIRECTDRAWENUMERATEEX)GetProcAddress(_hDDrawDLL, "DirectDrawEnumerateExA");
  if (_DirectDrawEnumerateExA == NULL) {
    wdxdisplay8_cat.error()
      << "GetProcAddr failed for DirectDrawEnumerateEx! (win95 system?)\n";
    return false;
  }

  static const char * const d3d8_name = "d3d8.dll";
  _hD3D8_DLL = LoadLibrary(d3d8_name);
  if (_hD3D8_DLL == 0) {
    wdxdisplay8_cat.error()
      << "PandaDX8 requires DX8, can't locate " << d3d8_name << "!\n";
    return false;
  }

  // dont want to statically link to possibly non-existent d3d8 dll,
  // so must call D3DCr8 indirectly
  static const char * const d3dcreate8_name = "Direct3DCreate8";
  _Direct3DCreate8 =
    (Direct3DCreate8_ProcPtr)GetProcAddress(_hD3D8_DLL, d3dcreate8_name);

  if (_Direct3DCreate8 == NULL) {
    wdxdisplay8_cat.error()
      << "GetProcAddress for " << d3dcreate8_name << "failed!" << endl;
    return false;
  }

  return find_all_card_memavails();
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
        // goto skip_device;
        exit(1);  // probably want to exit, since it may be my fault
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
      wdxdisplay8_cat.error()
        << "GetAvailableVidMem failed for device #"<< i<< D3DERRORSTRING(hr);
      // goto skip_device;
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
    // (or small amts) for dwVidMemTotal, so ignore value if its <
    // CRAPPY_DRIVER_IS_LYING_VIDMEMTHRESHOLD
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
