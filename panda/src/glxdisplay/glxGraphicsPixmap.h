/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glxGraphicsPixmap.h
 * @author drose
 * @date 2009-03-10
 */

#ifndef GLXGRAPHICSPIXMAP_H
#define GLXGRAPHICSPIXMAP_H

#include "pandabase.h"

#include "glxGraphicsPipe.h"
#include "graphicsBuffer.h"

/**
 * Another offscreen buffer in the GLX environment.  This creates a Pixmap
 * object, which is probably less efficient than a GLXPBuffer, so this class
 * is a second choice to glxGraphicsBuffer.  However, this might be the only
 * option for some graphics drivers.
 */
class glxGraphicsPixmap : public GraphicsBuffer {
public:
  glxGraphicsPixmap(GraphicsEngine *engine, GraphicsPipe *pipe,
                    const std::string &name,
                    const FrameBufferProperties &fb_prop,
                    const WindowProperties &win_prop,
                    int flags,
                    GraphicsStateGuardian *gsg,
                    GraphicsOutput *host);
  virtual ~glxGraphicsPixmap();

  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);

protected:
  virtual void close_buffer();
  virtual bool open_buffer();

private:
  X11_Display *_display;
  X11_Window _drawable;
  Pixmap _x_pixmap;
  GLXPixmap _glx_pixmap;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsBuffer::init_type();
    register_type(_type_handle, "glxGraphicsPixmap",
                  GraphicsBuffer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "glxGraphicsPixmap.I"

#endif
