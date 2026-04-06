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

#ifdef USE_X11
#include "x11GraphicsPipe.h"
typedef x11GraphicsPipe BaseGraphicsPipe;
#else
#include "graphicsPipe.h"
typedef GraphicsPipe BaseGraphicsPipe;
#undef EGL_NO_X11
#define EGL_NO_X11 1
#endif

#ifdef OPENGLES_2
  #include "gles2gsg.h"
  #include "pre_x11_include.h"
  #include <EGL/egl.h>
  #include "post_x11_include.h"
  #define NativeDisplayType EGLNativeDisplayType
  #define NativePixmapType EGLNativePixmapType
  #define NativeWindowType EGLNativeWindowType
#elif defined(OPENGLES_1)
  #include "glesgsg.h"
  #include "pre_x11_include.h"
  #include <GLES/egl.h>
  #include "post_x11_include.h"
#else
  #include "glgsg.h"
  #include "pre_x11_include.h"
  #include <EGL/egl.h>
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
class eglGraphicsPipe : public BaseGraphicsPipe {
public:
  eglGraphicsPipe();
  virtual ~eglGraphicsPipe();

  virtual std::string get_interface_name() const;
  static PT(GraphicsPipe) pipe_constructor();

  INLINE EGLDisplay get_egl_display() const;

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
  EGLDisplay _egl_display = 0;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BaseGraphicsPipe::init_type();
    register_type(_type_handle, "eglGraphicsPipe",
                  BaseGraphicsPipe::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "eglGraphicsPipe.I"

#endif
