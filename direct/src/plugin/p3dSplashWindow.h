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

  virtual void set_image_filename(const string &image_filename,
                                  bool image_filename_temp);
  virtual void set_install_label(const string &install_label);
  virtual void set_install_progress(double install_progress);

  virtual bool handle_event(P3D_event_data event);

  void setup_splash_image();

protected:
  bool read_image(const string &image_filename, bool image_filename_temp,
                  int &height, int &width, int &num_channels, string &data);
  void get_bar_placement(int win_width, int win_height,
                         int &bar_x, int &bar_y,
                         int &bar_width, int &bar_height);

protected:
  P3DInstance *_inst;
  P3DFileParams _fparams;
  P3DWindowParams _wparams;
};

#include "p3dSplashWindow.I"

#endif
