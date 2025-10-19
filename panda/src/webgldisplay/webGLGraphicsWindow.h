/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file webGLGraphicsWindow.h
 * @author rdb
 * @date 2015-03-31
 */

#ifndef WEBGLGRAPHICSWINDOW_H
#define WEBGLGRAPHICSWINDOW_H

#include "pandabase.h"

#include "webGLGraphicsPipe.h"
#include "graphicsWindow.h"

#include <emscripten/html5.h>

/**
 * An interface to Emscripten's WebGL interface that represents an HTML5
 * canvas.
 */
class WebGLGraphicsWindow : public GraphicsWindow {
public:
  WebGLGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                      const std::string &name,
                      const FrameBufferProperties &fb_prop,
                      const WindowProperties &win_prop,
                      int flags,
                      GraphicsStateGuardian *gsg,
                      GraphicsOutput *host);
  virtual ~WebGLGraphicsWindow();

  virtual bool move_pointer(int device, int x, int y);
  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);
  virtual void end_flip();

  virtual void process_events();
  virtual void set_properties_now(WindowProperties &properties);

protected:
  virtual void close_window();
  virtual bool open_window();

public:
  void on_resize(double width, double height);

private:
  static EM_BOOL on_fullscreen_event(int type, const EmscriptenFullscreenChangeEvent *event, void *user_data);
  static EM_BOOL on_pointerlock_event(int type, const EmscriptenPointerlockChangeEvent *event, void *user_data);
  static EM_BOOL on_visibility_event(int type, const EmscriptenVisibilityChangeEvent *event, void *user_data);
  static EM_BOOL on_focus_event(int type, const EmscriptenFocusEvent *event, void *user_data);
  static EM_BOOL on_keyboard_event(int type, const EmscriptenKeyboardEvent *event, void *user_data);
  static EM_BOOL on_mouse_event(int type, const EmscriptenMouseEvent *event, void *user_data);
  static EM_BOOL on_wheel_event(int type, const EmscriptenWheelEvent *event, void *user_data);

  static ButtonHandle map_key(int which);
  static ButtonHandle map_raw_key(const char *code);

  std::string _canvas_id;

  GraphicsWindowInputDevice *_input;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsWindow::init_type();
    register_type(_type_handle, "WebGLGraphicsWindow",
                  GraphicsWindow::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "webGLGraphicsWindow.I"

#endif
