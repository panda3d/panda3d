/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eglGraphicsPixmap.h
 * @author rdb
 * @date 2009-06-13
 */

#ifndef EGLGRAPHICSPIXMAP_H
#define EGLGRAPHICSPIXMAP_H

#include "pandabase.h"

#include "eglGraphicsPipe.h"
#include "graphicsBuffer.h"

/**
 * Another offscreen buffer in the EGL environment.  This creates a Pixmap
 * object, which is probably less efficient than an EGLPBuffer, so this class
 * is a second choice to eglGraphicsBuffer.
 */
class eglGraphicsPixmap : public GraphicsBuffer {
public:
  eglGraphicsPixmap(GraphicsEngine *engine, GraphicsPipe *pipe,
                    const std::string &name,
                    const FrameBufferProperties &fb_prop,
                    const WindowProperties &win_prop,
                    int flags,
                    GraphicsStateGuardian *gsg,
                    GraphicsOutput *host);
  virtual ~eglGraphicsPixmap();

  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);

protected:
  virtual void close_buffer();
  virtual bool open_buffer();

private:
  X11_Display *_display;
  X11_Window _drawable;
  Pixmap _x_pixmap;
  EGLSurface _egl_surface;
  EGLDisplay _egl_display;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsBuffer::init_type();
    register_type(_type_handle, "eglGraphicsPixmap",
                  GraphicsBuffer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif
