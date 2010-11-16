// Filename: dxInput8.cxx
// Created by:  angelina jolie (07Oct99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "config_wdxdisplay8.h"
#include "dxInput8.h"

#define AXIS_RESOLUTION 2000   // use this many levels of resolution by default (could be more if needed and device supported it)
#define AXIS_RANGE_CENTERED    // if defined, axis range is centered on 0, instead of starting on 0

BOOL CALLBACK EnumGameCtrlsCallback( const DIDEVICEINSTANCE* pdidInstance,
                                     VOID* pContext ) {
    DI_DeviceInfos *pDevInfos = (DI_DeviceInfos *)pContext;

    (*pDevInfos).push_back(*pdidInstance);

    if(wdxdisplay_cat.is_debug())
        wdxdisplay_cat.debug() << "Found DevType 0x" << (void*)pdidInstance->dwDevType << ": " << pdidInstance->tszInstanceName << ": " << pdidInstance->tszProductName <<endl;

    return DIENUM_CONTINUE;
}

extern BOOL CALLBACK EnumObjectsCallbackJoystick(const DIDEVICEOBJECTINSTANCE* pdidoi,VOID* pContext);

DInput8Info::DInput8Info() {
    _pDInput8 = NULL;
    _hDInputDLL = NULL;
    _JoystickPollTimer = NULL;
}

DInput8Info::~DInput8Info() {
  for(UINT i=0;i<_DeviceList.size();i++) {
      _DeviceList[i]->Unacquire();
      SAFE_RELEASE(_DeviceList[i]);
  }

  // bugbug: need to handle this
  // if(_JoystickPollTimer!=NULL)
  //   KillTimer(...)

  SAFE_RELEASE(_pDInput8);
  if(_hDInputDLL) {
      FreeLibrary(_hDInputDLL);
      _hDInputDLL=NULL;
  }
}

bool DInput8Info::InitDirectInput() {
    HRESULT hr;

    // assumes dx8 exists
    // use dynamic load so non-dinput programs don't have to load dinput
    #define DLLNAME "dinput8.dll"
    #define DINPUTCREATE "DirectInput8Create"

    HINSTANCE _hDInputDLL = LoadLibrary(DLLNAME);
    if(_hDInputDLL == 0) {
        wdxdisplay_cat.fatal() << "LoadLibrary(" << DLLNAME <<") failed!, error=" << GetLastError() << endl;
        exit(1);
    }

    typedef HRESULT (WINAPI * LPDIRECTINPUT8CREATE)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter);
    LPDIRECTINPUT8CREATE pDInputCreate8;

    pDInputCreate8 = (LPDIRECTINPUT8CREATE) GetProcAddress(_hDInputDLL,DINPUTCREATE);
    if(pDInputCreate8 == NULL) {
        wdxdisplay_cat.fatal() << "GetProcAddr failed for " << DINPUTCREATE << endl;
        exit(1);
    }

    // Register with the DirectInput subsystem and get a pointer
    // to a IDirectInput interface we can use.
    // Create a DInput object
    if( FAILED( hr = (*pDInputCreate8)(GetModuleHandle(NULL), DIRECTINPUT_VERSION,
                                         IID_IDirectInput8, (VOID**)&_pDInput8, NULL ) ) ) {
        wdxdisplay_cat.error() << DINPUTCREATE << "failed" << D3DERRORSTRING(hr);
        return false;
    }

    // enum all the joysticks,etc  (but not keybd/mouse)
    if( FAILED( hr = _pDInput8->EnumDevices(DI8DEVCLASS_GAMECTRL,
                                         EnumGameCtrlsCallback,
                                         (LPVOID)&_DevInfos, DIEDFL_ATTACHEDONLY ) ) ) {
        wdxdisplay_cat.error() << "EnumDevices failed" << D3DERRORSTRING(hr);
        return false;
    }

    return true;
}

