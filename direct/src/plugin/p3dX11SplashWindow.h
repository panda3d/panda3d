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
#include "plugin_get_x11.h"

#include <vector>

////////////////////////////////////////////////////////////////////
//       Class : P3DX11SplashWindow
// Description : This is the Windows implementation of the
//               initial-download window.
////////////////////////////////////////////////////////////////////
class P3DX11SplashWindow : public P3DSplashWindow {
public:
  P3DX11SplashWindow(P3DInstance *inst, bool make_visible);
  virtual ~P3DX11SplashWindow();

  virtual void set_wparams(const P3DWindowParams &wparams);
  virtual void set_visible(bool visible);
  virtual void set_image_filename(const string &image_filename,
                                  ImagePlacement image_placement);
  virtual void set_install_label(const string &install_label);
  virtual void set_install_progress(double install_progress,
                                    bool is_progress_known, size_t received_data);

  virtual void set_button_active(bool flag);

protected:
  virtual void button_click_detected();
  virtual void set_bstate(ButtonState bstate);

private:
  void start_subprocess();
  void stop_subprocess();
  void check_stopped();

  void spawn_read_thread();
  void join_read_thread();

private:
  // These methods run only within the read thread.
  THREAD_CALLBACK_DECLARATION(P3DX11SplashWindow, rt_thread_run);
  void rt_thread_run();
  void rt_terminate();
  void rt_handle_request(TiXmlDocument *doc);

private:
  // Data members that are stored in the parent process.
  pid_t _subprocess_pid;
  HandleStream _pipe_read;
  HandleStream _pipe_write;

  // The remaining members are manipulated by or for the read thread.
  bool _started_read_thread;
  THREAD _read_thread;

private:
  // These methods run only within the subprocess.
  class X11ImageData;

  void subprocess_run();
  void receive_command();

  void redraw();
  void make_window();
  void setup_gc();
  void close_window();

  void update_image(X11ImageData &image); 
  void compose_image();
  bool scale_image(vector<unsigned char> &image0, int &image0_width, int &image0_height,
                   X11ImageData &image);

  void compose_two_images(vector<unsigned char> &image0, int &image0_width, int &image0_height,
                          const vector<unsigned char> &image1, int image1_width, int image1_height,
                          const vector<unsigned char> &image2, int image2_width, int image2_height);

private:
  // Data members that are stored in the subprocess.
  class X11ImageData : public ImageData {
  public:
    inline X11ImageData();
    inline ~X11ImageData();

    string _filename;
    bool _filename_changed;
    string _data;
  };

  X11ImageData _background_image;
  X11ImageData _button_ready_image;
  X11ImageData _button_rollover_image;
  X11ImageData _button_click_image;

  XImage *_composite_image;
  int _composite_width, _composite_height;
  bool _needs_new_composite;

  bool _subprocess_continue;

  bool _own_display;
  string _install_label;
  double _install_progress;
  bool _progress_known;
  size_t _received_data;
  
  string _label_text;

  X11_Display *_display;
  int _screen;
  GC _graphics_context;
  GC _bar_context;
  unsigned long _fg_pixel;
  unsigned long _bg_pixel;
  unsigned long _bar_pixel;
  
  X11_Window _window;
};

#include "p3dX11SplashWindow.I"

#endif  // HAVE_X11

#endif
