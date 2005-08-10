// Filename: glxGraphicsPipe.h
// Created by:  mike (09Jan97)
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

#ifndef GLXGRAPHICSPIPE_H
#define GLXGRAPHICSPIPE_H

#include "pandabase.h"
#include "graphicsWindow.h"
#include "graphicsPipe.h"
#include "glgsg.h"

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
#include <GL/glx.h>

#if defined(GLX_VERSION_1_3)
  // If the system glx version is at least 1.3, then we know we have
  // GLXFBConfig and GLXPbuffer.
  #define HAVE_GLXFBCONFIG
  #define HAVE_OFFICIAL_GLXFBCONFIG
#endif

// This must be included after we have included glgsg.h (which
// includes gl.h), and after we have checked GLX_VERSION_1_3.  But we
// must also include it before we redefine the GLXFBConfig types,
// below.
#include "glxext.h"

// drose: the version of GL/glx.h that ships with Fedora Core 2 seems
// to define GLX_VERSION_1_4, but for some reason does not define
// GLX_SAMPLE_BUFFERS or GLX_SAMPLES.  We work around that here.

#ifndef GLX_SAMPLE_BUFFERS
#define GLX_SAMPLE_BUFFERS                 100000
#endif
#ifndef GLX_SAMPLES
#define GLX_SAMPLES                        100001
#endif


#if !defined(HAVE_GLXFBCONFIG) && defined(GLX_SGIX_fbconfig) && defined(GLX_SGIX_pbuffer)
  // If the system glx version isn't 1.3, but these were defined as
  // extensions, we can work with that.
  #define GLXFBConfig GLXFBConfigSGIX
  #define GLXPbuffer GLXPbufferSGIX
  #define glXChooseFBConfig glXChooseFBConfigSGIX
  #define glXCreateNewContext glXCreateContextWithConfigSGIX
  #define glXGetVisualFromFBConfig glXGetVisualFromFBConfigSGIX
  #define glXGetFBConfigAttrib glXGetFBConfigAttribSGIX
  #define glXDestroyPbuffer glXDestroyGLXPbufferSGIX

  #define HAVE_GLXFBCONFIG
  #define HAVE_SGI_GLXFBCONFIG
#endif

#endif  // CPPPARSER

////////////////////////////////////////////////////////////////////
//       Class : glxGraphicsPipe
// Description : This graphics pipe represents the interface for
//               creating OpenGL graphics windows on an X-based
//               (e.g. Unix) client.
////////////////////////////////////////////////////////////////////
class glxGraphicsPipe : public GraphicsPipe {
public:
  glxGraphicsPipe(const string &display = string());
  virtual ~glxGraphicsPipe();

  virtual string get_interface_name() const;
  static PT(GraphicsPipe) pipe_constructor();

  INLINE Display *get_display() const;
  INLINE int get_screen() const;
  INLINE Window get_root() const;
  INLINE XIM get_im() const;

  INLINE Cursor get_hidden_cursor();

public:
  // Atom specifications.
  Atom _wm_delete_window;
  Atom _net_wm_window_type;
  Atom _net_wm_window_type_splash;
  Atom _net_wm_window_type_fullscreen;

protected:
  virtual PT(GraphicsStateGuardian) make_gsg(const FrameBufferProperties &properties,
                                             GraphicsStateGuardian *share_with);
  virtual PT(GraphicsWindow) make_window(GraphicsStateGuardian *gsg,
                                         const string &name);
  virtual PT(GraphicsBuffer) make_buffer(GraphicsStateGuardian *gsg, 
                                         const string &name,
                                         int x_size, int y_size);

private:
#ifdef HAVE_GLXFBCONFIG
  GLXFBConfig choose_fbconfig(FrameBufferProperties &properties) const;
  GLXFBConfig try_for_fbconfig(int framebuffer_mode,
                               int want_depth_bits = 1, 
                               int want_color_bits = 1,
                               int want_alpha_bits = 1, 
                               int want_stencil_bits = 1,
                               int want_multisamples = 1) const;
#endif

  XVisualInfo *choose_visual(FrameBufferProperties &properties) const;
  XVisualInfo *try_for_visual(int framebuffer_mode,
                              int want_depth_bits = 1, 
                              int want_color_bits = 1,
                              int want_alpha_bits = 1, 
                              int want_stencil_bits = 1,
                              int want_multisamples = 1) const;

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
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsPipe::init_type();
    register_type(_type_handle, "glxGraphicsPipe",
                  GraphicsPipe::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "glxGraphicsPipe.I"

#endif
