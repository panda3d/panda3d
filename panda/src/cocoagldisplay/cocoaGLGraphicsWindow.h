/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cocoaGLGraphicsWindow.h
 * @author rdb
 * @date 2023-03-18
 */

#ifndef COCOAGLGRAPHICSWINDOW_H
#define COCOAGLGRAPHICSWINDOW_H

#include "cocoaGraphicsWindow.h"

/**
 *
 */
class EXPCL_PANDA_COCOADISPLAY CocoaGLGraphicsWindow : public CocoaGraphicsWindow {
public:
  CocoaGLGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                        const std::string &name,
                        const FrameBufferProperties &fb_prop,
                        const WindowProperties &win_prop,
                        int flags,
                        GraphicsStateGuardian *gsg,
                        GraphicsOutput *host);
  virtual ~CocoaGLGraphicsWindow();

  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);
  virtual void end_flip();

  virtual void update_context();
  virtual void unbind_context();

protected:
  virtual bool open_window();

private:
  bool _vsync_enabled = false;
  uint32_t _vsync_counter = 0;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CocoaGraphicsWindow::init_type();
    register_type(_type_handle, "CocoaGLGraphicsWindow",
                  CocoaGraphicsWindow::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "cocoaGLGraphicsWindow.I"

#endif
