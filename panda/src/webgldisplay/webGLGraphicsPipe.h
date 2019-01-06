/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file webGLGraphicsPipe.h
 * @author rdb
 * @date 2015-04-01
 */

#ifndef WEBGLGRAPHICSPIPE_H
#define WEBGLGRAPHICSPIPE_H

#include "pandabase.h"
#include "graphicsWindow.h"
#include "graphicsPipe.h"

class FrameBufferProperties;

class WebGLGraphicsWindow;

/**
 * This graphics pipe represents the interface for creating OpenGL ES graphics
 * windows on an X-based (e.g.  Unix) client.
 */
class WebGLGraphicsPipe : public GraphicsPipe {
public:
  WebGLGraphicsPipe();
  virtual ~WebGLGraphicsPipe();

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

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsPipe::init_type();
    register_type(_type_handle, "WebGLGraphicsPipe",
                  GraphicsPipe::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "webGLGraphicsPipe.I"

#endif
