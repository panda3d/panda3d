// Filename: glxGraphicsWindow.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef GLXGRAPHICSWINDOW_H
#define GLXGRAPHICSWINDOW_H

#include "pandabase.h"

#include "graphicsWindow.h"
#include "buttonHandle.h"

#include <X11/Xlib.h>
#include <GL/glx.h>

class glxGraphicsPipe;

////////////////////////////////////////////////////////////////////
//       Class : glxGraphicsWindow
// Description : An interface to the glx system for managing GL
//               windows under X.
////////////////////////////////////////////////////////////////////
class glxGraphicsWindow : public GraphicsWindow {
public:
  glxGraphicsWindow(GraphicsPipe *pipe, GraphicsStateGuardian *gsg);
  virtual ~glxGraphicsWindow();

  virtual void make_current();
  virtual void release_gsg();

  virtual bool begin_frame();
  virtual void begin_flip();

  virtual void process_events();
  virtual void set_properties_now(WindowProperties &properties);

protected:
  virtual void close_window();
  virtual bool open_window();

private:
  void set_wm_properties(const WindowProperties &properties);

  void setup_colormap(XVisualInfo *visual);
  ButtonHandle get_button(XKeyEvent *key_event);

  static Bool check_event(Display *display, XEvent *event, char *arg);

private:
  Display *_display;
  int _screen;
  Window _xwindow;
  Colormap _colormap;
  XIC _ic;

  long _event_mask;
  bool _awaiting_configure;
  Atom _wm_delete_window;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsWindow::init_type();
    register_type(_type_handle, "glxGraphicsWindow",
                  GraphicsWindow::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "glxGraphicsWindow.I"

#endif
