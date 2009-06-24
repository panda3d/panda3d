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

  virtual void set_install_label(const string &install_label);
  virtual void set_install_progress(double install_progress);

private:
  void start_thread();
  void stop_thread();

private:
  // These methods run only within the window thread.
  void thread_run();
  static DWORD WINAPI win_thread_run(LPVOID data);

  void make_window();
  void make_progress_bar();
  void update_install_label(const string &install_label);
  void close_window();

  static LONG WINAPI window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
  bool _got_install;
  bool _install_label_changed;
  string _install_label;
  double _install_progress;
  LOCK _install_lock;

  int _loop_started;

  bool _thread_continue;
  bool _thread_running;
  HANDLE _thread;
  DWORD _thread_id;
  HWND _hwnd;
  HWND _progress_bar;
  HWND _text_label;
};

#include "p3dWinSplashWindow.I"

#endif  // _WIN32

#endif
