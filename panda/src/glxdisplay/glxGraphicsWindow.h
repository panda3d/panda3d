/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glxGraphicsWindow.h
 * @author mike
 * @date 1997-01-09
 */

#ifndef GLXGRAPHICSWINDOW_H
#define GLXGRAPHICSWINDOW_H

#include "pandabase.h"

#include "x11GraphicsWindow.h"
#include "glxGraphicsPipe.h"
#include "graphicsWindow.h"
#include "buttonHandle.h"

/**
 * An interface to the glx system for managing GL windows under X.
 */
class glxGraphicsWindow : public x11GraphicsWindow {
public:
  glxGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                    const std::string &name,
                    const FrameBufferProperties &fb_prop,
                    const WindowProperties &win_prop,
                    int flags,
                    GraphicsStateGuardian *gsg,
                    GraphicsOutput *host);
  virtual ~glxGraphicsWindow() {};

  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);
  virtual void end_flip();

protected:
  virtual void close_window();
  virtual bool open_window();

private:
  virtual void setup_colormap(GLXFBConfig fbconfig);
  virtual void setup_colormap(XVisualInfo *visual);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    x11GraphicsWindow::init_type();
    register_type(_type_handle, "glxGraphicsWindow",
                  x11GraphicsWindow::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif
