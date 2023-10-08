/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file tinyOffscreenGraphicsWindow.h
 * @author drose
 * @date 2008-04-24
 */

#ifndef TINYOffscreenGRAPHICSWINDOW_H
#define TINYOffscreenGRAPHICSWINDOW_H

#include "pandabase.h"


#include "tinyOffscreenGraphicsPipe.h"
#include "graphicsWindow.h"
#include "buttonHandle.h"
#include "zbuffer.h"


/**
 * This graphics window class is implemented via Offscreen.
 */
class EXPCL_TINYDISPLAY TinyOffscreenGraphicsWindow : public GraphicsWindow {
public:
  TinyOffscreenGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                        const std::string &name,
                        const FrameBufferProperties &fb_prop,
                        const WindowProperties &win_prop,
                        int flags,
                        GraphicsStateGuardian *gsg,
                        GraphicsOutput *host);
  virtual ~TinyOffscreenGraphicsWindow();

  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);
  virtual void end_flip();

  virtual void process_events();
  virtual void set_properties_now(WindowProperties &properties);

  virtual bool supports_pixel_zoom() const;

protected:
  virtual void close_window();
  virtual bool open_window();

private:
  void create_frame_buffer();
  static ButtonHandle get_keyboard_button(int sym);
  static ButtonHandle get_mouse_button(int button);

private:
  //SDL_Surface *_screen;
  ZBuffer *_frame_buffer;
  unsigned int _flags;
  unsigned int _pitch;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsWindow::init_type();
    register_type(_type_handle, "TinyOffscreenGraphicsWindow",
                  GraphicsWindow::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "tinyOffscreenGraphicsWindow.I"

#endif
