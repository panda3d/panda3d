// Filename: x11GraphicsPipe.h
// Created by:  rdb (07Jul09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef X11GRAPHICSPIPE_H
#define X11GRAPHICSPIPE_H

#include "pandabase.h"
#include "graphicsWindow.h"
#include "graphicsPipe.h"
#include "lightMutex.h"
#include "lightReMutex.h"
#include "windowHandle.h"
#include "get_x11.h"

class FrameBufferProperties;

////////////////////////////////////////////////////////////////////
//       Class : x11GraphicsPipe
// Description : This graphics pipe represents the interface for
//               creating graphics windows on an X-based client.
////////////////////////////////////////////////////////////////////
class x11GraphicsPipe : public GraphicsPipe {
public:
  x11GraphicsPipe(const string &display = string());
  virtual ~x11GraphicsPipe();

  INLINE X11_Display *get_display() const;
  INLINE int get_screen() const;
  INLINE X11_Window get_root() const;
  INLINE XIM get_im() const;

  INLINE X11_Cursor get_hidden_cursor();

  static INLINE int disable_x_error_messages();
  static INLINE int enable_x_error_messages();
  static INLINE int get_x_error_count();

public:
  virtual PreferredWindowThread get_preferred_window_thread() const;

public:
  // Atom specifications.
  Atom _wm_delete_window;
  Atom _net_wm_pid;
  Atom _net_wm_window_type;
  Atom _net_wm_window_type_splash;
  Atom _net_wm_window_type_fullscreen;
  Atom _net_wm_state;
  Atom _net_wm_state_fullscreen;
  Atom _net_wm_state_above;
  Atom _net_wm_state_below;
  Atom _net_wm_state_add;
  Atom _net_wm_state_remove;

protected:
  X11_Display *_display;
  int _screen;
  X11_Window _root;
  XIM _im;

  X11_Cursor _hidden_cursor;

private:
  void make_hidden_cursor();
  void release_hidden_cursor();

  static void install_error_handlers();
  static int error_handler(X11_Display *display, XErrorEvent *error);
  static int io_error_handler(X11_Display *display);

  typedef int ErrorHandlerFunc(X11_Display *, XErrorEvent *);
  typedef int IOErrorHandlerFunc(X11_Display *);
  static bool _error_handlers_installed;
  static ErrorHandlerFunc *_prev_error_handler;
  static IOErrorHandlerFunc *_prev_io_error_handler;
  
  static bool _x_error_messages_enabled;
  static int _x_error_count;

public:
  // This Mutex protects any X library calls, which all have to be
  // single-threaded.  In particular, it protects glXMakeCurrent().
  static LightReMutex _x_mutex;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsPipe::init_type();
    register_type(_type_handle, "x11GraphicsPipe",
                  GraphicsPipe::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "x11GraphicsPipe.I"

#endif
