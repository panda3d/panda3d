// Filename: wglGraphicsWindow.h
// Created by:  drose (20Dec02)
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
  wglGraphicsWindow(GraphicsPipe *pipe);
  virtual ~wglGraphicsWindow();

  virtual void make_gsg();
  virtual void release_gsg();
  virtual void make_current();

  virtual void begin_flip();

protected:
  virtual void close_window();
  virtual bool open_window();
  virtual void reconsider_fullscreen_size(DWORD &x_size, DWORD &y_size, 
                                          DWORD &bitdepth);

private:
  int choose_pfnum() const;
  int find_pixfmtnum(bool bLookforHW) const;
  void setup_colormap();

#ifdef _DEBUG
  static void print_pfd(PIXELFORMATDESCRIPTOR *pfd, char *msg);
#endif

  HGLRC _context;
  HDC _hdc;
  PIXELFORMATDESCRIPTOR _pixelformat;
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
