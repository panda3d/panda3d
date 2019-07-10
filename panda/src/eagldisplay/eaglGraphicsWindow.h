/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file EAGLGraphicsWindow.h
 * @author D. Lawrence
 * @date 2019-01-03
 */

#ifndef EAGLGRAPHICSWINDOW_H
#define EAGLGRAPHICSWINDOW_H

#import "pandaEAGLView.h"
#include "pandabase.h"
#include "eaglGraphicsPipe.h"

class EAGLGraphicsStateGuardian;

/**
 * Interface to a view containing an EAGLContext. Because there are no true
 * "windows" in iOS or tvOS, a given GraphicsWindow will instead correspond to
 * a view inside a single app, fullscreen or otherwise.
 */
class EAGLGraphicsWindow : public GraphicsWindow {
public:
  EAGLGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                     const std::string &name,
                     const FrameBufferProperties &fb_prop,
                     const WindowProperties &win_prop,
                     int flags,
                     GraphicsStateGuardian *gsg,
                     GraphicsOutput *host);
  ~EAGLGraphicsWindow();
  
  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);
  
  virtual void end_flip();
  
  void screen_size_changed();
  void app_activated();
  void app_deactivated();

private:
  void create_framebuffer(EAGLGraphicsStateGuardian *guardian);
  void destroy_framebuffer(EAGLGraphicsStateGuardian *guardian);
  
//  virtual void set_properties_now(WindowProperties &properties);
  
protected:
  virtual void close_window();
  virtual bool open_window();

  PandaEAGLView *_view;
  
  GLuint _fbo;
  GLuint _color_rb;
  GLuint _depth_stencil_rb;

  PT(GraphicsWindowInputDevice) _emulated_mouse_input;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsWindow::init_type();
    register_type(_type_handle, "EAGLGraphicsWindow",
                  GraphicsWindow::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  
private:
  static TypeHandle _type_handle;
};

#include "EAGLGraphicsWindow.I"

#endif
