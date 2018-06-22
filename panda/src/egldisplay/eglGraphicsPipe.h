/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eglGraphicsPipe.h
 * @author rdb
 * @date 2009-05-21
 */

#ifndef EGLGRAPHICSPIPE_H
#define EGLGRAPHICSPIPE_H

#include "pandabase.h"
#include "x11GraphicsPipe.h"

#ifdef OPENGLES_2
  #include "gles2gsg.h"
  #include "pre_x11_include.h"
  #include <EGL/egl.h>
  #include "post_x11_include.h"
  #define NativeDisplayType EGLNativeDisplayType
  #define NativePixmapType EGLNativePixmapType
  #define NativeWindowType EGLNativeWindowType
#else
  #include "glesgsg.h"
  #include "pre_x11_include.h"
  #include <GLES/egl.h>
  #include "post_x11_include.h"
#endif

class FrameBufferProperties;

class eglGraphicsBuffer;
class eglGraphicsPixmap;
class eglGraphicsWindow;

/**
 * This graphics pipe represents the interface for creating OpenGL ES graphics
 * windows on an X-based (e.g.  Unix) client.
 */
class eglGraphicsPipe : public x11GraphicsPipe {
public:
  eglGraphicsPipe(const std::string &display = std::string());
  virtual ~eglGraphicsPipe();

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

private:
  EGLDisplay _egl_display;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    x11GraphicsPipe::init_type();
    register_type(_type_handle, "eglGraphicsPipe",
                  x11GraphicsPipe::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class eglGraphicsBuffer;
  friend class eglGraphicsPixmap;
  friend class eglGraphicsWindow;
};

#include "eglGraphicsPipe.I"

#endif
