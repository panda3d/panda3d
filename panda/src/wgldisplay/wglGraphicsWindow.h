/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file wglGraphicsWindow.h
 * @author drose
 * @date 2002-12-20
 */

#ifndef WGLGRAPHICSWINDOW_H
#define WGLGRAPHICSWINDOW_H

#include "pandabase.h"
#include "winGraphicsWindow.h"

/**
 * A single graphics window for rendering OpenGL under Microsoft Windows.
 */
class EXPCL_PANDA_WGLDISPLAY wglGraphicsWindow : public WinGraphicsWindow {
public:
  wglGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                    const std::string &name,
                    const FrameBufferProperties &fb_prop,
                    const WindowProperties &win_prop,
                    int flags,
                    GraphicsStateGuardian *gsg,
                    GraphicsOutput *host);
  virtual ~wglGraphicsWindow();

  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);

  virtual void begin_flip();
  virtual void ready_flip();
  virtual void end_flip();

protected:
  virtual void close_window();
  virtual bool open_window();

private:
  void setup_colormap(const PIXELFORMATDESCRIPTOR &pixelformat);

#ifdef NOTIFY_DEBUG
  static void print_pfd(PIXELFORMATDESCRIPTOR *pfd, char *msg);
#endif

  HDC _hdc;
  HPALETTE _colormap;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    WinGraphicsWindow::init_type();
    register_type(_type_handle, "wglGraphicsWindow",
                  WinGraphicsWindow::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "wglGraphicsWindow.I"

#endif
