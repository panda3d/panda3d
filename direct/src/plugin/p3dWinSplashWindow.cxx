// Filename: p3dWinSplashWindow.cxx
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

#include "p3dWinSplashWindow.h"

#ifdef _WIN32

////////////////////////////////////////////////////////////////////
//     Function: P3DWinSplashWindow::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DWinSplashWindow::
P3DWinSplashWindow(P3DInstance *inst) : 
  P3DSplashWindow(inst)
{
  _hwnd = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinSplashWindow::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DWinSplashWindow::
~P3DWinSplashWindow() {
  close_window();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinSplashWindow::open_window
//       Access: Public, Virtual
//  Description: Creates the splash window.
////////////////////////////////////////////////////////////////////
void P3DWinSplashWindow::
open_window() {
  P3DSplashWindow::open_window();
  WNDCLASS wc;

  HINSTANCE application = GetModuleHandle(NULL);

  static bool registered_class = false;
  if (!registered_class) {
    ZeroMemory(&wc, sizeof(WNDCLASS));
    wc.lpfnWndProc = window_proc;
    wc.hInstance = application;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = "panda3d_splash";
    
    if (!RegisterClass(&wc)) {
      nout << "Could not register window class!\n";
    }
    registered_class = true;
  }
  
  int x = CW_USEDEFAULT;
  int y = CW_USEDEFAULT;
  if (_wparams.get_win_x() != 0 && _wparams.get_win_y() != 0) {
    x = _wparams.get_win_x();
    y = _wparams.get_win_y();
  }
  
  int width = 320;
  int height = 240;
  if (_wparams.get_win_width() != 0 && _wparams.get_win_height() != 0) {
    width = _wparams.get_win_width();
    height = _wparams.get_win_height();
  }

  if (_wparams.get_window_type() == P3D_WT_embedded) {
    // Create an embedded window.
    DWORD window_style = 
      WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

    HWND parent_hwnd = _wparams.get_parent_window()._hwnd;

    _hwnd = 
      CreateWindow("panda3d_splash", "Panda3D", window_style,
                   x, y, width, height,
                   parent_hwnd, NULL, application, 0);
    
    if (!_hwnd) {
      nout << "Could not create embedded window!\n";
      return;
    }

  } else {
    // Create a toplevel window.
    DWORD window_style = 
      WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
      WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX |
      WS_SIZEBOX | WS_MAXIMIZEBOX;
    
    _hwnd = 
      CreateWindow("panda3d_splash", "Panda3D", window_style,
                   x, y, width, height,
                   NULL, NULL, application, 0);
    if (!_hwnd) {
      nout << "Could not create toplevel window!\n";
      return;
    }
    ShowWindow(_hwnd, SW_SHOWNORMAL);
  }
}

  // Ensure the main window is visible now.
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinSplashWindow::close_window
//       Access: Public, Virtual
//  Description: Closes the window created above.
////////////////////////////////////////////////////////////////////
void P3DWinSplashWindow::
close_window() {
  P3DSplashWindow::close_window();
  if (_hwnd) {
    ShowWindow(_hwnd, SW_HIDE);
    CloseWindow(_hwnd);
    _hwnd = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinSplashWindow::window_proc
//       Access: Private, Static
//  Description: The windows event-processing handler.
////////////////////////////////////////////////////////////////////
LONG P3DWinSplashWindow::
window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
  case WM_DESTROY:
    PostQuitMessage(0);
    break;

  case WM_DRAWITEM:
    // Draw a text label placed within the window.
    {
      DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *)lparam;
      FillRect(dis->hDC, &(dis->rcItem), WHITE_BRUSH);

      static const int text_buffer_size = 512;
      char text_buffer[text_buffer_size];
      GetWindowText(dis->hwndItem, text_buffer, text_buffer_size);

      HFONT font = (HFONT)GetStockObject(ANSI_VAR_FONT); 
      SelectObject(dis->hDC, font);
      SetBkColor(dis->hDC, 0x00ffffff);

      DrawText(dis->hDC, text_buffer, -1, &(dis->rcItem), 
               DT_BOTTOM | DT_CENTER | DT_SINGLELINE);
    }
  };

  return DefWindowProc(hwnd, msg, wparam, lparam);
}


#endif  // _WIN32
