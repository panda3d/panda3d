// Filename: wglGraphicsBuffer.h
// Created by:  drose (08Feb04)
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

#ifndef WGLGRAPHICSBUFFER_H
#define WGLGRAPHICSBUFFER_H

#include "pandabase.h"
#include "graphicsBuffer.h"

#include <windows.h>

////////////////////////////////////////////////////////////////////
//       Class : wglGraphicsBuffer
// Description : An offscreen render buffer.  In OpenGL under Windows,
//               this simply renders into a window that is never made
//               visible.  There's a Windows interface for rendering
//               into a DIB, but this puts restrictions on the kind of
//               pixelformat we can use, and thus makes it difficult
//               to support one GSG rendering into an offscreen buffer
//               and also into a window.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAGL wglGraphicsBuffer : public GraphicsBuffer {
public:
  wglGraphicsBuffer(GraphicsPipe *pipe, GraphicsStateGuardian *gsg,
                    int x_size, int y_size, bool want_texture);
  virtual ~wglGraphicsBuffer();

  virtual void make_current();
  virtual void release_gsg();

  virtual void begin_flip();

protected:
  virtual void close_buffer();
  virtual bool open_buffer();

private:
  void setup_colormap(const PIXELFORMATDESCRIPTOR &pixelformat);

#ifdef _DEBUG
  static void print_pfd(PIXELFORMATDESCRIPTOR *pfd, char *msg);
#endif

  static void register_window_class();
  static LONG WINAPI static_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  static void show_error_message(DWORD message_id = 0);

  HWND _hWnd;
  HDC _hdc;
  HPALETTE _colormap;

  static const char * const _window_class_name;
  static bool _window_class_registered;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsBuffer::init_type();
    register_type(_type_handle, "wglGraphicsBuffer",
                  GraphicsBuffer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "wglGraphicsBuffer.I"

#endif
