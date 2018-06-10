/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file tinyWinGraphicsWindow.h
 * @author drose
 * @date 2008-05-06
 */

#ifndef TINYWINGRAPHICSWINDOW_H
#define TINYWINGRAPHICSWINDOW_H

#include "pandabase.h"

#ifdef WIN32

#include "winGraphicsWindow.h"
#include "tinyWinGraphicsPipe.h"

/**
 * Opens a window on Microsoft Windows to display the TinyPanda software
 * rendering.
 */
class EXPCL_TINYDISPLAY TinyWinGraphicsWindow : public WinGraphicsWindow {
public:
  TinyWinGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                        const std::string &name,
                        const FrameBufferProperties &fb_prop,
                        const WindowProperties &win_prop,
                        int flags,
                        GraphicsStateGuardian *gsg,
                        GraphicsOutput *host);
  virtual ~TinyWinGraphicsWindow();

  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);

  virtual void end_flip();
  virtual bool supports_pixel_zoom() const;

protected:
  virtual void close_window();
  virtual bool open_window();

  virtual void handle_reshape();
  virtual bool do_fullscreen_resize(int x_size, int y_size);

private:
  void create_frame_buffer();
  void setup_bitmap_info();

private:
  ZBuffer *_frame_buffer;
  HDC _hdc;
  BITMAPINFO _bitmap_info;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    WinGraphicsWindow::init_type();
    register_type(_type_handle, "TinyWinGraphicsWindow",
                  WinGraphicsWindow::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "tinyWinGraphicsWindow.I"

#endif  // WIN32

#endif
