/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eglGraphicsWindow.h
 * @author rdb
 * @date 2009-05-21
 */

#ifndef EGLGRAPHICSWINDOW_H
#define EGLGRAPHICSWINDOW_H

#include "pandabase.h"

#include "eglGraphicsPipe.h"
#include "x11GraphicsWindow.h"

/**
 * An interface to the egl system for managing GLES windows under X.
 */
class eglGraphicsWindow : public x11GraphicsWindow {
public:
  eglGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                    const std::string &name,
                    const FrameBufferProperties &fb_prop,
                    const WindowProperties &win_prop,
                    int flags,
                    GraphicsStateGuardian *gsg,
                    GraphicsOutput *host);
  virtual ~eglGraphicsWindow();

  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);
  virtual void end_flip();

protected:
  virtual void close_window();
  virtual bool open_window();

private:
  EGLDisplay _egl_display;
  EGLSurface _egl_surface;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    x11GraphicsWindow::init_type();
    register_type(_type_handle, "eglGraphicsWindow",
                  x11GraphicsWindow::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "eglGraphicsWindow.I"

#endif
