/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eglGraphicsBuffer.h
 * @author rdb
 * @date 2009-06-13
 */

#ifndef EGLGRAPHICSBUFFER_H
#define EGLGRAPHICSBUFFER_H

#include "pandabase.h"

#include "eglGraphicsPipe.h"
#include "graphicsBuffer.h"

/**
 * An offscreen buffer in the EGL environment.  This creates an EGL pbuffer.
 */
class eglGraphicsBuffer : public GraphicsBuffer {
public:
  eglGraphicsBuffer(GraphicsEngine *engine, GraphicsPipe *pipe,
                    const std::string &name,
                    const FrameBufferProperties &fb_prop,
                    const WindowProperties &win_prop,
                    int flags,
                    GraphicsStateGuardian *gsg,
                    GraphicsOutput *host);
  virtual ~eglGraphicsBuffer();

  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);

protected:
  virtual void close_buffer();
  virtual bool open_buffer();

private:
  EGLSurface _pbuffer;
  EGLDisplay _egl_display;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsBuffer::init_type();
    register_type(_type_handle, "eglGraphicsBuffer",
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
