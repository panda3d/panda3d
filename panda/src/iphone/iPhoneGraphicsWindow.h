// Filename: iPhoneGraphicsWindow.h
// Created by:  drose (08Apr09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef IPHONEGRAPHICSWINDOW_H
#define IPHONEGRAPHICSWINDOW_H

#include "pandabase.h"
#include "graphicsWindow.h"
#include "buttonHandle.h"
#include "glesgsg.h"
#import "eaglView.h"

////////////////////////////////////////////////////////////////////
//       Class : IPhoneGraphicsWindow
// Description : An interface to the osx/ system for managing GL
//               windows under X.
////////////////////////////////////////////////////////////////////
class IPhoneGraphicsWindow : public GraphicsWindow {
public:
  IPhoneGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe, 
                       const string &name,
                       const FrameBufferProperties &fb_prop,
                       const WindowProperties &win_prop,
                       int flags,
                       GraphicsStateGuardian *gsg,
                       GraphicsOutput *host);
  virtual ~IPhoneGraphicsWindow();

  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);
  virtual void begin_flip();
  virtual void end_flip();
  virtual void process_events();

  virtual void set_properties_now(WindowProperties &properties);
  virtual void clear_pipe();

  void rotate_window();

protected:
  virtual void close_window();
  virtual bool open_window();

private:
  void system_close_window();
  void system_set_window_foreground(bool foreground);	

  void set_pointer_in_window(int x, int y);
  void set_pointer_out_of_window();

private:
  EAGLView *_gl_view; 

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsWindow::init_type();
    register_type(_type_handle, "IPhoneGraphicsWindow",
                  GraphicsWindow::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "iPhoneGraphicsWindow.I"

#endif
