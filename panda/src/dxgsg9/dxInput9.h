// Filename: dxInput8.h
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
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef DXINPUT9_H
#define DXINPUT9_H

#define DIRECTINPUT_VERSION 0x800
#include <dinput.h>
typedef vector<DIDEVICEINSTANCE> DI_DeviceInfos;
typedef vector<DIDEVICEOBJECTINSTANCE> DI_DeviceObjInfos;

class DInput9Info {
public:
 DInput9Info();
 ~DInput9Info();
 bool InitDirectInput();
 bool CreateJoystickOrPad(HWND hWnd);
 bool ReadJoystick(int devnum, DIJOYSTATE2 &js);

 HINSTANCE _hDInputDLL;
 UINT_PTR  _JoystickPollTimer;
 LPDIRECTINPUT8 _pDInput9;
 DI_DeviceInfos _DevInfos;
 // arrays for all created devices.  Should probably put these together in a struct,
 // along with the data fmt info
 vector<LPDIRECTINPUTDEVICE8> _DeviceList;
 vector<DIDEVCAPS> _DevCaps;
};

#endif
