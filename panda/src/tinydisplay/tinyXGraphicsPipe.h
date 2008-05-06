// Filename: tinyXGraphicsPipe.h
// Created by:  drose (03May08)
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

#ifndef TINYXGRAPHICSPIPE_H
#define TINYXGRAPHICSPIPE_H

#include "pandabase.h"

#ifdef IS_LINUX

#include "graphicsWindow.h"
#include "graphicsPipe.h"
#include "tinyGraphicsStateGuardian.h"
#include "pmutex.h"
#include "reMutex.h"

class FrameBufferProperties;

#ifdef CPPPARSER
// A simple hack so interrogate can parse this file.
typedef int Display;
typedef int Window;
typedef int XErrorEvent;
typedef int XVisualInfo;
typedef int Atom;
typedef int Cursor;
typedef int XIM;
typedef int XIC;
#else
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#endif  // CPPPARSER

////////////////////////////////////////////////////////////////////
//       Class : TinyXGraphicsPipe
// Description : This graphics pipe represents the interface for
//               creating OpenGL graphics windows on an X-based
//               (e.g. Unix) client.
////////////////////////////////////////////////////////////////////
class EXPCL_TINYDISPLAY TinyXGraphicsPipe : public GraphicsPipe {
public:
  TinyXGraphicsPipe(const string &display = string());
  virtual ~TinyXGraphicsPipe();

  virtual string get_interface_name() const;
  static PT(GraphicsPipe) pipe_constructor();

  INLINE Display *get_display() const;
  INLINE int get_screen() const;
  INLINE Window get_root() const;
  INLINE XIM get_im() const;

  INLINE Cursor get_hidden_cursor();

public:
  virtual PreferredWindowThread get_preferred_window_thread() const;

public:
  // Atom specifications.
  Atom _wm_delete_window;
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
  virtual PT(GraphicsOutput) make_output(const string &name,
                                         const FrameBufferProperties &fb_prop,
                                         const WindowProperties &win_prop,
                                         int flags,
                                         GraphicsStateGuardian *gsg,
                                         GraphicsOutput *host,
                                         int retry,
                                         bool &precertify);

private:
  void make_hidden_cursor();
  void release_hidden_cursor();

  static void install_error_handlers();
  static int error_handler(Display *display, XErrorEvent *error);
  static int io_error_handler(Display *display);

  Display *_display;
  int _screen;
  Window _root;
  XIM _im;

  Cursor _hidden_cursor;

  typedef int ErrorHandlerFunc(Display *, XErrorEvent *);
  typedef int IOErrorHandlerFunc(Display *);
  static bool _error_handlers_installed;
  static ErrorHandlerFunc *_prev_error_handler;
  static IOErrorHandlerFunc *_prev_io_error_handler;

public:
  // This Mutex protects any X library calls, which all have to be
  // single-threaded.
  static ReMutex _x_mutex;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsPipe::init_type();
    register_type(_type_handle, "TinyXGraphicsPipe",
                  GraphicsPipe::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "tinyXGraphicsPipe.I"

#endif  // IS_LINUX

#endif
