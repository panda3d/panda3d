// Filename: wglGraphicsWindow.h
// Created by:  drose (20Dec02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef WGLGRAPHICSWINDOW_H
#define WGLGRAPHICSWINDOW_H

#include "pandabase.h"
#include "winGraphicsWindow.h"

////////////////////////////////////////////////////////////////////
//       Class : wglGraphicsWindow
// Description : A single graphics window for rendering OpenGL under
//               Microsoft Windows.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAGL wglGraphicsWindow : public WinGraphicsWindow {
public:
  wglGraphicsWindow(GraphicsPipe *pipe, GraphicsStateGuardian *gsg,
                    const string &name);
  virtual ~wglGraphicsWindow();

  virtual void make_current();
  virtual void release_gsg();

  virtual void begin_flip();

protected:
  virtual void close_window();
  virtual bool open_window();
  virtual void reconsider_fullscreen_size(DWORD &x_size, DWORD &y_size, 
                                          DWORD &bitdepth);

private:
  void setup_colormap(const PIXELFORMATDESCRIPTOR &pixelformat);

#ifdef _DEBUG
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
