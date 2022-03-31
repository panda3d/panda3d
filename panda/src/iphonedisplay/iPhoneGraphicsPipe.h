/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file iPhoneGraphicsPipe.h
 * @author drose
 * @date 2009-04-08
 */

#ifndef IPHONEGRAPHICSPIPE_H
#define IPHONEGRAPHICSPIPE_H

#include "pandabase.h"
#include "graphicsPipe.h"
#include "pset.h"
#import "viewController.h"
#import <UIKit/UIKit.h>

class IPhoneGraphicsStateGuardian;
class IPhoneGraphicsWindow;
class PNMImage;

/**
 * This graphics pipe represents the interface for creating OpenGL graphics
 * windows on the various IPHONE's.
 */
class EXPCL_MISC IPhoneGraphicsPipe : public GraphicsPipe {
public:
  IPhoneGraphicsPipe();
  virtual ~IPhoneGraphicsPipe();

  virtual std::string get_interface_name() const;
  static PT(GraphicsPipe) pipe_constructor();
  virtual PreferredWindowThread get_preferred_window_thread() const;

  void rotate_windows();

protected:
  virtual PT(GraphicsOutput) make_output(const std::string &name,
                                         const FrameBufferProperties &fb_prop,
                                         const WindowProperties &win_prop,
                                         int flags,
                                         GraphicsEngine *engine,
                                         GraphicsStateGuardian *gsg,
                                         GraphicsOutput *host,
                                         int retry,
                                         bool &precertify);

public:
  UIWindow *_window;
  ControllerDemoViewController *_view_controller;

  typedef pset<IPhoneGraphicsWindow *> GraphicsWindows;
  GraphicsWindows _graphics_windows;

private:
  static IPhoneGraphicsPipe *_global_ptr;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsPipe::init_type();
    register_type(_type_handle, "IPhoneGraphicsPipe",
                  GraphicsPipe::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class IPhoneGraphicsBuffer;
};

#endif
