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
  P3DWinSplashWindow(P3DInstance *inst, bool make_visible);
  virtual ~P3DWinSplashWindow();

  virtual void set_wparams(const P3DWindowParams &wparams);
  virtual void set_visible(bool visible);

  virtual void set_image_filename(const string &image_filename,
                                  ImagePlacement image_placement);
  virtual void set_install_label(const string &install_label);
  virtual void set_install_progress(double install_progress,
                                    bool is_progress_known, size_t received_data);
  virtual void request_keyboard_focus();

  static void register_window_class();
  static void unregister_window_class();

protected:
  virtual void button_click_detected();

private:
  void start_thread();
  void stop_thread();

private:
  class WinImageData;

  // These methods run only within the window thread.
  void thread_run();
  static DWORD WINAPI win_thread_run(LPVOID data);

  void make_window();
  void update_image(WinImageData &image);
  void close_window();

  void paint_window(HDC dc);
  bool paint_image(HDC dc, const WinImageData &image, bool use_alpha);
  void paint_progress_bar(HDC dc);
  LONG window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  static LONG WINAPI st_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
  class WinImageData : public ImageData {
  public:
    inline WinImageData();
    inline ~WinImageData();
    void dump_image();

    string _filename;
    bool _filename_changed;
    HBITMAP _bitmap;
  };

  WinImageData _background_image;
  WinImageData _button_ready_image;
  WinImageData _button_rollover_image;
  WinImageData _button_click_image;

  bool _got_install;
  string _install_label;
  double _install_progress;
  bool _progress_known;
  size_t _received_data;
  LOCK _install_lock;

  ButtonState _drawn_bstate;
  string _drawn_label;
  double _drawn_progress;
  bool _drawn_progress_known;
  size_t _drawn_received_data;
  int _focus_seq;

  int _request_focus_tick;

  bool _thread_continue;
  bool _thread_running;
  HANDLE _thread;
  DWORD _thread_id;
  HWND _hwnd;
  HBRUSH _fg_brush;
  HBRUSH _bg_brush;
  HBRUSH _bar_brush;

  static bool _registered_window_class;
};

#include "p3dWinSplashWindow.I"

#endif  // _WIN32

#endif
