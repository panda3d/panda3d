// Filename: p3dWinProgressWindow.cxx
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

#include "p3dWinProgressWindow.h"

#ifdef _WIN32

////////////////////////////////////////////////////////////////////
//     Function: P3DWinProgressWindow::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DWinProgressWindow::
P3DWinProgressWindow(P3DPackage *package, P3DSession *session,
                     P3DInstance *inst) : 
  P3DProgressWindow(package, session, inst)
{
  _thread = NULL;
  _hwnd = NULL;
  _progress_bar = NULL;
  _thread_running = false;

  INIT_LOCK(_progress_lock);

  assert(_window_type != P3D_WT_hidden);
  start_thread();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinProgressWindow::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DWinProgressWindow::
~P3DWinProgressWindow() {
  stop_thread();

  DESTROY_LOCK(_progress_lock);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinProgressWindow::install_progress
//       Access: Public, Virtual
//  Description: This callback is received during the download process
//               to inform us how much has been installed so far.
////////////////////////////////////////////////////////////////////
void P3DWinProgressWindow::
install_progress(P3DPackage *package, double progress) {
  P3DProgressWindow::install_progress(package, progress);

  ACQUIRE_LOCK(_progress_lock);
  _progress = progress;
  RELEASE_LOCK(_progress_lock);

  // Post a silly message to spin the message loop.
  PostThreadMessage(_thread_id, WM_USER, 0, 0);

  if (!_thread_running) {
    _inst->request_stop();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinProgressWindow::start_thraed
//       Access: Private
//  Description: Spawns the sub-thread.
////////////////////////////////////////////////////////////////////
void P3DWinProgressWindow::
start_thread() {
  _thread_continue = true;
  _thread = CreateThread(NULL, 0, &win_thread_run, this, 0, &_thread_id);
  if (_thread != NULL) {
    _thread_running = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinProgressWindow::stop_thraed
//       Access: Private
//  Description: Terminates and joins the sub-thread.
////////////////////////////////////////////////////////////////////
void P3DWinProgressWindow::
stop_thread() {
  _thread_continue = false;
  // Post a silly message to spin the message loop.
  PostThreadMessage(_thread_id, WM_USER, 0, 0);

  WaitForSingleObject(_thread, INFINITE);
  CloseHandle(_thread);
  _thread = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinProgressWindow::thread_run
//       Access: Private
//  Description: The sub-thread's main run method.
////////////////////////////////////////////////////////////////////
void P3DWinProgressWindow::
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

    {
      ACQUIRE_LOCK(_progress_lock);
      double new_progress = _progress;
      RELEASE_LOCK(_progress_lock);

      if (new_progress != last_progress) {
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
          SendMessage(_progress_bar, PBM_SETPOS, (int)(new_progress * 100.0), 0);
          
          last_progress = new_progress;
        }
      }
    }

    retval = GetMessage(&msg, NULL, 0, 0);
  }

  close_window();
  _thread_running = false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinProgressWindow::win_thread_run
//       Access: Private, Static
//  Description: The OS-specific thread callback function.
////////////////////////////////////////////////////////////////////
DWORD P3DWinProgressWindow::
win_thread_run(LPVOID data) {
  ((P3DWinProgressWindow *)data)->thread_run();
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinProgressWindow::make_window
//       Access: Private
//  Description: Creates the window for displaying progress.  Runs
//               within the sub-thread.
////////////////////////////////////////////////////////////////////
void P3DWinProgressWindow::
make_window() {
  WNDCLASS wc;

  HINSTANCE application = GetModuleHandle(NULL);

  static bool registered_class = false;
  if (!registered_class) {
    ZeroMemory(&wc, sizeof(WNDCLASS));
    wc.lpfnWndProc = window_proc;
    wc.hInstance = application;
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszClassName = "panda3d_progress";
    
    if (!RegisterClass(&wc)) {
      nout << "Could not register window class!\n";
    }
    registered_class = true;
  }
  
  int x = CW_USEDEFAULT;
  int y = CW_USEDEFAULT;
  if (_win_x != 0 && _win_y != 0) {
    x = _win_x;
    y = _win_y;
  }
  
  int width = 320;
  int height = 240;
  if (_win_width != 0 && _win_height != 0) {
    width = _win_width;
    height = _win_height;
  }

  if (_window_type == P3D_WT_embedded) {
    // Create an embedded window.
    DWORD window_style = 
      WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

    HWND parent_hwnd = _parent_window._hwnd;

    _hwnd = 
      CreateWindow("panda3d_progress", "Panda3D", window_style,
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
      CreateWindow("panda3d_progress", "Panda3D", window_style,
                   x, y, width, height,
                   NULL, NULL, application, 0);
    if (!_hwnd) {
      nout << "Could not create toplevel window!\n";
      return;
    }

    // We don't initially show the toplevel window.  We'll show it
    // when we create the actual progress bar, a few seconds later;
    // otherwise, it might be distracting if the window pops up and
    // then immediately disappears.
  }
}


////////////////////////////////////////////////////////////////////
//     Function: P3DWinProgressWindow::make_progress_bar
//       Access: Private
//  Description: Creates the progress bar and label.  Runs
//               within the sub-thread.  This is done a few seconds
//               after the main window is created, to give us a chance
//               to launch quickly without bothering the user.
////////////////////////////////////////////////////////////////////
void P3DWinProgressWindow::
make_progress_bar() {
  if (_progress_bar != NULL) {
    return;
  }

  HINSTANCE application = GetModuleHandle(NULL);
  DWORD window_style = 
    WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

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

  // Create a static text label.  What a major pain *this* is.
  string text_string = "Installing " + _package->get_package_output_name();
  const char *text = text_string.c_str();
  HFONT font = (HFONT)GetStockObject(ANSI_VAR_FONT); 

  HDC dc = GetDC(_hwnd);
  SelectObject(dc, font);
  SIZE text_size;
  GetTextExtentPoint32(dc, text, strlen(text), &text_size);
  ReleaseDC(_hwnd, dc);

  int text_width = text_size.cx;
  int text_height = text_size.cy;
  int text_x = (width - text_width) / 2;
  int text_y = bar_y - text_height - 2;

  CreateWindowEx(0, "STATIC", text, SS_OWNERDRAW | window_style,
                 text_x, text_y, text_width, text_height,
                 _hwnd, NULL, application, 0);

  // Ensure the main window is visible now.
  ShowWindow(_hwnd, SW_SHOWNORMAL);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinProgressWindow::close_window
//       Access: Private
//  Description: Closes the window created above.
////////////////////////////////////////////////////////////////////
void P3DWinProgressWindow::
close_window() {
  if (_hwnd) {
    ShowWindow(_hwnd, SW_HIDE);
    CloseWindow(_hwnd);
    _hwnd = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinProgressWindow::window_proc
//       Access: Private, Static
//  Description: The windows event-processing handler.
////////////////////////////////////////////////////////////////////
LONG P3DWinProgressWindow::
window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
  case WM_DESTROY:
    PostQuitMessage(0);
    break;

  case WM_DRAWITEM:
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
