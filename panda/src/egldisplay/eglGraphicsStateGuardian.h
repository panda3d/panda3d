/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eglGraphicsStateGuardian.h
 * @author rdb
 * @date 2009-05-21
 */

#ifndef EGLGRAPHICSSTATEGUARDIAN_H
#define EGLGRAPHICSSTATEGUARDIAN_H

#include "pandabase.h"
#include "eglGraphicsPipe.h"

#ifdef USE_X11
#include "get_x11.h"
#endif

#ifdef OPENGLES_2
typedef GLES2GraphicsStateGuardian BaseGraphicsStateGuardian;
#elif defined(OPENGLES_1)
typedef GLESGraphicsStateGuardian BaseGraphicsStateGuardian;
#else
typedef GLGraphicsStateGuardian BaseGraphicsStateGuardian;
#endif

/**
 * A tiny specialization on GLESGraphicsStateGuardian to add some egl-specific
 * information.
 */
class eglGraphicsStateGuardian : public BaseGraphicsStateGuardian {
public:
  INLINE const FrameBufferProperties &get_fb_properties() const;
  void get_properties(FrameBufferProperties &properties,
             bool &pbuffer_supported, bool &pixmap_supported,
                               bool &slow, EGLConfig config);
  void choose_pixel_format(const FrameBufferProperties &properties,
                           eglGraphicsPipe *egl_pipe, bool need_window,
                           bool need_pbuffer, bool need_pixmap);

  eglGraphicsStateGuardian(GraphicsEngine *engine, GraphicsPipe *pipe,
         eglGraphicsStateGuardian *share_with);

  virtual ~eglGraphicsStateGuardian();

  virtual void reset();

  bool egl_is_at_least_version(int major_version, int minor_version) const;

  EGLContext _share_context;
  EGLContext _context;
  EGLDisplay _egl_display;
#ifdef USE_X11
  XVisualInfo *_visual = nullptr;
#endif
  EGLConfig _fbconfig;
  FrameBufferProperties _fbprops;

protected:
  virtual void gl_flush() const;
  virtual GLenum gl_get_error() const;

  virtual void query_gl_version();
  virtual void get_extra_extensions();
  virtual void *do_get_extension_func(const char *name);

private:
  int _egl_version_major, _egl_version_minor;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BaseGraphicsStateGuardian::init_type();
    register_type(_type_handle, "eglGraphicsStateGuardian",
                  BaseGraphicsStateGuardian::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "eglGraphicsStateGuardian.I"

#endif
