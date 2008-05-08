// Filename: tinySDLGraphicsWindow.h
// Created by:  drose (24Apr08)
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

#ifndef TINYSDLGRAPHICSWINDOW_H
#define TINYSDLGRAPHICSWINDOW_H

#include "pandabase.h"

#ifdef HAVE_SDL

#include "tinySDLGraphicsPipe.h"
#include "graphicsWindow.h"
#include "buttonHandle.h"
#include "SDL.h"
#include "zbuffer.h"

////////////////////////////////////////////////////////////////////
//       Class : TinySDLGraphicsWindow
// Description : This graphics window class is implemented via SDL.
////////////////////////////////////////////////////////////////////
class EXPCL_TINYDISPLAY TinySDLGraphicsWindow : public GraphicsWindow {
public:
  TinySDLGraphicsWindow(GraphicsPipe *pipe, 
                        const string &name,
                        const FrameBufferProperties &fb_prop,
                        const WindowProperties &win_prop,
                        int flags,
                        GraphicsStateGuardian *gsg,
                        GraphicsOutput *host);
  virtual ~TinySDLGraphicsWindow();

  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);
  virtual void begin_flip();

  virtual void process_events();
  virtual void set_properties_now(WindowProperties &properties);

  virtual bool supports_pixel_zoom() const;

protected:
  virtual void close_window();
  virtual bool open_window();

private:
  void create_frame_buffer();
  static ButtonHandle get_keyboard_button(SDLKey sym);
  static ButtonHandle get_mouse_button(Uint8 button);

private:
  SDL_Surface *_screen;
  ZBuffer *_frame_buffer;
  unsigned int _flags;
  unsigned int _pitch;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsWindow::init_type();
    register_type(_type_handle, "TinySDLGraphicsWindow",
                  GraphicsWindow::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "tinySDLGraphicsWindow.I"

#endif  // HAVE_SDL

#endif
