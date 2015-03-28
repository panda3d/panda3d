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

#if defined(__APPLE__) && !__LP64__

#include "p3dSplashWindow.h"

#include <ApplicationServices/ApplicationServices.h>

////////////////////////////////////////////////////////////////////
//       Class : P3DOsxSplashWindow
// Description : This is the OSX implementation of the
//               initial-download window.
////////////////////////////////////////////////////////////////////
class P3DOsxSplashWindow : public P3DSplashWindow {
public:
  P3DOsxSplashWindow(P3DInstance *inst, bool make_visible);
  virtual ~P3DOsxSplashWindow();

  virtual void set_wparams(const P3DWindowParams &wparams);
  virtual void set_visible(bool visible);
  virtual void set_image_filename(const string &image_filename,
                                  ImagePlacement image_placement);
  virtual void set_install_label(const string &install_label);
  virtual void set_install_progress(double install_progress,
                                    bool is_progress_known, size_t received_data);

  virtual bool handle_event(const P3D_event_data &event);

protected:
  virtual void refresh();

private:
  void paint_window();
  void paint_window_osx_cgcontext(CGContextRef context);
  bool handle_event_osx_event_record(const P3D_event_data &event);
  bool handle_event_osx_cocoa(const P3D_event_data &event);
  class OsxImageData;

  void load_image(OsxImageData &image, const string &image_filename);
  bool paint_image(CGContextRef context, const OsxImageData &image);
  void paint_progress_bar(CGContextRef context);

  static pascal OSStatus
  st_event_callback(EventHandlerCallRef my_handler, EventRef event, 
                    void *user_data);
  OSStatus event_callback(EventHandlerCallRef my_handler, EventRef event);

private:
  bool _got_wparams;

  class OsxImageData : public ImageData {
  public:
    inline OsxImageData();
    inline ~OsxImageData();
    void dump_image();

    char *_raw_data;
    CFDataRef _data;
    CGDataProviderRef _provider;
    CGColorSpaceRef _color_space;
    CGImageRef _image;
  };

  OsxImageData _background_image;
  OsxImageData _button_ready_image;
  OsxImageData _button_rollover_image;
  OsxImageData _button_click_image;

  string _install_label;
  double _install_progress;
  bool _progress_known;
  size_t _received_data;

  // Used to track the mouse within the window in the embedded case.
  bool _mouse_active;

  // Filled only in the non-embedded case.
  WindowRef _toplevel_window;
};

#include "p3dOsxSplashWindow.I"

#endif  // __APPLE__

#endif
