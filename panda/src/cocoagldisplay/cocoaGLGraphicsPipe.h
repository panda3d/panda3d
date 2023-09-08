/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cocoaGLGraphicsPipe.h
 * @author rdb
 * @date 2023-03-20
 */

#ifndef COCOAGLGRAPHICSPIPE_H
#define COCOAGLGRAPHICSPIPE_H

#include "cocoaGraphicsPipe.h"

class FrameBufferProperties;

/**
 * This graphics pipe represents the interface for creating OpenGL graphics
 * windows on a Cocoa-based (e.g.  Mac OS X) client.
 */
class EXPCL_PANDA_COCOAGLDISPLAY CocoaGLGraphicsPipe : public CocoaGraphicsPipe {
public:
  CocoaGLGraphicsPipe(CGDirectDisplayID display = CGMainDisplayID());
  virtual ~CocoaGLGraphicsPipe();

  virtual std::string get_interface_name() const;
  static PT(GraphicsPipe) pipe_constructor();

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

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CocoaGraphicsPipe::init_type();
    register_type(_type_handle, "CocoaGLGraphicsPipe",
                  CocoaGraphicsPipe::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "cocoaGLGraphicsPipe.I"

#endif
