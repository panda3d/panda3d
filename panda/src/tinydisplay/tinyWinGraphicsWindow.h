// Filename: tinyWinGraphicsWindow.h
// Created by:  drose (06May08)
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

#ifndef TINYWINGRAPHICSWINDOW_H
#define TINYWINGRAPHICSWINDOW_H

#include "pandabase.h"

#ifdef WIN32

#include "winGraphicsWindow.h"
#include "tinyWinGraphicsPipe.h"

////////////////////////////////////////////////////////////////////
//       Class : TinyWinGraphicsWindow
// Description : Opens a window on Microsoft Windows to display the
//               TinyGL software rendering.
////////////////////////////////////////////////////////////////////
class EXPCL_TINYDISPLAY TinyWinGraphicsWindow : public WinGraphicsWindow {
public:
  TinyWinGraphicsWindow(GraphicsPipe *pipe, 
                        const string &name,
                        const FrameBufferProperties &fb_prop,
                        const WindowProperties &win_prop,
                        int flags,
                        GraphicsStateGuardian *gsg,
                        GraphicsOutput *host);
  virtual ~TinyWinGraphicsWindow();

  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);

  virtual void begin_flip();

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
