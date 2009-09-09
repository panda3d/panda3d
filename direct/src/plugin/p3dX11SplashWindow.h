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
#include "handleStream.h"

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

  virtual void set_wparams(const P3DWindowParams &wparams);
  virtual void set_image_filename(const string &image_filename,
                                  ImagePlacement image_placement);
  virtual void set_install_label(const string &install_label);
  virtual void set_install_progress(double install_progress);

  virtual void set_button_active(bool flag);

protected:
  virtual void button_click_detected();

private:
  void start_subprocess();
  void stop_subprocess();
  void check_stopped();

private:
  // Data members that are stored in the parent process.
  pid_t _subprocess_pid;
  HandleStream _pipe_write;

private:
  // These methods run only within the subprocess.
  class X11ImageData;

  void subprocess_run();
  void receive_command();

  void redraw();
  bool paint_image(X11ImageData &image);
  void make_window();
  void setup_gc();
  void update_image(X11ImageData &image, bool &needs_redraw); 
  void close_window();

private:
  // Data members that are stored in the subprocess.
  class X11ImageData : public ImageData {
  public:
    inline X11ImageData();
    inline ~X11ImageData();
    void dump_image();
    void dump_resized_image();

    string _filename;
    bool _filename_changed;
    XImage *_image;

    XImage *_resized_image;
    int _resized_width, _resized_height;
  };

  X11ImageData _background_image;
  X11ImageData _button_ready_image;
  X11ImageData _button_rollover_image;
  X11ImageData _button_click_image;

  bool _subprocess_continue;
  HandleStream _pipe_read;
  
  bool _own_display;
  string _install_label;
  double _install_progress;
  
  string _label_text;

  Display *_display;
  int _screen;
  GC _graphics_context;
  GC _bar_context;
  unsigned long _blue_pixel;
  
  Window _window;
};

#include "p3dX11SplashWindow.I"

#endif  // HAVE_X11

#endif
