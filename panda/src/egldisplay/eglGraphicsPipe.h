// Filename: eglGraphicsPipe.h
// Created by:  pro-rsoft (21May09)
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

#ifndef EGLGRAPHICSPIPE_H
#define EGLGRAPHICSPIPE_H

#include "pandabase.h"
#include "graphicsWindow.h"
#include "graphicsPipe.h"
#include "lightMutex.h"
#include "lightReMutex.h"

#ifdef OPENGLES_2
  #include "gles2gsg.h"
  #include <EGL/egl.h>
  #define NativeDisplayType EGLNativeDisplayType
  #define NativePixmapType EGLNativePixmapType
  #define NativeWindowType EGLNativeWindowType
#else
  #include "glesgsg.h"
  #include <GLES/egl.h>
#endif

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

#endif  // CPPPARSER

class eglGraphicsBuffer;
class eglGraphicsPixmap;
class eglGraphicsWindow;

////////////////////////////////////////////////////////////////////
//       Class : eglGraphicsPipe
// Description : This graphics pipe represents the interface for
//               creating OpenGL ES graphics windows on an X-based
//               (e.g. Unix) client.
////////////////////////////////////////////////////////////////////
class eglGraphicsPipe : public GraphicsPipe {
public:
  eglGraphicsPipe(const string &display = string());
  virtual ~eglGraphicsPipe();

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
                                         GraphicsEngine *engine,
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
  EGLDisplay _egl_display;

  Cursor _hidden_cursor;

  typedef int ErrorHandlerFunc(Display *, XErrorEvent *);
  typedef int IOErrorHandlerFunc(Display *);
  static bool _error_handlers_installed;
  static ErrorHandlerFunc *_prev_error_handler;
  static IOErrorHandlerFunc *_prev_io_error_handler;

public:
  // This Mutex protects any X library calls, which all have to be
  // single-threaded.  In particular, it protects eglMakeCurrent().
  static LightReMutex _x_mutex;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsPipe::init_type();
    register_type(_type_handle, "eglGraphicsPipe",
                  GraphicsPipe::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class eglGraphicsBuffer;
  friend class eglGraphicsPixmap;
  friend class eglGraphicsWindow;
};

#include "eglGraphicsPipe.I"

#endif