bool DInput8Info::CreateJoystickOrPad(HWND _window) {
    bool bFoundDev = false;
    UINT devnum=0;
    char *errstr=NULL;

    // look through the list for the first joystick or gamepad
    for(;devnum<_DevInfos.size();devnum++) {
        DWORD devType = GET_DIDEVICE_TYPE(_DevInfos[devnum].dwDevType);
        if((devType==DI8DEVTYPE_GAMEPAD) ||(devType==DI8DEVTYPE_JOYSTICK)) {
          bFoundDev=true;
          break;
        }
    }

    if(!bFoundDev) {
        wdxdisplay_cat.error() << "Cant find an attached Joystick or GamePad!\n";
        return false;
    }

    LPDIRECTINPUTDEVICE8 pJoyDevice;

    // Obtain an interface to the enumerated joystick.
    HRESULT hr = _pDInput8->CreateDevice(_DevInfos[devnum].guidInstance, &pJoyDevice, NULL );
    if(FAILED(hr)) {
        errstr="CreateDevice";
        goto handle_error;
    }

    nassertr(pJoyDevice!=NULL, false);
    _DeviceList.push_back(pJoyDevice);

    // Set the data format to "simple joystick" - a predefined data format
    //
    // A data format specifies which controls on a device we are interested in,
    // and how they should be reported. This tells DInput that we will be
    // passing a DIJOYSTATE2 structure to IDirectInputDevice::GetDeviceState().
    hr = pJoyDevice->SetDataFormat(&c_dfDIJoystick2);
    if(FAILED(hr)) {
        errstr="SetDataFormat";
        goto handle_error;
    }

    // must be called AFTER SetDataFormat to get all the proper flags
    DX_DECLARE_CLEAN(DIDEVCAPS, DIDevCaps);
    hr = pJoyDevice->GetCapabilities(&DIDevCaps);
    nassertr(SUCCEEDED(hr), false);

    _DevCaps.push_back(DIDevCaps);

    if(wdxdisplay_cat.is_debug())
        wdxdisplay_cat.debug() << "Joy/Pad has " << DIDevCaps.dwAxes << " Axes, " <<  DIDevCaps.dwButtons << " Buttons, " <<  DIDevCaps.dwPOVs << " POVs" << endl;

    // Set the cooperative level to let DInput know how this device should
    // interact with the system and with other DInput applications.
    hr = pJoyDevice->SetCooperativeLevel( _window, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
    if(FAILED(hr)) {
        errstr="SetCooperativeLevel";
        goto handle_error;
    }

    // set the min/max values property for discovered axes.
    hr = pJoyDevice->EnumObjects(EnumObjectsCallbackJoystick, (LPVOID)pJoyDevice, DIDFT_AXIS);
    if(FAILED(hr)) {
        errstr="EnumObjects";
        goto handle_error;
    }

    return true;

  handle_error:
    wdxdisplay_cat.error() << errstr << " failed for (" << _DevInfos[devnum].tszInstanceName << ":" << _DevInfos[devnum].tszProductName << ")" << D3DERRORSTRING(hr);
    return false;
}

//-----------------------------------------------------------------------------
// Name: EnumObjectsCallback()
// Desc: Callback function for enumerating objects (axes, buttons, POVs) on a
//       joystick. This function enables user interface elements for objects
//       that are found to exist, and scales axes min/max values.
//-----------------------------------------------------------------------------
BOOL CALLBACK EnumObjectsCallbackJoystick( const DIDEVICEOBJECTINSTANCE* pdidoi,
                                   VOID* pContext ) {

    LPDIRECTINPUTDEVICE8 pJoyDevice = (LPDIRECTINPUTDEVICE8) pContext;
    HRESULT hr;

    // For axes that are returned, set the DIPROP_RANGE property for the
    // enumerated axis in order to scale min/max values.
    if( pdidoi->dwType & DIDFT_AXIS ) {
        DIPROPRANGE diprg;
        diprg.diph.dwSize       = sizeof(DIPROPRANGE);
        diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        diprg.diph.dwHow        = DIPH_BYID;
        diprg.diph.dwObj        = pdidoi->dwType; // Specify the enumerated axis

     #ifdef AXIS_RANGE_CENTERED
        diprg.lMin              = -AXIS_RESOLUTION/2;
        diprg.lMax              = +AXIS_RESOLUTION/2;
     #else
        diprg.lMin              = 0;
        diprg.lMax              = +AXIS_RESOLUTION;
     #endif

        // Set the range for the axis
        hr = pJoyDevice->SetProperty( DIPROP_RANGE, &diprg.diph);
        if(FAILED(hr)) {
          wdxdisplay_cat.error() << "SetProperty on axis failed" << D3DERRORSTRING(hr);
          return DIENUM_STOP;
        }
    }

    return DIENUM_CONTINUE;
}

bool DInput8Info::ReadJoystick(int devnum, DIJOYSTATE2 &js) {
    LPDIRECTINPUTDEVICE8 pJoystick = _DeviceList[devnum];
    nassertr(pJoystick!=NULL, false);
    HRESULT hr;
    char *errstr;

    // Poll the device to read the current state

    hr = pJoystick->Poll();

    if( FAILED(hr) ) {
        // DInput is telling us that the input stream has been
        // interrupted. We aren't tracking any state between polls, so
        // we don't have any special reset that needs to be done. We
        // just re-acquire and try again.

        if((hr==DIERR_NOTACQUIRED)||(hr == DIERR_INPUTLOST)) {
            hr = pJoystick->Acquire();

            if(FAILED(hr)) {
                if(wdxdisplay_cat.is_spam())
                   wdxdisplay_cat.spam() << "Acquire failed" << D3DERRORSTRING(hr);

                // hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
                // may occur when the app is minimized or in the process of
                // switching, so just try again later
                return false;
            }

            hr = pJoystick->Poll();
            if(FAILED(hr)) {
                // should never happen!
                errstr = "Poll after successful Acquire failed";
                goto handle_error;
            }
        } else {
            errstr =  "Unknown Poll failure";
            goto handle_error;
        }
    }

    // should we make a vector of devstate dataformats to generalize this fn for all device types?

    // Get the input's device state
    hr = pJoystick->GetDeviceState( sizeof(DIJOYSTATE2), &js);
    if(FAILED(hr)) {
        errstr =  "GetDeviceState failed";
        goto handle_error;
    }

    return true;

  handle_error:
     wdxdisplay_cat.fatal() << errstr << D3DERRORSTRING(hr);
     return false;
}


