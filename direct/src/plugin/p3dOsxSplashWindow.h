// Filename: p3dOsxSplashWindow.h
// Created by:  drose (16Jul09)
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

#ifndef P3DOSXSPLASHWINDOW_H
#define P3DOSXSPLASHWINDOW_H

#include "p3d_plugin_common.h"

#ifdef __APPLE__

#include "p3dSplashWindow.h"

#include <ApplicationServices/ApplicationServices.h>

////////////////////////////////////////////////////////////////////
//       Class : P3DOsxSplashWindow
// Description : This is the OSX implementation of the
//               initial-download window.
////////////////////////////////////////////////////////////////////
class P3DOsxSplashWindow : public P3DSplashWindow {
public:
  P3DOsxSplashWindow(P3DInstance *inst);
  virtual ~P3DOsxSplashWindow();

  virtual void set_image_filename(const string &image_filename,
                                  bool image_filename_temp);
  virtual void set_install_label(const string &install_label);
  virtual void set_install_progress(double install_progress);

  virtual void handle_event(P3D_event_data event);

private:
  void paint_window();

private:
  GWorldPtr _image;
  char *_image_data;
  int _image_height, _image_width;

  string _install_label;
  double _install_progress;
};

#include "p3dOsxSplashWindow.I"

#endif  // __APPLE__

#endif
