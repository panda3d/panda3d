/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cocoaGraphicsPipe.h
 * @author rdb
 * @date 2012-05-14
 */

#ifndef COCOAGRAPHICSPIPE_H
#define COCOAGRAPHICSPIPE_H

#include "pandabase.h"
#include "graphicsWindow.h"
#include "graphicsPipe.h"
#include "lightMutex.h"
#include "lightReMutex.h"

#ifdef __OBJC__
#import <AppKit/NSScreen.h>
#else
struct NSScreen;
#endif
#include <ApplicationServices/ApplicationServices.h>

class FrameBufferProperties;

/**
 * This graphics pipe represents the interface for creating OpenGL graphics
 * windows on a Cocoa-based (e.g.  Mac OS X) client.
 */
class EXPCL_PANDA_COCOADISPLAY CocoaGraphicsPipe : public GraphicsPipe {
public:
  CocoaGraphicsPipe(CGDirectDisplayID display = CGMainDisplayID());
  virtual ~CocoaGraphicsPipe();

  INLINE CGDirectDisplayID get_display_id() const;

  virtual std::string get_interface_name() const;
  static PT(GraphicsPipe) pipe_constructor();

public:
  virtual PreferredWindowThread get_preferred_window_thread() const;

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
  virtual PT(GraphicsStateGuardian) make_callback_gsg(GraphicsEngine *engine);

private:
  void load_display_information();

  // This is the Quartz display identifier.
  CGDirectDisplayID _display;

  friend class CocoaGraphicsWindow;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsPipe::init_type();
    register_type(_type_handle, "CocoaGraphicsPipe",
                  GraphicsPipe::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "cocoaGraphicsPipe.I"

#endif
