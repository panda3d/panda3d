// Filename: p3dX11SplashWindow.h
// Created by:  pro-rsoft (08Jul09)
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

#ifndef P3DX11SPLASHWINDOW_H
#define P3DX11SPLASHWINDOW_H

#include "p3d_plugin_common.h"

#ifdef HAVE_X11

#include "p3dSplashWindow.h"
#include "p3d_lock.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

////////////////////////////////////////////////////////////////////
//       Class : P3DX11SplashWindow
// Description : This is the Windows implementation of the
//               initial-download window.
////////////////////////////////////////////////////////////////////
class P3DX11SplashWindow : public P3DSplashWindow {
public:
  P3DX11SplashWindow(P3DInstance *inst);
  virtual ~P3DX11SplashWindow();

  virtual void set_image_filename(const string &image_filename,
                                  bool image_filename_temp);
  virtual void set_install_label(const string &install_label);
  virtual void set_install_progress(double install_progress);

private:
  void start_thread();
  void stop_thread();

private:
  // These methods run only within the window thread.
  void thread_run();
  THREAD_CALLBACK_DECLARATION(P3DX11SplashWindow, thread_run);

  void redraw(string label);
  void make_window();
  void setup_gc();
  void update_image_filename(const string &image_filename, 
                             bool image_filename_temp);
  void close_window();

private:
  int _width, _height;
  
  bool _own_display;
  bool _got_install;
  bool _image_filename_changed;
  string _image_filename;
  bool _image_filename_temp;
  bool _install_label_changed;
  string _install_label;
  double _install_progress;
  LOCK _install_lock;
  
  string _label_text;

  bool _thread_continue;
  bool _thread_running;
  Display *_display;
  int _screen;
  GC _graphics_context;
  XImage* _image;
  XImage* _resized_image;
  int _image_width, _image_height;
  int _resized_width, _resized_height;
  
  THREAD _thread;
  Window _window;
};

#endif  // HAVE_X11

#endif
