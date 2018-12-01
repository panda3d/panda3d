/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file wglGraphicsPipe.h
 * @author drose
 * @date 2002-12-20
 */

#ifndef WGLGRAPHICSPIPE_H
#define WGLGRAPHICSPIPE_H

#include "pandabase.h"
#include "winGraphicsPipe.h"

class wglGraphicsStateGuardian;

/**
 * This graphics pipe represents the interface for creating OpenGL graphics
 * windows on the various Windows OSes.
 */
class EXPCL_PANDA_WGLDISPLAY wglGraphicsPipe : public WinGraphicsPipe {
public:
  wglGraphicsPipe();
  virtual ~wglGraphicsPipe();

  virtual std::string get_interface_name() const;
  static PT(GraphicsPipe) pipe_constructor();

protected:
  virtual PT(GraphicsOutput) make_output(const std::string &name,
                                         const FrameBufferProperties &fb_prop,
                                         const WindowProperties &win_prop,
                                         int flags,
                                         GraphicsEngine *engine,
                                         GraphicsStateGuardian *gsg,
                                         GraphicsOutput *host,
                                         int retry,
                                         bool &precertify);
  virtual PT(GraphicsStateGuardian) make_callback_gsg(GraphicsEngine *engine);

private:

  static std::string format_pfd_flags(DWORD pfd_flags);
  static bool wgl_make_current(HDC hdc, HGLRC hglrc, PStatCollector *collector);

  static bool  _current_valid;
  static HDC   _current_hdc;
  static HGLRC _current_hglrc;
  static Thread *_current_thread;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    WinGraphicsPipe::init_type();
    register_type(_type_handle, "wglGraphicsPipe",
                  WinGraphicsPipe::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class wglGraphicsBuffer;
  friend class wglGraphicsWindow;
  friend class wglGraphicsStateGuardian;
};

#include "wglGraphicsPipe.I"

#endif
