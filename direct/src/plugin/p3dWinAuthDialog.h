// Filename: p3dWinAuthDialog.h
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

#ifndef P3DWINAUTHDIALOG_H
#define P3DWINAUTHDIALOG_H

#include "p3dAuthDialog.h"

#ifdef _WIN32

////////////////////////////////////////////////////////////////////
//       Class : P3DWinAuthDialog
// Description : A specialization on P3DAuthDialog for windows.
////////////////////////////////////////////////////////////////////
class P3DWinAuthDialog : public P3DAuthDialog {
public:
  P3DWinAuthDialog();

  virtual void open();
  virtual void close();

  static void register_window_class();
  static void unregister_window_class();

private:
  void make_window();

  HWND make_button(int &width, int &height, const string &label);
  HWND make_static_text(int &width, int &height, HFONT font, 
                        const string &text);
  void measure_text(int &width, int &height, HFONT font, const string &text);

  LONG window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  static LONG WINAPI st_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
  HWND _hwnd;

  static bool _registered_window_class;
};

#include "p3dAuthDialog.I"

#endif  // _WIN32

#endif
