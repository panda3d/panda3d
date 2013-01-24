// Filename: androidGraphicsPipe.h
// Created by:  rdb (11Jan13)
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

#ifndef ANDROIDGRAPHICSPIPE_H
#define ANDROIDGRAPHICSPIPE_H

#include "pandabase.h"
#include "graphicsWindow.h"
#include "graphicsPipe.h"

#ifdef OPENGLES_2
  #include "gles2gsg.h"
//  #define NativeDisplayType EGLNativeDisplayType
//  #define NativePixmapType EGLNativePixmapType
//  #define NativeWindowType EGLNativeWindowType
#else
  #include "glesgsg.h"
#endif
#include <EGL/egl.h>

class FrameBufferProperties;

class AndroidGraphicsBuffer;
class AndroidGraphicsPixmap;
class AndroidGraphicsWindow;

////////////////////////////////////////////////////////////////////
//       Class : AndroidGraphicsPipe
// Description : This graphics pipe represents the interface for
//               creating OpenGL ES graphics windows on an X-based
//               (e.g. Unix) client.
////////////////////////////////////////////////////////////////////
class AndroidGraphicsPipe : public GraphicsPipe {
public:
  AndroidGraphicsPipe();
  virtual ~AndroidGraphicsPipe();

  virtual string get_interface_name() const;
  static PT(GraphicsPipe) pipe_constructor();

public:
  virtual PreferredWindowThread get_preferred_window_thread() const;

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
  EGLDisplay _egl_display;
  GraphicsWindow *_window;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsPipe::init_type();
    register_type(_type_handle, "AndroidGraphicsPipe",
                  GraphicsPipe::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class AndroidGraphicsBuffer;
  friend class AndroidGraphicsPixmap;
  friend class AndroidGraphicsWindow;
};

#include "androidGraphicsPipe.I"

#endif
