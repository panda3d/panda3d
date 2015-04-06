// Filename: webGLGraphicsStateGuardian.h
// Created by:  rdb (01Apr15)
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

#ifndef WEBGLGRAPHICSSTATEGUARDIAN_H
#define WEBGLGRAPHICSSTATEGUARDIAN_H

#include "pandabase.h"
#include "webGLGraphicsPipe.h"
#include "gles2gsg.h"

#include <html5.h>

////////////////////////////////////////////////////////////////////
//       Class : WebGLGraphicsStateGuardian
// Description : A specialization on GLES2GraphicsStateGuardian
//               to add emscripten-specific context set-up.
////////////////////////////////////////////////////////////////////
class WebGLGraphicsStateGuardian : public GLES2GraphicsStateGuardian {
public:
  WebGLGraphicsStateGuardian(GraphicsEngine *engine, GraphicsPipe *pipe);
  virtual ~WebGLGraphicsStateGuardian();

  void choose_pixel_format(const FrameBufferProperties &properties,
                           const char *target);

  static EM_BOOL on_context_event(int type, const void *, void *user_data);

  virtual void reset();

private:
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE _context;
  bool _have_context;

protected:
  virtual void get_extra_extensions();
  virtual bool has_extension(const string &extension) const;
  virtual void *do_get_extension_func(const char *name);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GLES2GraphicsStateGuardian::init_type();
    register_type(_type_handle, "WebGLGraphicsStateGuardian",
                  GLES2GraphicsStateGuardian::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class WebGLGraphicsWindow;
};

#include "webGLGraphicsStateGuardian.I"

#endif
