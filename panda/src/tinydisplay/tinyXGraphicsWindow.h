/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file tinyXGraphicsWindow.h
 * @author drose
 * @date 2008-05-03
 */

#ifndef TINYXGRAPHICSWINDOW_H
#define TINYXGRAPHICSWINDOW_H

#include "pandabase.h"

#ifdef HAVE_X11

#include "tinyXGraphicsPipe.h"
#include "x11GraphicsWindow.h"
#include "buttonHandle.h"

/**
 * Opens a window on X11 to display the TinyPanda software rendering.
 */
class EXPCL_TINYDISPLAY TinyXGraphicsWindow : public x11GraphicsWindow {
public:
  TinyXGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                      const std::string &name,
                      const FrameBufferProperties &fb_prop,
                      const WindowProperties &win_prop,
                      int flags,
                      GraphicsStateGuardian *gsg,
                      GraphicsOutput *host);
  virtual ~TinyXGraphicsWindow();

  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);
  virtual void end_flip();
  virtual bool supports_pixel_zoom() const;

  virtual void process_events();

protected:
  virtual void close_window();
  virtual bool open_window();
  virtual void pixel_factor_changed();

private:
  void create_full_frame_buffer();
  void create_reduced_frame_buffer();
  void create_ximage();

private:
  ZBuffer *_reduced_frame_buffer;
  ZBuffer *_full_frame_buffer;
  int _pitch;
  XImage *_ximage;
  GC _gc;
  int _bytes_per_pixel;
  Visual *_visual;
  int _depth;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    x11GraphicsWindow::init_type();
    register_type(_type_handle, "TinyXGraphicsWindow",
                  x11GraphicsWindow::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "tinyXGraphicsWindow.I"

#endif  // HAVE_X11

#endif
