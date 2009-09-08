// Filename: p3dSplashWindow.h
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


#ifndef P3DSPLASHWINDOW_H
#define P3DSPLASHWINDOW_H

#include "p3d_plugin_common.h"
#include "p3dFileParams.h"
#include "p3dWindowParams.h"

class P3DInstance;

////////////////////////////////////////////////////////////////////
//       Class : P3DSplashWindow
// Description : This window is displayed temporarily, in place of an
//               instance's actual window, during the initial launch
//               of an applet, and also during the initial download of
//               Panda3D code if necessary.
//
//               This is the base implementation; it contains no
//               specific code to open a window.
////////////////////////////////////////////////////////////////////
class P3DSplashWindow {
public:
  P3DSplashWindow(P3DInstance *inst);
  virtual ~P3DSplashWindow();

  inline const P3DFileParams &get_fparams() const;

  virtual void set_wparams(const P3DWindowParams &wparams);
  inline const P3DWindowParams &get_wparams() const;

  enum ImagePlacement {
    IP_background,
    IP_button_ready,
    IP_button_rollover,
    IP_button_click,
    IP_none
  };

  virtual void set_image_filename(const string &image_filename,
                                  ImagePlacement image_placement);
  virtual void set_install_label(const string &install_label);
  virtual void set_install_progress(double install_progress);

  virtual bool handle_event(P3D_event_data event);

  void set_button_active(bool flag);

protected:
  // This ImageData base class provides minimal functionality for
  // storing a loaded image.  Most of the real meat of this class is
  // provided by the various subclasses.
  class ImageData {
  public:
    inline ImageData();
    int _width, _height, _num_channels;
  };

  bool read_image_data(ImageData &image, string &data,
                       const string &image_filename);
  bool read_image_data_jpeg(ImageData &image, string &data,
                            FILE *fp, const string &image_filename);
  bool read_image_data_png(ImageData &image, string &data,
                           FILE *fp, const string &image_filename);
  void get_bar_placement(int &bar_x, int &bar_y,
                         int &bar_width, int &bar_height);
  void set_button_range(const ImageData &image);
  void set_mouse_data(int mouse_x, int mouse_y, bool mouse_down);

  virtual void button_click_detected();
  virtual void refresh();

protected:
  P3DInstance *_inst;
  P3DFileParams _fparams;
  P3DWindowParams _wparams;
  int _win_width, _win_height;

  // The region of the window for accepting button clicks.
  int _button_width, _button_height;
  int _button_x, _button_y;
  bool _button_active;

  // Tracking the mouse pointer within the window, for the purposes of
  // clicking the button.
  int _mouse_x, _mouse_y;
  bool _mouse_down;
  bool _button_depressed;

  // The current visual state of the button.
  enum ButtonState {
    BS_hidden,
    BS_ready,
    BS_rollover,
    BS_click,
  };
  ButtonState _bstate;
};

#include "p3dSplashWindow.I"

#endif
