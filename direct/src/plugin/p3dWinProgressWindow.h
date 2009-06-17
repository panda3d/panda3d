// Filename: p3dWinProgressWindow.h
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

#ifndef P3DWINPROGRESSWINDOW_H
#define P3DWINPROGRESSWINDOW_H

#include "p3d_plugin_common.h"

#ifdef _WIN32

#include "p3dProgressWindow.h"
#include "p3d_lock.h"

#include <windows.h>

////////////////////////////////////////////////////////////////////
//       Class : P3DWinProgressWindow
// Description : This is the Windows implementation of the
//               initial-download window.
////////////////////////////////////////////////////////////////////
class P3DWinProgressWindow : public P3DProgressWindow {
public:
  P3DWinProgressWindow(P3DPackage *package, P3DSession *session, P3DInstance *inst);
  virtual ~P3DWinProgressWindow();

protected:
  virtual void install_progress(P3DPackage *package, double progress);

private:
  void start_thread();
  void stop_thread();

private:
  // These methods run only within the window thread.
  void thread_run();
  static DWORD WINAPI win_thread_run(LPVOID data);

  void make_window();
  void make_progress_bar();
  void close_window();

  static LONG WINAPI window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
  double _progress;
  LOCK _progress_lock;
  int _loop_started;

  bool _thread_continue;
  bool _thread_running;
  HANDLE _thread;
  DWORD _thread_id;
  HWND _hwnd;
  HWND _progress_bar;
};

#include "p3dWinProgressWindow.I"

#endif  // _WIN32

#endif
