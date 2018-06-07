/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file iPhoneGraphicsWindow.h
 * @author drose
 * @date 2009-04-08
 */

#ifndef IPHONEGRAPHICSWINDOW_H
#define IPHONEGRAPHICSWINDOW_H

#include "pandabase.h"
#include "graphicsWindow.h"
#include "buttonHandle.h"
#include "glesgsg.h"
#import "eaglView.h"

#import <UIKit/UIKit.h>

/**
 * An interface to the osx/ system for managing GL windows under X.
 */
class IPhoneGraphicsWindow : public GraphicsWindow {
public:
  IPhoneGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                       const std::string &name,
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
  void touches_began(NSSet *touches, UIEvent *event);
  void touches_moved(NSSet *touches, UIEvent *event);
  void touches_ended(NSSet *touches, UIEvent *event);
  void touches_cancelled(NSSet *touches, UIEvent *event);

  CGPoint get_average_location(NSSet *touches);


protected:
  virtual void close_window();
  virtual bool open_window();

private:
  void set_pointer_in_window(int x, int y);
  void set_pointer_out_of_window();
  void handle_button_delta(int num_touches);

private:
  EAGLView *_gl_view;
  int _last_buttons;

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
