/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file androidGraphicsWindow.h
 * @author rdb
 * @date 2013-01-11
 */

#ifndef ANDROIDGRAPHICSWINDOW_H
#define ANDROIDGRAPHICSWINDOW_H

#include "pandabase.h"

#include "androidGraphicsPipe.h"
#include "graphicsWindow.h"
#include "buttonHandle.h"

#include <android/native_window.h>
#include <android/input.h>
#include <android/native_activity.h>
#include <android/rect.h>

struct android_app;

/**
 * An interface to manage Android windows and their appropriate EGL surfaces.
 */
class AndroidGraphicsWindow : public GraphicsWindow {
public:
  AndroidGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                        const std::string &name,
                        const FrameBufferProperties &fb_prop,
                        const WindowProperties &win_prop,
                        int flags,
                        GraphicsStateGuardian *gsg,
                        GraphicsOutput *host);
  virtual ~AndroidGraphicsWindow();

  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);
  virtual void end_flip();

  virtual void process_events();
  virtual void set_properties_now(WindowProperties &properties);

protected:
  virtual void close_window();
  virtual bool open_window();

  virtual void destroy_surface();
  virtual bool create_surface();

private:
  static void handle_command(struct android_app *app, int32_t command);
  static int32_t handle_input_event(struct android_app *app, AInputEvent *event);

  void ns_handle_command(int32_t command);
  int32_t handle_key_event(const AInputEvent *event);
  int32_t handle_motion_event(const AInputEvent *event);

  ButtonHandle map_button(int32_t keycode);

private:
  struct android_app* _app;

  EGLDisplay _egl_display;
  EGLSurface _egl_surface;

  int32_t _mouse_button_state;

  GraphicsWindowInputDevice *_input;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsWindow::init_type();
    register_type(_type_handle, "AndroidGraphicsWindow",
                  GraphicsWindow::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "androidGraphicsWindow.I"

#endif
