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
  _thread = NULL;
  _thread_id = 0;
  _hwnd = NULL;
  _progress_bar = NULL;
  _text_label = NULL;
  _thread_running = false;
  _got_install = false;
  _install_progress = 0.0;
  _install_label_changed = false;

  INIT_LOCK(_install_lock);

  start_thread();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinSplashWindow::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DWinSplashWindow::
~P3DWinSplashWindow() {
  stop_thread();

  DESTROY_LOCK(_install_lock);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinSplashWindow::set_install_label
//       Access: Public, Virtual
//  Description: Specifies the text that is displayed above the
//               install progress bar.
////////////////////////////////////////////////////////////////////
void P3DWinSplashWindow::
set_install_label(const string &install_label) {
  ACQUIRE_LOCK(_install_lock);
  if (_install_label != install_label) {
    _install_label = install_label;
    _install_label_changed = true;
  }
  RELEASE_LOCK(_install_lock);

  // Post a silly message to spin the message loop.
  PostThreadMessage(_thread_id, WM_USER, 0, 0);

  if (!_thread_running && _thread_continue) {
    // The user must have closed the window.  Let's shut down the
    // instance, too.
    _inst->request_stop();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinSplashWindow::set_install_progress
//       Access: Public, Virtual
//  Description: Moves the install progress bar from 0.0 to 1.0.
////////////////////////////////////////////////////////////////////
void P3DWinSplashWindow::
set_install_progress(double install_progress) {
  _got_install = true;

  ACQUIRE_LOCK(_install_lock);
  _install_progress = install_progress;
  RELEASE_LOCK(_install_lock);

  // Post a silly message to spin the message loop.
  PostThreadMessage(_thread_id, WM_USER, 0, 0);

  if (!_thread_running && _thread_continue) {
    // The user must have closed the window.  Let's shut down the
    // instance, too.
    _inst->request_stop();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinSplashWindow::start_thraed
//       Access: Private
//  Description: Spawns the sub-thread.
////////////////////////////////////////////////////////////////////
void P3DWinSplashWindow::
start_thread() {
  _thread_continue = true;
  _thread = CreateThread(NULL, 0, &win_thread_run, this, 0, &_thread_id);
  if (_thread != NULL) {
    _thread_running = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinSplashWindow::stop_thraed
//       Access: Private
//  Description: Terminates and joins the sub-thread.
////////////////////////////////////////////////////////////////////
void P3DWinSplashWindow::
stop_thread() {
  _thread_continue = false;
  // Post a silly message to spin the message loop.
  PostThreadMessage(_thread_id, WM_USER, 0, 0);

  WaitForSingleObject(_thread, INFINITE);
  CloseHandle(_thread);
  _thread = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinSplashWindow::thread_run
//       Access: Private
//  Description: The sub-thread's main run method.
////////////////////////////////////////////////////////////////////
void P3DWinSplashWindow::
thread_run() {
  make_window();

  double last_progress = -1.0;

  _loop_started = GetTickCount();
  MSG msg;
  int retval;
  retval = GetMessage(&msg, NULL, 0, 0);
  while (retval != 0 && _thread_continue) {
    if (retval == -1) {
      nout << "Error processing message queue.\n";
      break;
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);

    if (_got_install) {
      ACQUIRE_LOCK(_install_lock);
      double install_progress = _install_progress;
      if (_install_label_changed && _progress_bar != NULL) {
        update_install_label(_install_label);
      }
      _install_label_changed = false;
      RELEASE_LOCK(_install_lock);

      if (install_progress != last_progress) {
        if (_progress_bar == NULL) {
          // Is it time to create the progress bar?
          int now = GetTickCount();
          if (now - _loop_started > 2000) {
            make_progress_bar();
          }
        } else {
          // Update the progress bar.  We do this only within the
          // thread, to ensure we don't get a race condition when
          // starting or closing the thread.
          SendMessage(_progress_bar, PBM_SETPOS, (int)(install_progress * 100.0), 0);
          
          last_progress = install_progress;
        }
      }
    }

    retval = GetMessage(&msg, NULL, 0, 0);
  }

  close_window();
  _thread_running = false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinSplashWindow::win_thread_run
//       Access: Private, Static
//  Description: The OS-specific thread callback function.
////////////////////////////////////////////////////////////////////
DWORD P3DWinSplashWindow::
win_thread_run(LPVOID data) {
  ((P3DWinSplashWindow *)data)->thread_run();
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinSplashWindow::make_window
//       Access: Private
//  Description: Creates the window for displaying progress.  Runs
//               within the sub-thread.
////////////////////////////////////////////////////////////////////
void P3DWinSplashWindow::
make_window() {
  WNDCLASS wc;

  HINSTANCE application = GetModuleHandle(NULL);

  static bool registered_class = false;
  if (!registered_class) {
    ZeroMemory(&wc, sizeof(WNDCLASS));
    wc.lpfnWndProc = window_proc;
    wc.hInstance = application;
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
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


////////////////////////////////////////////////////////////////////
//     Function: P3DWinSplashWindow::make_progress_bar
//       Access: Private
//  Description: Creates the progress bar and label.  Runs
//               within the sub-thread.  This is done a few seconds
//               after the main window is created, to give us a chance
//               to launch quickly without bothering the user.
////////////////////////////////////////////////////////////////////
void P3DWinSplashWindow::
make_progress_bar() {
  if (_progress_bar != NULL) {
    return;
  }

  ACQUIRE_LOCK(_install_lock);
  string install_label = _install_label;
  double install_progress = _install_progress;
  _install_label_changed = false;
  RELEASE_LOCK(_install_lock);

  HINSTANCE application = GetModuleHandle(NULL);
  DWORD window_style = 
    WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

  RECT rect;
  GetClientRect(_hwnd, &rect);
  int width = rect.right - rect.left;
  int height = rect.bottom - rect.top;

  int bar_width = min((int)(width * 0.6), 400);
  int bar_height = min((int)(height * 0.1), 24);
  int bar_x = (width - bar_width) / 2;
  int bar_y = (height - bar_height) / 2;

  _progress_bar = 
      CreateWindowEx(0, PROGRESS_CLASS, "", window_style,
                     bar_x, bar_y, bar_width, bar_height,
                     _hwnd, NULL, application, 0);
  SendMessage(_progress_bar, PBM_SETPOS, (int)(install_progress * 100.0), 0);
  ShowWindow(_progress_bar, SW_SHOWNORMAL);

  update_install_label(install_label);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinSplashWindow::update_install_label
//       Access: Private
//  Description: Changes the text on the install label.  Runs within
//               the sub-thread.
////////////////////////////////////////////////////////////////////
void P3DWinSplashWindow::
update_install_label(const string &install_label) {
  assert(_progress_bar != NULL);

  if (_text_label != NULL) {
    DestroyWindow(_text_label);
    _text_label = NULL;
  }

  if (install_label.empty()) {
    // Trivial case.
    return;
  }

  // Create a static text label.  What a major pain *this* is.

  const char *text = install_label.c_str();
  HFONT font = (HFONT)GetStockObject(ANSI_VAR_FONT); 

  HDC dc = GetDC(_hwnd);
  SelectObject(dc, font);
  SIZE text_size;
  GetTextExtentPoint32(dc, text, strlen(text), &text_size);
  ReleaseDC(_hwnd, dc);

  HINSTANCE application = GetModuleHandle(NULL);
  DWORD window_style = 
    SS_OWNERDRAW | WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

  RECT rect;
  GetClientRect(_hwnd, &rect);
  int width = rect.right - rect.left;
  int height = rect.bottom - rect.top;

  int bar_height = min((int)(height * 0.1), 24);
  int bar_y = (height - bar_height) / 2;

  int text_width = text_size.cx;
  int text_height = text_size.cy;
  int text_x = (width - text_width) / 2;
  int text_y = bar_y - text_height - 2;

  _text_label = CreateWindowEx(0, "STATIC", text, window_style,
                               text_x, text_y, text_width, text_height,
                               _hwnd, NULL, application, 0);
  ShowWindow(_text_label, SW_SHOWNORMAL);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinSplashWindow::close_window
//       Access: Private
//  Description: Closes the window created above.
////////////////////////////////////////////////////////////////////
void P3DWinSplashWindow::
close_window() {
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
