// Filename: p3dWinAuthDialog.cxx
// Created by:  drose (16Sep09)
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

#include "p3dWinAuthDialog.h"

#ifdef _WIN32

bool P3DWinAuthDialog::_registered_window_class = false;

////////////////////////////////////////////////////////////////////
//     Function: P3DWinAuthDialog::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DWinAuthDialog::
P3DWinAuthDialog() {
  _hwnd = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinAuthDialog::open
//       Access: Public, Virtual
//  Description: Displays the dialog and waits for user to click a
//               button.
////////////////////////////////////////////////////////////////////
void P3DWinAuthDialog::
open() {
  close();
  make_window();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinAuthDialog::close
//       Access: Public, Virtual
//  Description: Closes the dialog prematurely.
////////////////////////////////////////////////////////////////////
void P3DWinAuthDialog::
close() {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinAuthDialog::register_window_class
//       Access: Public, Static
//  Description: Registers the window class for this window, if
//               needed.
////////////////////////////////////////////////////////////////////
void P3DWinAuthDialog::
register_window_class() {
  if (!_registered_window_class) {
    HINSTANCE application = GetModuleHandle(NULL);

    WNDCLASS wc;
    ZeroMemory(&wc, sizeof(WNDCLASS));
    wc.lpfnWndProc = st_window_proc;
    wc.hInstance = application;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = HBRUSH(COLOR_WINDOW + 1);
    wc.lpszClassName = "panda3d_auth_dialog";
    
    if (!RegisterClass(&wc)) {
      nout << "Could not register window class panda3d_auth_dialog\n";
    }
    _registered_window_class = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinAuthDialog::unregister_window_class
//       Access: Public, Static
//  Description: Unregisters the window class for this window.  It is
//               necessary to do this before unloading the DLL.
////////////////////////////////////////////////////////////////////
void P3DWinAuthDialog::
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
//     Function: P3DWinAuthDialog::make_window
//       Access: Private
//  Description: Creates the window.
////////////////////////////////////////////////////////////////////
void P3DWinAuthDialog::
make_window() {
  register_window_class();
  HINSTANCE application = GetModuleHandle(NULL);
  
  int x = CW_USEDEFAULT;
  int y = CW_USEDEFAULT;
  
  int width = 320;
  int height = 240;

  // Create a toplevel window.
  DWORD window_style = 
    WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
    WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
  
  _hwnd = 
    CreateWindow("panda3d_auth_dialog", "New Panda3D Application",
                 window_style,
                 x, y, width, height,
                 NULL, NULL, application, 0);
  if (!_hwnd) {
    nout << "Could not create toplevel window!\n";
    return;
  }

  SetWindowLongPtr(_hwnd, GWLP_USERDATA, (LONG_PTR)this);

  // Stack items vertically from the top of the window.
  static const int margin = 10;
  int cy = margin;
  int maxx = 0;

  // Set up the window text.
  string header, text;
  get_text(header, text);

  HFONT font = (HFONT)GetStockObject(ANSI_VAR_FONT); 

  HWND header_label = NULL;
  int header_width, header_height;
  if (!header.empty()) {
    header_label = make_static_text(header_width, header_height, font, header);
    SetWindowPos(header_label, 0, margin, cy, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    cy += header_height;
    maxx = max(maxx, header_width);
    cy += margin;
  }

  int text_width, text_height;
  HWND text_label = make_static_text(text_width, text_height, font, text);
  SetWindowPos(text_label, 0, margin, cy, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
  cy += text_height;
  maxx = max(maxx, text_width);
  cy += margin;

  if (header_label != NULL && header_width < text_width) {
    // Make the header as wide as the text to center it.
    SetWindowPos(header_label, 0, 0, 0, text_width, header_height,
                 SWP_NOZORDER | SWP_NOMOVE);
  }

  // Now make a button.
  int button_width, button_height;
  HWND cancel_button = make_button(button_width, button_height, "Cancel");
  SetWindowPos(cancel_button, 0, margin, cy, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
  cy += button_height;
  maxx = max(maxx, button_width);

  {
    HDC dc = GetDC(cancel_button);
    SelectObject(dc, font);
    ReleaseDC(cancel_button, dc);
  }

  // Make the window an appropriate size for all its items.
  cy += margin;
  int cx = maxx + margin * 2;

  // Compensate for the window title and border.
  RECT win_rect = { 0, 0, cx, cy };
  AdjustWindowRect(&win_rect, window_style, FALSE);

  SetWindowPos(_hwnd, HWND_TOP, 0, 0, 
               win_rect.right - win_rect.left, win_rect.bottom - win_rect.top, 
               SWP_NOMOVE);

  ShowWindow(_hwnd, SW_SHOWNORMAL);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinAuthDialog::make_button
//       Access: Private
//  Description: Creates a standard button object with the indicated
//               label.
////////////////////////////////////////////////////////////////////
HWND P3DWinAuthDialog::
make_button(int &width, int &height, const string &label) {
  HINSTANCE application = GetModuleHandle(NULL);

  DWORD window_style = 
    BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

  width = 64;
  height = 24;

  HWND button = CreateWindowEx(0, "BUTTON", label.c_str(), window_style,
                               0, 0, width, height,
                               _hwnd, (HMENU)1, application, 0);
  return button;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinAuthDialog::make_static_text
//       Access: Private
//  Description: Creates a static text object for displaying the
//               indicated text block with the given font.
////////////////////////////////////////////////////////////////////
HWND P3DWinAuthDialog::
make_static_text(int &width, int &height, HFONT font, 
                 const string &text) {
  HINSTANCE application = GetModuleHandle(NULL);

  DWORD window_style = 
    SS_OWNERDRAW | WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

  measure_text(width, height, font, text);
  HWND label = CreateWindowEx(0, "STATIC", text.c_str(), window_style,
                              0, 0, width, height,
                              _hwnd, NULL, application, 0);
  HDC dc = GetDC(label);
  SelectObject(dc, font);
  ReleaseDC(label, dc);
  
  return label;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinAuthDialog::measure_text
//       Access: Private
//  Description: Determines the height and width of the indicated
//               string of text, using the indicated font.
////////////////////////////////////////////////////////////////////
void P3DWinAuthDialog::
measure_text(int &width, int &height, HFONT font, const string &text) {
  HDC dc = GetDC(_hwnd);
  SelectObject(dc, font);

  width = 0;
  height = 0;
  SIZE text_size;

  // GetTextExtentPoint() only works for one line at a time.
  const char *text_data = text.data();
  size_t start = 0;
  size_t newline = text.find('\n');
  while (newline != string::npos) {
    string line(text_data + start, newline - start);
    if (line.empty()) {
      line = " ";
    }
    GetTextExtentPoint32(dc, line.data(), line.size(), &text_size);
    width = max(width, text_size.cx); 
    height += text_size.cy;

    start = newline + 1;
    newline = text.find('\n', start);
  }

  GetTextExtentPoint32(dc, text_data + start, text.size() - start, &text_size);
  width = max(width, text_size.cx); 
  height += text_size.cy;

  ReleaseDC(_hwnd, dc);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinAuthDialog::window_proc
//       Access: Private
//  Description: The windows event-processing handler.
////////////////////////////////////////////////////////////////////
LONG P3DWinAuthDialog::
window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
  case WM_COMMAND:
    cerr << "WM_COMMAND " << wparam << " " << lparam << "\n";
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
               DT_CENTER);
    }
    return true;
  };

  return DefWindowProc(hwnd, msg, wparam, lparam);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinAuthDialog::st_window_proc
//       Access: Private, Static
//  Description: The windows event-processing handler, static version.
////////////////////////////////////////////////////////////////////
LONG P3DWinAuthDialog::
st_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  LONG_PTR self = GetWindowLongPtr(hwnd, GWLP_USERDATA);
  if (self == NULL) {
    // We haven't assigned the pointer yet.
    return DefWindowProc(hwnd, msg, wparam, lparam);
  }

  return ((P3DWinAuthDialog *)self)->window_proc(hwnd, msg, wparam, lparam);
}

#endif  // _WIN32
