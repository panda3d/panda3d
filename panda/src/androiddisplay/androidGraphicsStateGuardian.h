// Filename: androidGraphicsStateGuardian.h
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

#ifndef ANDROIDGRAPHICSSTATEGUARDIAN_H
#define ANDROIDGRAPHICSSTATEGUARDIAN_H

#include "pandabase.h"
#include "androidGraphicsPipe.h"

////////////////////////////////////////////////////////////////////
//       Class : AndroidGraphicsStateGuardian
// Description : A tiny specialization on GLESGraphicsStateGuardian
//               to add some egl-specific information.
////////////////////////////////////////////////////////////////////
#ifdef OPENGLES_2
class AndroidGraphicsStateGuardian : public GLES2GraphicsStateGuardian {
#else
class AndroidGraphicsStateGuardian : public GLESGraphicsStateGuardian {
#endif
public:
  INLINE const FrameBufferProperties &get_fb_properties() const;
  void get_properties(FrameBufferProperties &properties,
                      bool &pbuffer_supported, bool &pixmap_supported,
                      bool &slow, EGLConfig config);
  void choose_pixel_format(const FrameBufferProperties &properties,
                           bool need_pbuffer, bool need_pixmap);
  bool create_context();
  void destroy_context();

  AndroidGraphicsStateGuardian(GraphicsEngine *engine, GraphicsPipe *pipe,
         AndroidGraphicsStateGuardian *share_with);

  virtual ~AndroidGraphicsStateGuardian();

  virtual void reset();

  bool egl_is_at_least_version(int major_version, int minor_version) const;

protected:
  EGLContext _share_context;
  EGLContext _context;
  EGLDisplay _egl_display;
  EGLConfig _fbconfig;
  EGLint _format;
  FrameBufferProperties _fbprops;

protected:
  virtual void gl_flush() const;
  virtual GLenum gl_get_error() const;

  virtual void query_gl_version();
  virtual void get_extra_extensions();
  virtual void *do_get_extension_func(const char *name);

private:
  int _egl_version_major, _egl_version_minor;

  friend class AndroidGraphicsWindow;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
#ifdef OPENGLES_2
    GLES2GraphicsStateGuardian::init_type();
    register_type(_type_handle, "AndroidGraphicsStateGuardian",
                  GLES2GraphicsStateGuardian::get_class_type());
#else
    GLESGraphicsStateGuardian::init_type();
    register_type(_type_handle, "AndroidGraphicsStateGuardian",
                  GLESGraphicsStateGuardian::get_class_type());
#endif
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "androidGraphicsStateGuardian.I"

#endif
