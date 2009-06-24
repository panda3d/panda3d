// Filename: p3dWinSplashWindow.h
// Created by:  drose (17Jun09)
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

#ifndef P3DWINSPLASHWINDOW_H
#define P3DWINSPLASHWINDOW_H

#include "p3d_plugin_common.h"

#ifdef _WIN32

#include "p3dSplashWindow.h"
#include "p3d_lock.h"

#include <windows.h>

////////////////////////////////////////////////////////////////////
//       Class : P3DWinSplashWindow
// Description : This is the Windows implementation of the
//               initial-download window.
////////////////////////////////////////////////////////////////////
class P3DWinSplashWindow : public P3DSplashWindow {
public:
  P3DWinSplashWindow(P3DInstance *inst);
  virtual ~P3DWinSplashWindow();

  virtual void open_window();
  virtual void close_window();

private:
  static LONG WINAPI window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
  HWND _hwnd;
};

#include "p3dWinSplashWindow.I"

#endif  // _WIN32

#endif
