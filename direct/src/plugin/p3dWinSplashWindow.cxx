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

bool P3DWinSplashWindow::_registered_window_class = false;

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
  _bitmap = NULL;
  _progress_bar = NULL;
  _text_label = NULL;
  _thread_running = false;
  _got_install = false;
  _image_filename_changed = false;
  _image_filename_temp = false;
  _install_label_changed = false;
  _install_progress = 0.0;

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
//     Function: P3DWinSplashWindow::set_image_filename
//       Access: Public, Virtual
//               displayed in the center of the splash window.  If
//               image_filename_temp is true, the file is immediately
//               deleted after it has been read.
////////////////////////////////////////////////////////////////////
void P3DWinSplashWindow::
set_image_filename(const string &image_filename,
                   bool image_filename_temp) {
  ACQUIRE_LOCK(_install_lock);
  if (_image_filename != image_filename) {
    _image_filename = image_filename;
    _image_filename_temp = image_filename_temp;
    _image_filename_changed = true;
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
//     Function: P3DWinSplashWindow::register_window_class
//       Access: Public, Static
//  Description: Registers the window class for this window, if
//               needed.
////////////////////////////////////////////////////////////////////
void P3DWinSplashWindow::
register_window_class() {
  if (!_registered_window_class) {
    HINSTANCE application = GetModuleHandle(NULL);

    WNDCLASS wc;
    ZeroMemory(&wc, sizeof(WNDCLASS));
    wc.lpfnWndProc = st_window_proc;
    wc.hInstance = application;
    wc.lpszClassName = "panda3d_splash";
    
    if (!RegisterClass(&wc)) {
      nout << "Could not register window class panda3d_splash\n";
    }
    _registered_window_class = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinSplashWindow::unregister_window_class
//       Access: Public, Static
//  Description: Unregisters the window class for this window.  It is
//               necessary to do this before unloading the DLL.
////////////////////////////////////////////////////////////////////
void P3DWinSplashWindow::
unregister_window_class() {
  if (_registered_window_class) {
    HINSTANCE application = GetModuleHandle(NULL);

    if (!UnregisterClass("panda3d_splash", application)) {
      nout << "Could not unregister window class panda3d_splash\n";
    }
    _registered_window_class = false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinSplashWindow::start_thread
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
//     Function: P3DWinSplashWindow::stop_thread
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
      if (_image_filename_changed) {
        update_image_filename(_image_filename, _image_filename_temp);
      }
      _image_filename_changed = false;
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
  register_window_class();
  HINSTANCE application = GetModuleHandle(NULL);
  
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
      WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

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
  }
  SetWindowLongPtr(_hwnd, GWLP_USERDATA, (LONG_PTR)this);
  ShowWindow(_hwnd, SW_SHOWNORMAL);
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
  int bar_y = (height - bar_height * 2);

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
  int bar_y = (height - bar_height * 2);

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
//     Function: P3DWinSplashWindow::update_image_filename
//       Access: Private
//  Description: Loads the splash image, converts to to BITMAP form,
//               and stores it in _bitmap.  Runs only in the
//               sub-thread.
////////////////////////////////////////////////////////////////////
void P3DWinSplashWindow::
update_image_filename(const string &image_filename, bool image_filename_temp) {
  // Clear the old image.
  if (_bitmap != NULL) {
    DeleteObject(_bitmap);
    _bitmap = NULL;
  }

  // Since we'll be displaying a new image, we need to refresh the
  // window.
  InvalidateRect(_hwnd, NULL, TRUE);

  // Go read the image.
  string data;
  int num_channels, row_stride;
  if (!read_image(image_filename, image_filename_temp, 
                  _bitmap_height, _bitmap_width, num_channels, row_stride,
                  data)) {
    return;
  }

  // Massage the data into Windows' conventions.
  int new_row_stride = (_bitmap_width * 3);
  // DWORD-pad the row.
  new_row_stride = 4 * ((new_row_stride + 3) / 4);

  int new_data_length = new_row_stride * _bitmap_height;
  char *new_data = new char[new_data_length];

  if (num_channels == 3) {
    // We have to reverse the order of the RGB channels: libjpeg and
    // Windows follow an opposite convention.
    for (int yi = 0; yi < _bitmap_height; ++yi) {
      const char *sp = data.data() + yi * row_stride;
      char *dp = new_data + yi * new_row_stride;
      for (int xi = 0; xi < _bitmap_width; ++xi) {
        dp[0] = sp[2];
        dp[1] = sp[1];
        dp[2] = sp[0];
        sp += num_channels;
        dp += 3;
      }
    }
  } else if (num_channels == 1) {
    // A grayscale image.  Replicate out the channels.
    for (int yi = 0; yi < _bitmap_height; ++yi) {
      const char *sp = data.data() + yi * row_stride;
      char *dp = new_data + yi * new_row_stride;
      for (int xi = 0; xi < _bitmap_width; ++xi) {
        dp[0] = sp[0];
        dp[1] = sp[0];
        dp[2] = sp[0];
        sp += num_channels;
        dp += 3;
      }
    }
  }

  // Now load the image.
  BITMAPINFOHEADER bmih;
  bmih.biSize = sizeof(bmih);
  bmih.biWidth = _bitmap_width;
  bmih.biHeight = -_bitmap_height;
  bmih.biPlanes = 1;
  bmih.biBitCount = 24;
  bmih.biCompression = BI_RGB;
  bmih.biSizeImage = 0;
  bmih.biXPelsPerMeter = 0;
  bmih.biYPelsPerMeter = 0;
  bmih.biClrUsed = 0;
  bmih.biClrImportant = 0;

  BITMAPINFO bmi;
  memcpy(&bmi, &bmih, sizeof(bmih));

  HDC dc = GetDC(_hwnd);
  _bitmap = CreateDIBitmap(dc, &bmih, CBM_INIT, new_data, &bmi, 0);
  ReleaseDC(_hwnd, dc);

  delete[] new_data;

  nout << "Loaded splash file image: " << image_filename << "\n"
       << flush;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinSplashWindow::close_window
//       Access: Private
//  Description: Closes the window created above.
////////////////////////////////////////////////////////////////////
void P3DWinSplashWindow::
close_window() {
  if (_hwnd != NULL) {
    ShowWindow(_hwnd, SW_HIDE);
    CloseWindow(_hwnd);
    _hwnd = NULL;
  }

  if (_bitmap != NULL) {
    DeleteObject(_bitmap);
    _bitmap = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinSplashWindow::paint_window
//       Access: Private
//  Description: Paints the contents of the window into the indicated
//               DC.
////////////////////////////////////////////////////////////////////
void P3DWinSplashWindow::
paint_window(HDC dc) {
  RECT rect;
  GetClientRect(_hwnd, &rect);
  int win_width = rect.right - rect.left;
  int win_height = rect.bottom - rect.top;
  
  if (_bitmap != NULL) {
    // Paint the background splash image.
    HDC mem_dc = CreateCompatibleDC(dc);
    SelectObject(mem_dc, _bitmap);
    
    // Determine the relative size of bitmap and window.
    int bm_width = _bitmap_width;
    int bm_height = _bitmap_height;
    
    int win_cx = win_width / 2;
    int win_cy = win_height / 2;
    
    if (bm_width <= win_width && bm_height <= win_height) {
      // The bitmap fits within the window; center it.
      
      // This is the top-left corner of the bitmap in window coordinates.
      int p_x = win_cx - bm_width / 2;
      int p_y = win_cy - bm_height / 2;
      
      BitBlt(dc, p_x, p_y, bm_width, bm_height,
             mem_dc, 0, 0, SRCCOPY);

      // Now don't paint over this in the below FillRect().
      ExcludeClipRect(dc, p_x, p_y, p_x + bm_width, p_y + bm_height);
      
    } else {
      // The bitmap is larger than the window; scale it down.
      double x_scale = (double)win_width / (double)bm_width;
      double y_scale = (double)win_height / (double)bm_height;
      double scale = min(x_scale, y_scale);
      int sc_width = (int)(bm_width * scale);
      int sc_height = (int)(bm_height * scale);
      
      int p_x = win_cx - sc_width / 2;
      int p_y = win_cy - sc_height / 2;
      StretchBlt(dc, p_x, p_y, sc_width, sc_height,
                 mem_dc, 0, 0, bm_width, bm_height, SRCCOPY);

      // Now don't paint over this in the below FillRect().
      ExcludeClipRect(dc, p_x, p_y, p_x + sc_width, p_y + sc_height);
    }
    
    SelectObject(mem_dc, NULL);
    DeleteDC(mem_dc);
  }

  // Paint everything else the background color.
  FillRect(dc, &rect, WHITE_BRUSH);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinSplashWindow::window_proc
//       Access: Private
//  Description: The windows event-processing handler.
////////////////////////////////////////////////////////////////////
LONG P3DWinSplashWindow::
window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
  case WM_DESTROY:
    PostQuitMessage(0);
    break;

  case WM_SIZE:
    InvalidateRect(hwnd, NULL, FALSE);
    break;

  case WM_ERASEBKGND:
    return true;

  case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC dc = BeginPaint(hwnd, &ps);
      paint_window(dc);
      EndPaint(hwnd, &ps);
    }
    return true;

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

////////////////////////////////////////////////////////////////////
//     Function: P3DWinSplashWindow::st_window_proc
//       Access: Private, Static
//  Description: The windows event-processing handler, static version.
////////////////////////////////////////////////////////////////////
LONG P3DWinSplashWindow::
st_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  LONG_PTR self = GetWindowLongPtr(hwnd, GWLP_USERDATA);
  if (self == NULL) {
    // We haven't assigned the pointer yet.
    return DefWindowProc(hwnd, msg, wparam, lparam);
  }

  return ((P3DWinSplashWindow *)self)->window_proc(hwnd, msg, wparam, lparam);
}

#endif  // _WIN32
